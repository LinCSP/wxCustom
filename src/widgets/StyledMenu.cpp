#include "wxCustomization/widgets/StyledMenu.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/StyleResolver.h"
#include "wxCustomization/StyleResolverContext.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/dcbuffer.h>
#include <wx/menu.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

namespace {

/// Resolver context for the popup menu: selectors `StyledMenu`, `::item`,
/// `::separator`. Pseudo-classes match only against the state passed to
/// ResolveStyle (strict), like for other per-element widgets.
class StyledMenuContext : public StyleResolverContext {
public:
    StyledMenuContext(const StyledMenu* menu, const wxString& state)
        : m_menu(menu)
        , m_state(state)
    {
    }

    wxString GetControlType() const override { return "StyledMenu"; }
    wxString GetControlId() const override { return wxString(); }
    std::vector<wxString> GetControlClasses() const override { return {}; }

    bool HasPseudoState(const wxString& s) const override { return s == m_state; }

    wxString GetDynamicProperty(const wxString& /*name*/) const override
    {
        return wxString();
    }

    const wxWindow* GetWindow() const override { return m_menu; }

private:
    const StyledMenu* m_menu;
    wxString m_state;
};

} // namespace

wxBEGIN_EVENT_TABLE(StyledMenu, wxPopupTransientWindow)
    EVT_PAINT(StyledMenu::OnPaint)
    EVT_MOTION(StyledMenu::OnMouseMove)
    EVT_LEFT_DOWN(StyledMenu::OnLeftDown)
    EVT_LEFT_UP(StyledMenu::OnLeftUp)
    EVT_KEY_DOWN(StyledMenu::OnKeyDown)
wxEND_EVENT_TABLE()

StyledMenu::StyledMenu(wxWindow* owner, StyleSheet* sheet)
    : wxPopupTransientWindow(owner, wxBORDER_NONE | wxWANTS_CHARS)
    , m_owner(owner)
    , m_sheet(sheet)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void StyledMenu::SetDismissHandler(std::function<void()> handler)
{
    m_dismissHandler = std::move(handler);
}

void StyledMenu::BuildFromMenu(const wxMenu* menu)
{
    m_items.clear();
    m_hovered = -1;

    if (menu == nullptr) {
        return;
    }

    for (wxMenuItem* menuItem : menu->GetMenuItems()) {
        Item item;
        if (menuItem->IsSeparator()) {
            item.separator = true;
        } else {
            item.label = menuItem->GetItemLabelText();
            const wxString full = menuItem->GetItemLabel();
            const int tab = full.Find('\t');
            if (tab != wxNOT_FOUND) {
                item.shortcut = full.Mid(tab + 1);
            }
            item.id = menuItem->GetId();
            item.enabled = menuItem->IsEnabled();
            item.checkable = menuItem->IsCheckable();
            // IsChecked() asserts on uncheckable items.
            item.checked = item.checkable && menuItem->IsChecked();
            item.submenu = menuItem->IsSubMenu();
        }
        m_items.push_back(std::move(item));
    }
}

void StyledMenu::PopupAt(const wxPoint& screenPos)
{
    if (m_items.empty()) {
        return;
    }

    const wxSize size = CalcMenuSize();
    SetSize(size);
    Position(screenPos, wxDefaultSize);

    wxPopupTransientWindow::Popup(this);
    SetFocus();
}

void StyledMenu::OnDismiss()
{
    wxPopupTransientWindow::OnDismiss();
    if (m_dismissHandler) {
        m_dismissHandler();
    }
}

// ---------------------------------------------------------------------------
// Styles
// ---------------------------------------------------------------------------

Style StyledMenu::ResolveStyle(const wxString& subControl, const wxString& state) const
{
    if (m_sheet == nullptr) {
        return Style();
    }
    StyledMenuContext context(this, state);
    StyleResolver resolver;
    return resolver.Resolve(*m_sheet, context, subControl, state);
}

Style StyledMenu::GetItemStyle(int index) const
{
    Style style = ResolveStyle("item", "");
    if (!m_items[index].enabled || m_items[index].submenu) {
        style.Merge(ResolveStyle("item", "disabled"));
        return style;
    }
    if (index == m_hovered) {
        style.Merge(ResolveStyle("item", "hover"));
    }
    return style;
}

