#include "wxCustomization/widgets/StyledTable.h"

#include "wxCustomization/Painter.h"

#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

wxDEFINE_EVENT(wxEVT_STYLED_TABLE_SELECTION, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_STYLED_TABLE_ACTIVATED, wxCommandEvent);

namespace {

constexpr int DEFAULT_SCROLLBAR_THICKNESS_DIP = 8;
constexpr int MIN_THUMB_SIZE_DIP = 16;
constexpr int DEFAULT_VISIBLE_ROWS = 8;
constexpr int WHEEL_SCROLL_ROWS = 3;
constexpr int DEFAULT_CELL_PADDING_DIP = 4;
constexpr int SORT_ARROW_WIDTH_DIP = 8;
constexpr int SORT_ARROW_HEIGHT_DIP = 5;

} // namespace

wxBEGIN_EVENT_TABLE(StyledTable, StyledControl)
    EVT_MOUSEWHEEL(StyledTable::OnMouseWheel)
    EVT_LEFT_DCLICK(StyledTable::OnLeftDClick)
wxEND_EVENT_TABLE()

StyledTable::StyledTable(wxWindow* parent,
                         wxWindowID id,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
{
}

void StyledTable::SetColumns(const std::vector<StyledTableColumn>& cols)
{
    m_columns = cols;
    m_sortColumn = -1;
    m_scrollX = 0;
    m_hoveredHeaderCol = -1;
    m_pressedHeaderCol = -1;
    InvalidateBestSize();
    Refresh();
}

void StyledTable::SetCellProvider(std::function<wxString(int row, int col)> provider)
{
    m_cellProvider = std::move(provider);
    Refresh();
}

void StyledTable::SetRowCount(int count)
{
    m_rowCount = std::max(0, count);
    // Data changed under us: drop a now-invalid selection and scroll offset
    // silently (no event — this is not a user selection change).
    if (m_selectedRow >= m_rowCount) {
        m_selectedRow = -1;
    }
    SetFirstRow(m_firstRow);
    Refresh();
}

void StyledTable::RefreshRows()
{
    SetFirstRow(m_firstRow);
    Refresh();
}

void StyledTable::SetSelectedRow(int row)
{
    SetSelectedRowInternal(row, true);
}

void StyledTable::SetOnSelectionChanged(std::function<void(int row)> callback)
{
    m_onSelectionChanged = std::move(callback);
}

void StyledTable::SetOnHeaderClick(std::function<void(int col)> callback)
{
    m_onHeaderClick = std::move(callback);
}

void StyledTable::SetOnRowActivated(std::function<void(int row)> callback)
{
    m_onRowActivated = std::move(callback);
}

void StyledTable::SetSortIndicator(int col, bool ascending)
{
    if (col < -1 || col >= GetColumnCount()) {
        return;
    }
    if (m_sortColumn == col && (col < 0 || m_sortAscending == ascending)) {
        return;
    }
    m_sortColumn = col;
    m_sortAscending = ascending;
    Refresh();
}

void StyledTable::EnsureRowVisible(int row)
{
    if (row < 0 || row >= m_rowCount) {
        return;
    }

    const Layout layout = GetLayout();
    const int page = std::max(1, layout.visibleRows);
    if (row < m_firstRow) {
        SetFirstRow(row);
    } else if (row >= m_firstRow + page) {
        SetFirstRow(row - page + 1);
    }
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

wxFont StyledTable::GetEffectiveFont() const
{
    const Style& style = GetCurrentStyle();
    if (style.font.IsOk()) {
        return style.font;
    }
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

int StyledTable::GetRowHeight() const
{
    const Style rowStyle = GetSubControlStyle("row");
    if (rowStyle.IsSet(Property::Height) && rowStyle.height > 0) {
        return rowStyle.height;
    }

    const wxFont font = GetEffectiveFont();
    int textHeight = 0;
    GetTextExtent("Wg", nullptr, &textHeight, nullptr, nullptr,
                  const_cast<wxFont*>(&font));

    int height = textHeight + rowStyle.paddingTop + rowStyle.paddingBottom;
    if (rowStyle.IsSet(Property::MinHeight) && height < rowStyle.minHeight) {
        height = rowStyle.minHeight;
    }
    return std::max(height, 1);
}

int StyledTable::GetHeaderHeight() const
{
    const Style headerStyle = GetSubControlStyle("header");
    if (headerStyle.IsSet(Property::Height) && headerStyle.height > 0) {
        return headerStyle.height;
    }

    const wxFont font = headerStyle.font.IsOk() ? headerStyle.font : GetEffectiveFont();
    int textHeight = 0;
    GetTextExtent("Wg", nullptr, &textHeight, nullptr, nullptr,
                  const_cast<wxFont*>(&font));

    int height = textHeight + headerStyle.paddingTop + headerStyle.paddingBottom;
    if (headerStyle.IsSet(Property::MinHeight) && height < headerStyle.minHeight) {
        height = headerStyle.minHeight;
    }
    return std::max(height, 1);
}

int StyledTable::GetColumnWidth(int col) const
{
    return FromDIP(m_columns[col].widthDip);
}

int StyledTable::GetTotalColumnsWidth() const
{
    int total = 0;
    for (size_t i = 0; i < m_columns.size(); ++i) {
        total += GetColumnWidth(static_cast<int>(i));
    }
    return total;
}

StyledTable::Layout StyledTable::GetLayout() const
{
    Layout layout;
    layout.content = GetContentRect();
    layout.headerHeight = GetHeaderHeight();
    layout.rowHeight = GetRowHeight();

    const Style barStyle = GetSubControlStyle("scroll-bar");
    const int vBarWidth = barStyle.IsSet(Property::Width) && barStyle.width > 0
                              ? barStyle.width
                              : FromDIP(DEFAULT_SCROLLBAR_THICKNESS_DIP);
    const int hBarHeight = barStyle.IsSet(Property::Height) && barStyle.height > 0
                               ? barStyle.height
                               : FromDIP(DEFAULT_SCROLLBAR_THICKNESS_DIP);
    layout.scrollBarSize = vBarWidth;

    const int bodyWidth = layout.content.width;
    const int bodyHeight = std::max(0, layout.content.height - layout.headerHeight);
    const int totalRowsHeight = m_rowCount * layout.rowHeight;
    const int totalColsWidth = GetTotalColumnsWidth();

    // The bars steal space from each other, so resolve the overflow twice.
    bool needV = false;
    bool needH = false;
    for (int i = 0; i < 2; ++i) {
        needV = totalRowsHeight > bodyHeight - (needH ? hBarHeight : 0);
        needH = totalColsWidth > bodyWidth - (needV ? vBarWidth : 0);
    }

    layout.header = wxRect(layout.content.x, layout.content.y,
                           layout.content.width, layout.headerHeight);
    layout.body = wxRect(layout.content.x,
                         layout.content.y + layout.headerHeight,
                         bodyWidth - (needV ? vBarWidth : 0),
                         bodyHeight - (needH ? hBarHeight : 0));

    layout.visibleRows = std::max(1, layout.body.height / layout.rowHeight);
    layout.maxFirstRow = std::max(0, m_rowCount - layout.visibleRows);
    layout.maxScrollX = std::max(0, totalColsWidth - layout.body.width);

    const int minThumb = FromDIP(MIN_THUMB_SIZE_DIP);
    if (needV) {
        layout.vTrack = wxRect(layout.body.x + layout.body.width, layout.body.y,
                               vBarWidth, layout.body.height);
        int thumbHeight = layout.vTrack.height;
        if (m_rowCount > 0) {
            thumbHeight = std::max(
                layout.vTrack.height * layout.visibleRows / std::max(m_rowCount, 1),
                std::min(minThumb, layout.vTrack.height));
        }
        const int range = std::max(0, layout.vTrack.height - thumbHeight);
        const int offset = layout.maxFirstRow > 0
                               ? range * m_firstRow / layout.maxFirstRow
                               : 0;
        layout.vThumb = wxRect(layout.vTrack.x, layout.vTrack.y + offset,
                               vBarWidth, thumbHeight);
    }
    if (needH) {
        layout.hTrack = wxRect(layout.body.x, layout.body.y + layout.body.height,
                               layout.body.width, hBarHeight);
        int thumbWidth = layout.hTrack.width;
        if (totalColsWidth > 0) {
            thumbWidth = std::max(
                layout.hTrack.width * layout.body.width / std::max(totalColsWidth, 1),
                std::min(minThumb, layout.hTrack.width));
        }
        const int range = std::max(0, layout.hTrack.width - thumbWidth);
        const int offset = layout.maxScrollX > 0 ? range * m_scrollX / layout.maxScrollX
                                                 : 0;
        layout.hThumb = wxRect(layout.hTrack.x + offset, layout.hTrack.y,
                               thumbWidth, hBarHeight);
    }
    return layout;
}

int StyledTable::HitTestHeader(const wxPoint& pt) const
{
    const Layout layout = GetLayout();
    if (!layout.header.Contains(pt)) {
        return -1;
    }

    int x = layout.header.x - m_scrollX;
    for (size_t i = 0; i < m_columns.size(); ++i) {
        x += GetColumnWidth(static_cast<int>(i));
        if (pt.x < x) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int StyledTable::HitTestRow(const wxPoint& pt) const
{
    const Layout layout = GetLayout();
    if (!layout.body.Contains(pt)) {
        return -1;
    }

    const int row = m_firstRow + (pt.y - layout.body.y) / layout.rowHeight;
    return row < m_rowCount ? row : -1;
}

// ---------------------------------------------------------------------------
// Styles
// ---------------------------------------------------------------------------

Style StyledTable::GetHeaderStyle(int col) const
{
    // Strict resolution: the widget's own hover must not light up every
    // column header — only the actually hovered one does.
    Style style = GetSubControlStyleStrict("header", "");
    if (!IsEnabled()) {
        style.Merge(GetSubControlStyleStrict("header", "disabled"));
        return style;
    }
    if (col == m_hoveredHeaderCol) {
        style.Merge(GetSubControlStyleStrict("header", "hover"));
    }
    if (col == m_pressedHeaderCol) {
        style.Merge(GetSubControlStyleStrict("header", "pressed"));
    }
    return style;
}

Style StyledTable::GetRowStyle(int row) const
{
    Style style = GetSubControlStyleStrict("row", "");
    if (!IsEnabled()) {
        style.Merge(GetSubControlStyleStrict("row", "disabled"));
        return style;
    }
    if (row % 2 == 1) {
        // Zebra striping: applies only when the stylesheet defines it.
        style.Merge(GetSubControlStyleStrict("row", "alternate"));
    }
    if (row == m_hoveredRow) {
        style.Merge(GetSubControlStyleStrict("row", "hover"));
    }
    if (row == m_selectedRow) {
        // Selected wins over hover, like the later QSS rule would.
        style.Merge(GetSubControlStyleStrict("row", "selected"));
    }
    return style;
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

void StyledTable::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    DrawTable(dc, GetClientRect());
}

void StyledTable::DrawSortArrow(wxDC& dc, const wxRect& headerCellRect,
                                const Style& headerStyle)
{
    const int arrowWidth = FromDIP(SORT_ARROW_WIDTH_DIP);
    const int arrowHeight = FromDIP(SORT_ARROW_HEIGHT_DIP);

    wxColour colour = headerStyle.color;
    if (!colour.IsOk()) {
        colour = GetCurrentStyle().color;
    }
    if (!colour.IsOk()) {
        colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    }

    const int cx = headerCellRect.x + headerCellRect.width - arrowWidth / 2
                   - (headerStyle.paddingRight > 0 ? headerStyle.paddingRight
                                                   : FromDIP(DEFAULT_CELL_PADDING_DIP));
    const int cy = headerCellRect.y + headerCellRect.height / 2;

    wxPoint points[3];
    if (m_sortAscending) {
        points[0] = wxPoint(cx - arrowWidth / 2, cy + arrowHeight / 2);
        points[1] = wxPoint(cx + arrowWidth / 2, cy + arrowHeight / 2);
        points[2] = wxPoint(cx, cy - arrowHeight / 2);
    } else {
        points[0] = wxPoint(cx - arrowWidth / 2, cy - arrowHeight / 2);
        points[1] = wxPoint(cx + arrowWidth / 2, cy - arrowHeight / 2);
        points[2] = wxPoint(cx, cy + arrowHeight / 2);
    }

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(colour));
    dc.DrawPolygon(3, points);
}

void StyledTable::DrawHeader(wxDC& dc, const Layout& layout)
{
    if (m_columns.empty() || layout.header.height <= 0) {
        return;
    }

    Painter painter;

    // Header strip background (the base ::header style, without per-column
    // pseudo states). Painter would stroke the full box for the border, so
    // the bottom separator is drawn by hand instead.
    const Style baseStyle = GetSubControlStyleStrict("header", "");
    painter.DrawBackground(dc, layout.header, baseStyle);

    int separatorWidth = baseStyle.IsSet(Property::BorderBottomWidth)
                             ? baseStyle.borderBottomWidth
                             : 0;
    if (separatorWidth == 0 && baseStyle.IsSet(Property::BorderWidth)) {
        separatorWidth = baseStyle.borderWidth;
    }
    if (separatorWidth > 0 && baseStyle.borderStyle != BorderStyle::None
        && baseStyle.borderStyle != BorderStyle::Hidden
        && baseStyle.borderColor.IsOk() && baseStyle.borderColor.Alpha() > 0) {
        dc.SetPen(wxPen(baseStyle.borderColor, separatorWidth));
        const int lineY = layout.header.y + layout.header.height - separatorWidth / 2 - 1;
        dc.DrawLine(layout.header.x, lineY, layout.header.x + layout.header.width, lineY);
    }

    wxDCClipper clip(dc, layout.header);

    const Style& widgetStyle = GetCurrentStyle();
    int x = layout.header.x - m_scrollX;
    for (size_t i = 0; i < m_columns.size(); ++i) {
        const int col = static_cast<int>(i);
        const int colWidth = GetColumnWidth(col);
        const wxRect cellRect(x, layout.header.y, colWidth, layout.header.height);
        x += colWidth;

        if (cellRect.x + cellRect.width < layout.header.x
            || cellRect.x > layout.header.x + layout.header.width) {
            continue;
        }

        const Style colStyle = GetHeaderStyle(col);
        // Per-column state background (hover/pressed) over the strip.
        if (colStyle.IsSet(Property::BackgroundColor) && colStyle.backgroundColor.IsOk()
            && (col == m_hoveredHeaderCol || col == m_pressedHeaderCol)) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(colStyle.backgroundColor));
            dc.DrawRectangle(cellRect);
        }

        const wxFont font = colStyle.font.IsOk() ? colStyle.font : GetEffectiveFont();
        dc.SetFont(font);
        wxColour textColour = colStyle.color.IsOk() ? colStyle.color : widgetStyle.color;
        if (!textColour.IsOk()) {
            textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
        }
        dc.SetTextForeground(textColour);

        const int padLeft = colStyle.IsSet(Property::PaddingLeft)
                                ? colStyle.paddingLeft
                                : FromDIP(DEFAULT_CELL_PADDING_DIP);
        const int padRight = colStyle.IsSet(Property::PaddingRight)
                                 ? colStyle.paddingRight
                                 : FromDIP(DEFAULT_CELL_PADDING_DIP);
        // Keep room for the sort arrow on the sorted column.
        const int arrowReserve = col == m_sortColumn
                                     ? FromDIP(SORT_ARROW_WIDTH_DIP) + padRight
                                     : 0;

        const wxString& title = m_columns[i].title;
        int textWidth = 0;
        int textHeight = 0;
        dc.GetTextExtent(title, &textWidth, &textHeight);

        const int availWidth = cellRect.width - padLeft - padRight - arrowReserve;
        int textX = cellRect.x + padLeft;
        if (m_columns[i].align == wxALIGN_RIGHT) {
            textX = cellRect.x + cellRect.width - padRight - arrowReserve - textWidth;
        } else if (m_columns[i].align == wxALIGN_CENTRE) {
            textX = cellRect.x + padLeft + (availWidth - textWidth) / 2;
        }
        const int textY = cellRect.y + (cellRect.height - textHeight) / 2;

        wxDCClipper cellClip(dc, cellRect);
        dc.DrawText(title, textX, textY);

        if (col == m_sortColumn) {
            DrawSortArrow(dc, cellRect, colStyle);
        }
    }
}

void StyledTable::DrawRows(wxDC& dc, const Layout& layout)
{
    if (layout.body.width <= 0 || layout.body.height <= 0) {
        return;
    }

    const Style& widgetStyle = GetCurrentStyle();
    const wxFont widgetFont = GetEffectiveFont();

    wxColour gridColour = widgetStyle.borderColor;

    wxDCClipper clip(dc, layout.body);

    // Rows scroll by whole rows, so the first row always starts at body top.
    const int bottom = layout.body.y + layout.body.height;
    int y = layout.body.y;
    for (int row = m_firstRow; row < m_rowCount && y < bottom;
         ++row, y += layout.rowHeight) {
        const wxRect rowRect(layout.body.x, y, layout.body.width, layout.rowHeight);
        const Style rowStyle = GetRowStyle(row);

        if (rowStyle.IsSet(Property::BackgroundColor) && rowStyle.backgroundColor.IsOk()
            && rowStyle.backgroundColor.Alpha() > 0) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(rowStyle.backgroundColor));
            dc.DrawRectangle(rowRect);
        }

        const wxFont font = rowStyle.font.IsOk() ? rowStyle.font : widgetFont;
        dc.SetFont(font);
        wxColour textColour = rowStyle.color.IsOk() ? rowStyle.color : widgetStyle.color;
        if (!textColour.IsOk()) {
            textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
        }
        dc.SetTextForeground(textColour);

        const int padLeft = rowStyle.IsSet(Property::PaddingLeft)
                                ? rowStyle.paddingLeft
                                : FromDIP(DEFAULT_CELL_PADDING_DIP);
        const int padRight = rowStyle.IsSet(Property::PaddingRight)
                                 ? rowStyle.paddingRight
                                 : FromDIP(DEFAULT_CELL_PADDING_DIP);

        int x = layout.body.x - m_scrollX;
        for (size_t c = 0; c < m_columns.size(); ++c) {
            const int colWidth = GetColumnWidth(static_cast<int>(c));
            const wxRect cellRect(x, y, colWidth, layout.rowHeight);
            x += colWidth;

            // Vertical grid line at the column boundary.
            if (gridColour.IsOk() && gridColour.Alpha() > 0 && c + 1 < m_columns.size()) {
                dc.SetPen(wxPen(gridColour, 1));
                dc.DrawLine(x, y, x, y + layout.rowHeight);
            }

            if (cellRect.x + cellRect.width < layout.body.x
                || cellRect.x > layout.body.x + layout.body.width) {
                continue;
            }

            wxString text;
            if (m_cellProvider) {
                text = m_cellProvider(row, static_cast<int>(c));
            }
            if (text.empty()) {
                continue;
            }

            int textWidth = 0;
            int textHeight = 0;
            dc.GetTextExtent(text, &textWidth, &textHeight);

            int textX = cellRect.x + padLeft;
            if (m_columns[c].align == wxALIGN_RIGHT) {
                textX = cellRect.x + cellRect.width - padRight - textWidth;
            } else if (m_columns[c].align == wxALIGN_CENTRE) {
                textX = cellRect.x + (cellRect.width - textWidth) / 2;
            }
            const int textY = cellRect.y + (cellRect.height - textHeight) / 2;

            wxDCClipper cellClip(dc, cellRect);
            dc.DrawText(text, textX, textY);
        }

        // Horizontal grid line under the row.
        if (gridColour.IsOk() && gridColour.Alpha() > 0) {
            dc.SetPen(wxPen(gridColour, 1));
            dc.DrawLine(layout.body.x, y + layout.rowHeight - 1,
                        layout.body.x + layout.body.width, y + layout.rowHeight - 1);
        }
    }
}

