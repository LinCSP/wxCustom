#include "wxCustomization/widgets/StyledTitleBar.h"

#include "wxCustomization/Painter.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/toplevel.h>

#ifdef __WXGTK__
#include <gtk/gtk.h>
#endif

#include <algorithm>

namespace wxCustomization {

wxBEGIN_EVENT_TABLE(StyledTitleBar, StyledControl)
    EVT_LEFT_DCLICK(StyledTitleBar::OnLeftDClick)
wxEND_EVENT_TABLE()

StyledTitleBar::StyledTitleBar(wxWindow* parent,
                               wxWindowID id,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
{
    SetAccessibleRole(wxROLE_SYSTEM_TITLEBAR);
}

void StyledTitleBar::SetTitle(const wxString& title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    InvalidateBestSize();
    Refresh();
}

wxString StyledTitleBar::CaptionButtonKind(int index)
{
    switch (index) {
        case BtnMinimize: return "minimize";
        case BtnMaximize: return "maximize";
        case BtnClose: return "close";
        default: return wxString();
    }
}

wxTopLevelWindow* StyledTitleBar::GetTopLevelWindow() const
{
    return dynamic_cast<wxTopLevelWindow*>(
        wxGetTopLevelParent(const_cast<StyledTitleBar*>(this)));
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

int StyledTitleBar::GetCaptionButtonsWidth() const
{
    int width = 0;
    for (int i = 0; i < BtnCount; ++i) {
        width += GetCaptionButtonRect(i).width;
    }
    return width;
}

wxRect StyledTitleBar::GetCaptionButtonRect(int index) const
{
    const wxRect content = GetContentRect();

    const Style capStyle = GetSubControlStyle("caption-button");
    const Style kindStyle = GetSubControlStyle(CaptionButtonKind(index) + "-button");

    int width = content.height;
    if (capStyle.IsSet(Property::Width) && capStyle.width > 0) {
        width = capStyle.width;
    }
    if (kindStyle.IsSet(Property::Width) && kindStyle.width > 0) {
        width = kindStyle.width;
    }

    // Buttons are anchored to the right edge; Close is rightmost.
    const int rightSlot = BtnCount - 1 - index;
    const int x = content.x + content.width - (rightSlot + 1) * width;
    return wxRect(x, content.y, width, content.height);
}

int StyledTitleBar::HitTestCaptionButton(const wxPoint& pt) const
{
    for (int i = 0; i < BtnCount; ++i) {
        if (GetCaptionButtonRect(i).Contains(pt)) {
            return i;
        }
    }
    return -1;
}

wxFont StyledTitleBar::GetTitleFont() const
{
    const Style titleStyle = GetSubControlStyle("title");
    if (titleStyle.font.IsOk()) {
        return titleStyle.font;
    }
    const Style& style = GetCurrentStyle();
    if (style.font.IsOk()) {
        return style.font;
    }
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

void StyledTitleBar::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    DrawTitleBar(dc, GetClientRect());
}

Style StyledTitleBar::GetCaptionButtonStyle(int index) const
{
    const wxString kind = CaptionButtonKind(index) + "-button";

    // Strict resolution: the bar's own hover state must not make every
    // button match `:hover` — only the actually hovered one does.
    Style style = GetSubControlStyleStrict("caption-button", "");
    style.Merge(GetSubControlStyleStrict(kind, ""));

    if (!IsEnabled()) {
        style.Merge(GetSubControlStyleStrict("caption-button", "disabled"));
        style.Merge(GetSubControlStyleStrict(kind, "disabled"));
        return style;
    }
    if (index == m_hoveredButton) {
        style.Merge(GetSubControlStyleStrict("caption-button", "hover"));
        style.Merge(GetSubControlStyleStrict(kind, "hover"));
    }
    if (index == m_pressedButton) {
        style.Merge(GetSubControlStyleStrict("caption-button", "pressed"));
        style.Merge(GetSubControlStyleStrict(kind, "pressed"));
    }
    return style;
}

void StyledTitleBar::DrawCaptionButton(wxDC& dc, int index)
{
    const wxRect rect = GetCaptionButtonRect(index);
    const Style style = GetCaptionButtonStyle(index);

    Painter painter;
    painter.DrawBackground(dc, rect, style);

    const wxColour colour = style.color.IsOk() ? style.color
        : (GetCurrentStyle().color.IsOk() ? GetCurrentStyle().color
                                          : wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    dc.SetPen(wxPen(colour, std::max(1, FromDIP(1))));

    // Glyph, ~1/3 of the button size, centred.
    const int half = std::min(rect.width, rect.height);
    const int glyph = std::max(4, half / 3);
    const int cx = rect.x + rect.width / 2;
    const int cy = rect.y + rect.height / 2;
    const int g = glyph / 2;

    switch (index) {
        case BtnMinimize:
            dc.DrawLine(cx - g, cy, cx + g + 1, cy);
            break;
        case BtnMaximize:
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle(cx - g, cy - g, glyph + 1, glyph + 1);
            break;
        case BtnClose:
            dc.DrawLine(cx - g, cy - g, cx + g + 1, cy + g + 1);
            dc.DrawLine(cx + g, cy - g, cx - g - 1, cy + g + 1);
            break;
        default:
            break;
    }
}

void StyledTitleBar::DrawTitleBar(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();

    Painter painter;
    painter.Paint(dc, rect, style, this);

    // Title text, centred in the whole bar.
    if (!m_title.empty()) {
        Style titleStyle = GetSubControlStyle("title");
        if (!titleStyle.font.IsOk()) {
            titleStyle.font = GetTitleFont();
        }
        if (!titleStyle.color.IsOk() && style.color.IsOk()) {
            titleStyle.color = style.color;
        }
        const wxRect content = GetContentRect();
        painter.DrawText(dc, content, m_title, titleStyle);
    }

    for (int i = 0; i < BtnCount; ++i) {
        DrawCaptionButton(dc, i);
    }
}

wxSize StyledTitleBar::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();

    const wxFont font = GetTitleFont();
    int textWidth = 0;
    int textHeight = 0;
    GetTextExtent(m_title, &textWidth, &textHeight, nullptr, nullptr,
                  const_cast<wxFont*>(&font));

    wxSize size;
    size.y = textHeight + style.borderTopWidth + style.borderBottomWidth
             + style.paddingTop + style.paddingBottom;
    size.x = textWidth + style.borderLeftWidth + style.borderRightWidth
             + style.paddingLeft + style.paddingRight + GetCaptionButtonsWidth();

    if (style.IsSet(Property::MinWidth) && size.x < style.minWidth) {
        size.x = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && size.y < style.minHeight) {
        size.y = style.minHeight;
    }
    return size;
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void StyledTitleBar::OnLeftDown(wxMouseEvent& evt)
{
    const int btn = HitTestCaptionButton(evt.GetPosition());
    if (btn >= 0) {
        m_pressedButton = btn;
        Refresh();
        return;
    }

    wxTopLevelWindow* tlw = GetTopLevelWindow();
    if (tlw == nullptr || tlw->IsMaximized()) {
        return;
    }

#ifdef __WXGTK__
    // Hand the drag to the toolkit/compositor: on Wayland an application
    // cannot move its own windows, so wxWindow::Move() is a no-op there.
    // gtk_window_begin_move_drag works on both X11 and Wayland and needs no
    // mouse capture on our side.
    if (GtkWidget* widget = tlw->GetHandle()) {
        const wxPoint root = ClientToScreen(evt.GetPosition());
        gtk_window_begin_move_drag(GTK_WINDOW(widget), 1, root.x, root.y,
                                   gtk_get_current_event_time());
    }
#else
    // Manual drag (Windows, macOS): the window follows the pointer.
    m_dragging = true;
    if (IsShownOnScreen()) {
        CaptureMouse();
    }
    m_dragOffset = ClientToScreen(evt.GetPosition()) - tlw->GetPosition();
#endif
}

void StyledTitleBar::OnLeftUp(wxMouseEvent& evt)
{
    // A caption button press and a drag never start together; handle the
    // button first so a stale drag state can never swallow the click.
    const int btn = m_pressedButton;
    m_pressedButton = -1;
    if (btn >= 0) {
        if (HitTestCaptionButton(evt.GetPosition()) == btn) {
            ActivateCaptionButton(btn);
        }
        Refresh();
        return;
    }

    if (m_dragging) {
        m_dragging = false;
        if (HasCapture()) {
            ReleaseMouse();
        }
        return;
    }
    evt.Skip();
}

void StyledTitleBar::OnMotion(wxMouseEvent& evt)
{
    if (m_dragging) {
        if (wxTopLevelWindow* tlw = GetTopLevelWindow()) {
            tlw->Move(ClientToScreen(evt.GetPosition()) - m_dragOffset);
        }
        return;
    }

    const int btn = HitTestCaptionButton(evt.GetPosition());
    if (btn != m_hoveredButton) {
        m_hoveredButton = btn;
        Refresh();
    }
    evt.Skip();
}

void StyledTitleBar::OnMouseLeave(wxMouseEvent& evt)
{
    m_hoveredButton = -1;
    m_pressedButton = -1;
    StyledControl::OnMouseLeave(evt);
}

void StyledTitleBar::OnLeftDClick(wxMouseEvent& evt)
{
    if (HitTestCaptionButton(evt.GetPosition()) >= 0) {
        return;
    }
    if (wxTopLevelWindow* tlw = GetTopLevelWindow()) {
        tlw->Maximize(!tlw->IsMaximized());
    }
}

void StyledTitleBar::ActivateCaptionButton(int index)
{
    wxTopLevelWindow* tlw = GetTopLevelWindow();
    if (tlw == nullptr) {
        return;
    }

    switch (index) {
        case BtnMinimize:
            tlw->Iconize();
            break;
        case BtnMaximize:
            tlw->Maximize(!tlw->IsMaximized());
            break;
        case BtnClose:
            tlw->Close();
            break;
        default:
            break;
    }
}

} // namespace wxCustomization