wxFont StyledMenu::GetItemFont(const Style& itemStyle) const
{
    if (itemStyle.font.IsOk()) {
        return itemStyle.font;
    }
    const Style menuStyle = ResolveStyle("", "");
    if (menuStyle.font.IsOk()) {
        return menuStyle.font;
    }
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

int StyledMenu::GetItemHeight(int index) const
{
    if (m_items[index].separator) {
        return std::max(FromDIP(7), 2);
    }

    const Style itemStyle = ResolveStyle("item", "");
    const wxFont font = GetItemFont(itemStyle);
    int textHeight = 0;
    GetTextExtent("Ag", nullptr, &textHeight, nullptr, nullptr,
                  const_cast<wxFont*>(&font));
    return textHeight + itemStyle.paddingTop + itemStyle.paddingBottom;
}

wxRect StyledMenu::GetItemRect(int index) const
{
    const Style menuStyle = ResolveStyle("", "");
    const int x = menuStyle.paddingLeft;
    const int width = GetClientSize().x - menuStyle.paddingLeft - menuStyle.paddingRight;

    int y = menuStyle.paddingTop;
    for (int i = 0; i < index; ++i) {
        y += GetItemHeight(i);
    }
    return wxRect(x, y, std::max(width, 0), GetItemHeight(index));
}

wxSize StyledMenu::CalcMenuSize() const
{
    const Style menuStyle = ResolveStyle("", "");
    const Style itemStyle = ResolveStyle("item", "");
    const wxFont font = GetItemFont(itemStyle);

    const bool hasCheckable = std::any_of(m_items.begin(), m_items.end(),
                                          [](const Item& item) { return item.checkable; });
    const int checkSpace = hasCheckable ? FromDIP(20) : 0;

    int contentWidth = FromDIP(120);
    int height = menuStyle.paddingTop + menuStyle.paddingBottom;
    for (size_t i = 0; i < m_items.size(); ++i) {
        const Item& item = m_items[i];
        height += GetItemHeight(static_cast<int>(i));
        if (item.separator) {
            continue;
        }

        int labelWidth = 0;
        int shortcutWidth = 0;
        GetTextExtent(item.label, &labelWidth, nullptr, nullptr, nullptr,
                      const_cast<wxFont*>(&font));
        if (!item.shortcut.empty()) {
            GetTextExtent(item.shortcut, &shortcutWidth, nullptr, nullptr, nullptr,
                          const_cast<wxFont*>(&font));
        }
        const int width = itemStyle.paddingLeft + checkSpace + labelWidth
                          + FromDIP(24) + shortcutWidth + itemStyle.paddingRight;
        contentWidth = std::max(contentWidth, width);
    }

    wxSize size(contentWidth + menuStyle.paddingLeft + menuStyle.paddingRight, height);
    if (menuStyle.IsSet(Property::MinWidth) && size.x < menuStyle.minWidth) {
        size.x = menuStyle.minWidth;
    }
    return size;
}

int StyledMenu::HitTestItem(const wxPoint& pt) const
{
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (GetItemRect(static_cast<int>(i)).Contains(pt)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

void StyledMenu::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    DrawMenu(dc, GetClientRect());
}

void StyledMenu::DrawMenu(wxDC& dc, const wxRect& rect)
{
    const Style menuStyle = ResolveStyle("", "");

    Painter painter;
    painter.Paint(dc, rect, menuStyle, this);

    const bool hasCheckable = std::any_of(m_items.begin(), m_items.end(),
                                          [](const Item& item) { return item.checkable; });
    const int checkSpace = hasCheckable ? FromDIP(20) : 0;

    const wxColour fallbackText = menuStyle.color.IsOk()
                                      ? menuStyle.color
                                      : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    for (size_t i = 0; i < m_items.size(); ++i) {
        const int index = static_cast<int>(i);
        const Item& item = m_items[i];
        const wxRect itemRect = GetItemRect(index);

        if (item.separator) {
            const Style sepStyle = ResolveStyle("separator", "");
            wxColour line = sepStyle.backgroundColor.IsOk()
                                ? sepStyle.backgroundColor
                                : (menuStyle.borderColor.IsOk() ? menuStyle.borderColor
                                                                : fallbackText);
            dc.SetPen(wxPen(line, 1));
            const int y = itemRect.y + itemRect.height / 2;
            dc.DrawLine(itemRect.x + FromDIP(4), y, itemRect.x + itemRect.width - FromDIP(4),
                        y);
            continue;
        }

        const Style itemStyle = GetItemStyle(index);
        const wxFont font = GetItemFont(itemStyle);
        dc.SetFont(font);

        const wxColour textColour = itemStyle.color.IsOk() ? itemStyle.color : fallbackText;

        // Hover background (inset slightly, like native menus).
        if (index == m_hovered && itemStyle.IsSet(Property::BackgroundColor)
            && itemStyle.backgroundColor.IsOk()) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(itemStyle.backgroundColor));
            dc.DrawRectangle(itemRect.x, itemRect.y, itemRect.width, itemRect.height);
        }

        dc.SetTextForeground(textColour);

        int textWidth = 0;
        int textHeight = 0;
        dc.GetTextExtent(item.label, &textWidth, &textHeight);
        const int textY = itemRect.y + (itemRect.height - textHeight) / 2;

        // Check mark.
        if (item.checkable && item.checked) {
            dc.SetPen(wxPen(textColour, std::max(1, FromDIP(2))));
            const int cx = itemRect.x + FromDIP(5);
            const int cy = itemRect.y + itemRect.height / 2;
            dc.DrawLine(cx, cy, cx + FromDIP(3), cy + FromDIP(3));
            dc.DrawLine(cx + FromDIP(3), cy + FromDIP(3), cx + FromDIP(9), cy - FromDIP(4));
        }

        // Label.
        const int labelX = itemRect.x + itemStyle.paddingLeft + checkSpace;
        dc.DrawText(item.label, labelX, textY);

        // Shortcut, right aligned, slightly dimmed.
        if (!item.shortcut.empty()) {
            int shortcutWidth = 0;
            dc.GetTextExtent(item.shortcut, &shortcutWidth, nullptr);
            const wxColour dimmed(textColour.Red(), textColour.Green(), textColour.Blue(),
                                  160);
            dc.SetTextForeground(dimmed);
            dc.DrawText(item.shortcut,
                        itemRect.x + itemRect.width - itemStyle.paddingRight - shortcutWidth,
                        textY);
            dc.SetTextForeground(textColour);
        }

        // Submenu arrow (chevron).
        if (item.submenu) {
            dc.SetPen(wxPen(textColour, std::max(1, FromDIP(1))));
            const int ax = itemRect.x + itemRect.width - itemStyle.paddingRight - FromDIP(8);
            const int ay = itemRect.y + itemRect.height / 2;
            dc.DrawLine(ax, ay - FromDIP(4), ax + FromDIP(4), ay);
            dc.DrawLine(ax + FromDIP(4), ay, ax, ay + FromDIP(4));
        }
    }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

int StyledMenu::NextSelectable(int from, int delta) const
{
    const int count = static_cast<int>(m_items.size());
    if (count == 0) {
        return -1;
    }

    int index = from;
    for (int steps = 0; steps < count; ++steps) {
        index = ((index + delta) % count + count) % count;
        const Item& item = m_items[index];
        if (!item.separator && item.enabled && !item.submenu) {
            return index;
        }
    }
    return -1;
}

void StyledMenu::MoveHover(int delta)
{
    SetHovered(NextSelectable(m_hovered, delta));
}

void StyledMenu::SetHovered(int index)
{
    if (index >= 0) {
        const Item& item = m_items[index];
        if (item.separator || !item.enabled || item.submenu) {
            index = -1;
        }
    }
    if (index != m_hovered) {
        m_hovered = index;
        Refresh();
    }
}

void StyledMenu::SelectItem(int index)
{
    if (index < 0 || index >= static_cast<int>(m_items.size())) {
        return;
    }
    const Item& item = m_items[index];
    if (item.separator || !item.enabled || item.submenu) {
        return;
    }

    // Same contract as a native menu: a wxEVT_MENU with the item id on the
    // window that opened the menu.
    wxCommandEvent event(wxEVT_MENU, item.id);
    event.SetEventObject(m_owner);
    if (m_owner != nullptr) {
        m_owner->GetEventHandler()->ProcessEvent(event);
    }

    Dismiss();
}

void StyledMenu::OnMouseMove(wxMouseEvent& evt)
{
    SetHovered(HitTestItem(evt.GetPosition()));
    evt.Skip();
}

void StyledMenu::OnLeftDown(wxMouseEvent& evt)
{
    SetHovered(HitTestItem(evt.GetPosition()));
}

void StyledMenu::OnLeftUp(wxMouseEvent& evt)
{
    SelectItem(HitTestItem(evt.GetPosition()));
}

void StyledMenu::OnKeyDown(wxKeyEvent& evt)
{
    switch (evt.GetKeyCode()) {
        case WXK_UP:
        case WXK_NUMPAD_UP:
            MoveHover(-1);
            return;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            MoveHover(1);
            return;
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            SetHovered(NextSelectable(-1, 1));
            return;
        case WXK_END:
        case WXK_NUMPAD_END:
            SetHovered(NextSelectable(0, -1));
            return;
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            SelectItem(m_hovered);
            return;
        case WXK_ESCAPE:
            Dismiss();
            return;
        default:
            break;
    }
    evt.Skip();
}

} // namespace wxCustomization