void StyledTable::DrawScrollBars(wxDC& dc, const Layout& layout)
{
    if (layout.vTrack.width <= 0 && layout.hTrack.height <= 0) {
        return;
    }

    const Style barStyle = GetSubControlStyle("scroll-bar");
    const Style thumbStyle = GetSubControlStyle("scroll-thumb");

    wxColour thumbColour = thumbStyle.backgroundColor;
    if (!thumbColour.IsOk()) {
        thumbColour = GetCurrentStyle().borderColor;
    }
    if (!thumbColour.IsOk()) {
        thumbColour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
    }

    const wxRect tracks[] = {layout.vTrack, layout.hTrack};
    for (const wxRect& track : tracks) {
        if (track.width <= 0 || track.height <= 0) {
            continue;
        }
        if (barStyle.IsSet(Property::BackgroundColor) && barStyle.backgroundColor.IsOk()) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(barStyle.backgroundColor));
            dc.DrawRectangle(track);
        }
    }

    // Corner square where the two bars meet.
    if (layout.vTrack.width > 0 && layout.hTrack.height > 0) {
        const wxRect corner(layout.vTrack.x, layout.hTrack.y,
                            layout.vTrack.width, layout.hTrack.height);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(barStyle.IsSet(Property::BackgroundColor)
                            && barStyle.backgroundColor.IsOk()
                        ? wxBrush(barStyle.backgroundColor)
                        : wxBrush(GetCurrentStyle().backgroundColor.IsOk()
                                      ? GetCurrentStyle().backgroundColor
                                      : wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)));
        dc.DrawRectangle(corner);
    }

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(thumbColour));
    if (layout.vThumb.width > 0 && layout.vThumb.height > 0) {
        if (thumbStyle.borderRadius > 0) {
            dc.DrawRoundedRectangle(layout.vThumb, thumbStyle.borderRadius);
        } else {
            dc.DrawRectangle(layout.vThumb);
        }
    }
    if (layout.hThumb.width > 0 && layout.hThumb.height > 0) {
        if (thumbStyle.borderRadius > 0) {
            dc.DrawRoundedRectangle(layout.hThumb, thumbStyle.borderRadius);
        } else {
            dc.DrawRectangle(layout.hThumb);
        }
    }
}

