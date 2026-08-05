#include "wxCustomization/widgets/StyledTitleBar.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/widgets/StyledMenu.h"

#include <wx/dcbuffer.h>
#include <wx/menu.h>
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

void StyledTitleBar::AddMenu(const wxString& label, wxMenu* menu)
{
    m_menus.push_back({label, menu});
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

int StyledTitleBar::GetMenuButtonsWidth() const
{
    int width = 0;
    for (size_t i = 0; i < m_menus.size(); ++i) {
        width += GetMenuButtonRect(static_cast<int>(i)).width;
    }
    return width;
}

wxRect StyledTitleBar::GetMenuButtonRect(int index) const
{
    const wxRect content = GetContentRect();
    const Style menuStyle = GetSubControlStyleStrict("menu-button", "");

    wxFont font = menuStyle.font.IsOk() ? menuStyle.font : GetTitleFont();
    int textWidth = 0;
    int textHeight = 0;
    GetTextExtent(m_menus[index].label, &textWidth, &textHeight, nullptr, nullptr,
                  &font);

    const int width = textWidth + menuStyle.paddingLeft + menuStyle.paddingRight;

    // Menu buttons are laid out left to right from the content origin.
    int x = content.x;
    for (int i = 0; i < index; ++i) {
        x += GetMenuButtonRect(i).width;
    }
    return wxRect(x, content.y, width, content.height);
}

int StyledTitleBar::HitTestMenuButton(const wxPoint& pt) const
{
    for (size_t i = 0; i < m_menus.size(); ++i) {
        if (GetMenuButtonRect(static_cast<int>(i)).Contains(pt)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

Style StyledTitleBar::GetMenuButtonStyle(int index) const
{
    // Strict resolution, like for the caption buttons: the bar's own hover
    // state must not make every menu button match `:hover`.
    Style style = GetSubControlStyleStrict("menu-button", "");

    if (!IsEnabled()) {
        style.Merge(GetSubControlStyleStrict("menu-button", "disabled"));
        return style;
    }
    if (index == m_hoveredMenuButton) {
        style.Merge(GetSubControlStyleStrict("menu-button", "hover"));
    }
    if (index == m_openMenuButton) {
        style.Merge(GetSubControlStyleStrict("menu-button", "pressed"));
    }
    return style;
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

void StyledTitleBar::DrawMenuButton(wxDC& dc, int index)
{
    const wxRect rect = GetMenuButtonRect(index);
    Style style = GetMenuButtonStyle(index);

    Painter painter;
    painter.DrawBackground(dc, rect, style);

    if (!style.font.IsOk()) {
        style.font = GetTitleFont();
    }
    if (!style.color.IsOk() && GetCurrentStyle().color.IsOk()) {
        style.color = GetCurrentStyle().color;
    }
    painter.DrawText(dc, rect, m_menus[index].label, style);
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

    for (size_t i = 0; i < m_menus.size(); ++i) {
        DrawMenuButton(dc, static_cast<int>(i));
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
             + style.paddingLeft + style.paddingRight + GetCaptionButtonsWidth()
             + GetMenuButtonsWidth();

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

    // Menus open on mouse press, like in a regular menu bar. The pressed
    // state is cleared when the menu closes (by ShowMenu/dismiss handler).
    const int menu = HitTestMenuButton(evt.GetPosition());
    if (menu >= 0) {
        m_openMenuButton = menu;
        Refresh();
        ShowMenu(menu);
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

    bool changed = false;

    const int btn = HitTestCaptionButton(evt.GetPosition());
    if (btn != m_hoveredButton) {
        m_hoveredButton = btn;
        changed = true;
    }

    const int menu = HitTestMenuButton(evt.GetPosition());
    if (menu != m_hoveredMenuButton) {
        m_hoveredMenuButton = menu;
        changed = true;
    }

    if (changed) {
        Refresh();
    }
    evt.Skip();
}

void StyledTitleBar::OnMouseLeave(wxMouseEvent& evt)
{
    m_hoveredButton = -1;
    m_pressedButton = -1;
    m_hoveredMenuButton = -1;
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

void StyledTitleBar::ShowMenu(int index)
{
    if (index < 0 || index >= static_cast<int>(m_menus.size())) {
        return;
    }
    wxMenu* menu = m_menus[index].menu;
    if (menu == nullptr) {
        return;
    }

    if (m_styleSheet == nullptr) {
        // No theme: fall back to the native popup (modal), then clear the
        // pressed state right away.
        const wxRect rect = GetMenuButtonRect(index);
        PopupMenu(menu, rect.x, rect.y + rect.height);
        m_openMenuButton = -1;
        Refresh();
        return;
    }

    // Styled popup (non-modal): the pressed state is cleared by the dismiss
    // handler when the menu closes.
    if (m_menuPopup == nullptr) {
        m_menuPopup = new StyledMenu(this, m_styleSheet);
        m_menuPopup->SetDismissHandler([this]() {
            m_openMenuButton = -1;
            Refresh();
        });
    }
    m_menuPopup->BuildFromMenu(menu);
    m_menuPopup->PopupAt(ClientToScreen(GetMenuButtonRect(index).GetBottomLeft()));
}

} // namespace wxCustomization
