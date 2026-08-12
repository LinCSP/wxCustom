#pragma once

#include "wxCustomization/StyledControl.h"

#include <functional>
#include <vector>

namespace wxCustomization {

/// Emitted when the selected row changes (mouse click, keyboard or
/// SetSelectedRow). The selected row index is in wxCommandEvent::GetInt()
/// (-1 when the selection was cleared).
wxDECLARE_EVENT(wxEVT_STYLED_TABLE_SELECTION, wxCommandEvent);

/// Emitted when a row is activated (left double-click). The row index is in
/// wxCommandEvent::GetInt().
wxDECLARE_EVENT(wxEVT_STYLED_TABLE_ACTIVATED, wxCommandEvent);

/// Column description for StyledTable.
struct StyledTableColumn {
    wxString title;
    int widthDip = 80;        ///< Column width in device-independent pixels.
    int align = wxALIGN_LEFT; ///< wxALIGN_LEFT, wxALIGN_RIGHT or wxALIGN_CENTRE.
    bool sortable = true;     ///< Whether clicking the header reports a sort request.
};

/// A styled, owner-drawn table widget with virtual rows, single row
/// selection and header-click sort reporting.
///
/// The widget does not store cell data: the application provides a cell
/// text provider via SetCellProvider() and a row count via SetRowCount(),
/// and calls RefreshRows() after the underlying data changed.
///
/// Clicking the header of a sortable column invokes the callback set via
/// SetOnHeaderClick(); sorting the data itself is the application's job.
/// The widget only draws the direction arrow set via SetSortIndicator().
///
/// Sub-control styling: `::header` (states `:hover`, `:pressed` per column),
/// `::row` (states `:hover`, `:selected`, `:alternate` for zebra striping),
/// `::scroll-bar` (track; `width`/`height` set the bar thickness) and
/// `::scroll-thumb`. Grid lines are drawn with the widget's `border-color`.
///
/// Selection changes emit `wxEVT_STYLED_TABLE_SELECTION` with the row index
/// in wxCommandEvent::GetInt().
class StyledTable : public StyledControl {
public:
    StyledTable(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxControlNameStr);

    /// Replace the column set. Resets the sort indicator and the
    /// horizontal scroll offset.
    void SetColumns(const std::vector<StyledTableColumn>& cols);

    /// Return the current column set.
    const std::vector<StyledTableColumn>& GetColumns() const { return m_columns; }

    /// Return the number of columns.
    int GetColumnCount() const { return static_cast<int>(m_columns.size()); }

    /// Set the virtual cell text provider. May be empty (blank cells).
    void SetCellProvider(std::function<wxString(int row, int col)> provider);

    /// Set the number of virtual rows. Clamps the selection and the scroll
    /// offset when they fall out of range.
    void SetRowCount(int count);

    /// Return the number of virtual rows.
    int GetRowCount() const { return m_rowCount; }

    /// Repaint after the underlying data changed (row count unchanged).
    void RefreshRows();

    /// Select @p row (-1 clears the selection). Emits
    /// `wxEVT_STYLED_TABLE_SELECTION` and invokes the selection callback
    /// when the selection actually changes.
    void SetSelectedRow(int row);

    /// Return the selected row index or -1.
    int GetSelectedRow() const { return m_selectedRow; }

    /// Convenience callback invoked on every selection change.
    void SetOnSelectionChanged(std::function<void(int row)> callback);

    /// Callback invoked when the header of a sortable column is clicked.
    /// The application is expected to re-sort the data, call
    /// SetSortIndicator() and then RefreshRows().
    void SetOnHeaderClick(std::function<void(int col)> callback);

    /// Callback invoked when a row is activated (left double-click).
    void SetOnRowActivated(std::function<void(int row)> callback);

    /// Draw the sort direction arrow on column @p col. Pass -1 to remove
    /// the indicator.
    void SetSortIndicator(int col, bool ascending);

    /// Return the column carrying the sort indicator or -1.
    int GetSortColumn() const { return m_sortColumn; }

    /// Return the direction of the sort indicator.
    bool IsSortAscending() const { return m_sortAscending; }

    /// Scroll so that @p row becomes visible.
    void EnsureRowVisible(int row);

    wxString GetStyledControlType() const override { return "StyledTable"; }

protected:
    void OnPaint(wxPaintEvent& evt) override;
    /// Renders the header, the rows and the scrollbars into @p rect.
    /// Separated from OnPaint so tests can paint into an arbitrary DC.
    virtual void DrawTable(wxDC& dc, const wxRect& rect);

    void OnLeftDown(wxMouseEvent& evt) override;
    void OnLeftUp(wxMouseEvent& evt) override;
    void OnLeftDClick(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt) override;
    void OnMouseLeave(wxMouseEvent& evt) override;
    void OnMouseWheel(wxMouseEvent& evt);
    void OnKeyDown(wxKeyEvent& evt) override;

    wxSize DoGetBestSize() const override;

private:
    /// Resolved geometry snapshot for the current client size.
    struct Layout {
        wxRect content;     ///< Content rect (client minus border/padding).
        wxRect header;      ///< Header strip.
        wxRect body;        ///< Visible rows area (scrollbars excluded).
        wxRect vTrack;      ///< Vertical scrollbar track (empty when hidden).
        wxRect hTrack;      ///< Horizontal scrollbar track (empty when hidden).
        wxRect vThumb;
        wxRect hThumb;
        int headerHeight = 0;
        int rowHeight = 1;
        int visibleRows = 0; ///< Fully/partially visible rows in the body.
        int maxFirstRow = 0;
        int maxScrollX = 0;
        int scrollBarSize = 0;
    };

    enum class DragMode { None, VThumb, HThumb };

    Layout GetLayout() const;
    int GetHeaderHeight() const;
    int GetRowHeight() const;
    int GetColumnWidth(int col) const;
    int GetTotalColumnsWidth() const;
    wxFont GetEffectiveFont() const;

    /// Column index whose header contains @p pt (client coords) or -1.
    int HitTestHeader(const wxPoint& pt) const;
    /// Row index at @p pt (client coords, body area) or -1.
    int HitTestRow(const wxPoint& pt) const;

    void SetSelectedRowInternal(int row, bool sendEvent);
    void SendSelectionEvent(int row);
    void ScrollRows(int delta);
    void SetFirstRow(int firstRow);
    void SetScrollX(int scrollX);

    Style GetHeaderStyle(int col) const;
    Style GetRowStyle(int row) const;
    void DrawHeader(wxDC& dc, const Layout& layout);
    void DrawRows(wxDC& dc, const Layout& layout);
    void DrawScrollBars(wxDC& dc, const Layout& layout);
    void DrawSortArrow(wxDC& dc, const wxRect& headerCellRect, const Style& headerStyle);

    std::vector<StyledTableColumn> m_columns;
    std::function<wxString(int row, int col)> m_cellProvider;
    int m_rowCount = 0;

    int m_selectedRow = -1;
    int m_hoveredRow = -1;
    int m_hoveredHeaderCol = -1;
    int m_pressedHeaderCol = -1;

    int m_firstRow = 0;
    int m_scrollX = 0;

    int m_sortColumn = -1;
    bool m_sortAscending = true;

    DragMode m_dragMode = DragMode::None;
    int m_dragGrabOffset = 0; ///< Mouse offset inside the thumb when drag started.

    std::function<void(int row)> m_onSelectionChanged;
    std::function<void(int col)> m_onHeaderClick;
    std::function<void(int row)> m_onRowActivated;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