void StyledTable::DrawTable(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();

    Painter painter;
    // Widget background, border and grid-line colour source.
    painter.Paint(dc, rect, style, this);

    const Layout layout = GetLayout();
    DrawHeader(dc, layout);
    DrawRows(dc, layout);
    DrawScrollBars(dc, layout);

    // Focus indicator around the whole table.
    if (IsFocused()) {
        painter.DrawOutline(dc, rect, style);
    }
}

wxSize StyledTable::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();

    int width = GetTotalColumnsWidth() + FromDIP(DEFAULT_SCROLLBAR_THICKNESS_DIP);
    int height = GetHeaderHeight() + GetRowHeight() * DEFAULT_VISIBLE_ROWS;

    width += style.borderLeftWidth + style.borderRightWidth + style.paddingLeft
             + style.paddingRight;
    height += style.borderTopWidth + style.borderBottomWidth + style.paddingTop
              + style.paddingBottom;

    if (style.IsSet(Property::Width) && style.width > 0) {
        width = style.width;
    }
    if (style.IsSet(Property::Height) && style.height > 0) {
        height = style.height;
    }
    if (style.IsSet(Property::MinWidth) && width < style.minWidth) {
        width = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && height < style.minHeight) {
        height = style.minHeight;
    }

    return wxSize(width, height);
}

// ---------------------------------------------------------------------------
// Selection and scrolling
// ---------------------------------------------------------------------------

void StyledTable::SetSelectedRowInternal(int row, bool sendEvent)
{
    if (m_rowCount == 0) {
        row = -1;
    } else if (row >= m_rowCount) {
        row = m_rowCount - 1;
    } else if (row < -1) {
        row = -1;
    }

    if (row == m_selectedRow) {
        return;
    }

    m_selectedRow = row;
    if (row >= 0) {
        EnsureRowVisible(row);
    }
    Refresh();

    if (sendEvent) {
        SendSelectionEvent(row);
        if (m_onSelectionChanged) {
            m_onSelectionChanged(row);
        }
    }
}

void StyledTable::SendSelectionEvent(int row)
{
    wxCommandEvent event(wxEVT_STYLED_TABLE_SELECTION, GetId());
    event.SetInt(row);
    event.SetEventObject(this);
    ProcessWindowEvent(event);
}

void StyledTable::SetFirstRow(int firstRow)
{
    const Layout layout = GetLayout();
    firstRow = std::clamp(firstRow, 0, layout.maxFirstRow);
    if (firstRow != m_firstRow) {
        m_firstRow = firstRow;
        Refresh();
    }
}

void StyledTable::SetScrollX(int scrollX)
{
    const Layout layout = GetLayout();
    scrollX = std::clamp(scrollX, 0, layout.maxScrollX);
    if (scrollX != m_scrollX) {
        m_scrollX = scrollX;
        Refresh();
    }
}

void StyledTable::ScrollRows(int delta)
{
    SetFirstRow(m_firstRow + delta);
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void StyledTable::OnLeftDown(wxMouseEvent& evt)
{
    const Layout layout = GetLayout();
    const wxPoint pt = evt.GetPosition();

    if (layout.vThumb.Contains(pt)) {
        m_dragMode = DragMode::VThumb;
        m_dragGrabOffset = pt.y - layout.vThumb.y;
        CaptureMouse();
        SetFocus();
        return;
    }
    if (layout.hThumb.Contains(pt)) {
        m_dragMode = DragMode::HThumb;
        m_dragGrabOffset = pt.x - layout.hThumb.x;
        CaptureMouse();
        SetFocus();
        return;
    }
    // Click on a track outside the thumb: page scroll towards the click.
    if (layout.vTrack.Contains(pt)) {
        ScrollRows(pt.y < layout.vThumb.y ? -layout.visibleRows : layout.visibleRows);
        return;
    }
    if (layout.hTrack.Contains(pt)) {
        SetScrollX(m_scrollX
                   + (pt.x < layout.hThumb.x ? -layout.body.width : layout.body.width));
        return;
    }

    const int col = HitTestHeader(pt);
    if (col >= 0) {
        m_pressedHeaderCol = col;
        SetFocus();
        Refresh();
        return;
    }

    if (layout.body.Contains(pt)) {
        SetFocus();
        const int row = HitTestRow(pt);
        if (row >= 0) {
            SetSelectedRowInternal(row, true);
        }
        return;
    }

    evt.Skip();
}

void StyledTable::OnLeftDClick(wxMouseEvent& evt)
{
    const Layout layout = GetLayout();
    const wxPoint pt = evt.GetPosition();
    if (layout.body.Contains(pt)) {
        const int row = HitTestRow(pt);
        if (row >= 0) {
            SetSelectedRowInternal(row, true);
            wxCommandEvent event(wxEVT_STYLED_TABLE_ACTIVATED, GetId());
            event.SetInt(row);
            event.SetEventObject(this);
            ProcessWindowEvent(event);
            if (m_onRowActivated) {
                m_onRowActivated(row);
            }
            return;
        }
    }
    evt.Skip();
}

void StyledTable::OnLeftUp(wxMouseEvent& evt)
{    if (m_dragMode != DragMode::None) {
        m_dragMode = DragMode::None;
        if (HasCapture()) {
            ReleaseMouse();
        }
        return;
    }

    if (m_pressedHeaderCol >= 0) {
        const int col = m_pressedHeaderCol;
        m_pressedHeaderCol = -1;
        Refresh();
        // Sort request fires on release over the same header, like a button.
        if (HitTestHeader(evt.GetPosition()) == col && m_columns[col].sortable
            && m_onHeaderClick) {
            m_onHeaderClick(col);
        }
        return;
    }

    evt.Skip();
}

void StyledTable::OnMotion(wxMouseEvent& evt)
{
    const wxPoint pt = evt.GetPosition();

    if (m_dragMode != DragMode::None) {
        const Layout layout = GetLayout();
        if (m_dragMode == DragMode::VThumb) {
            const int range = layout.vTrack.height - layout.vThumb.height;
            const int pos =
                std::clamp(pt.y - layout.vTrack.y - m_dragGrabOffset, 0, std::max(range, 0));
            SetFirstRow(range > 0 ? pos * layout.maxFirstRow / range : 0);
        } else {
            const int range = layout.hTrack.width - layout.hThumb.width;
            const int pos =
                std::clamp(pt.x - layout.hTrack.x - m_dragGrabOffset, 0, std::max(range, 0));
            SetScrollX(range > 0 ? pos * layout.maxScrollX / range : 0);
        }
        return;
    }

    const int headerCol = HitTestHeader(pt);
    const int row = headerCol >= 0 ? -1 : HitTestRow(pt);
    if (headerCol != m_hoveredHeaderCol || row != m_hoveredRow) {
        m_hoveredHeaderCol = headerCol;
        m_hoveredRow = row;
        Refresh();
    }
    evt.Skip();
}

void StyledTable::OnMouseLeave(wxMouseEvent& evt)
{
    m_hoveredRow = -1;
    m_hoveredHeaderCol = -1;
    m_pressedHeaderCol = -1;
    StyledControl::OnMouseLeave(evt);
}

void StyledTable::OnMouseWheel(wxMouseEvent& evt)
{
    const Layout layout = GetLayout();
    const int rotation = evt.GetWheelRotation();

    if (layout.maxFirstRow > 0) {
        ScrollRows(rotation > 0 ? -WHEEL_SCROLL_ROWS : WHEEL_SCROLL_ROWS);
    } else if (layout.maxScrollX > 0) {
        // No vertical overflow: the wheel scrolls horizontally instead.
        const int step = GetRowHeight() * WHEEL_SCROLL_ROWS;
        SetScrollX(m_scrollX + (rotation > 0 ? -step : step));
    } else {
        evt.Skip();
    }
}

void StyledTable::OnKeyDown(wxKeyEvent& evt)
{
    if (m_rowCount == 0) {
        evt.Skip();
        return;
    }

    const Layout layout = GetLayout();
    const int page = std::max(1, layout.visibleRows);

    int target = m_selectedRow;
    switch (evt.GetKeyCode()) {
        case WXK_UP:
        case WXK_NUMPAD_UP:
            target = m_selectedRow < 0 ? 0 : m_selectedRow - 1;
            break;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            target = m_selectedRow < 0 ? 0 : m_selectedRow + 1;
            break;
        case WXK_PAGEUP:
        case WXK_NUMPAD_PAGEUP:
            target = m_selectedRow < 0 ? 0 : m_selectedRow - page;
            break;
        case WXK_PAGEDOWN:
        case WXK_NUMPAD_PAGEDOWN:
            target = m_selectedRow < 0 ? 0 : m_selectedRow + page;
            break;
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            target = 0;
            break;
        case WXK_END:
        case WXK_NUMPAD_END:
            target = m_rowCount - 1;
            break;
        default:
            evt.Skip();
            return;
    }

    SetSelectedRowInternal(std::clamp(target, 0, m_rowCount - 1), true);
}

} // namespace wxCustomization
