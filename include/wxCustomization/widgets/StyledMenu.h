#pragma once

#include "wxCustomization/Style.h"

#include <wx/popupwin.h>

#include <functional>
#include <vector>

class wxMenu;

namespace wxCustomization {

class StyleSheet;

/// A styled popup menu (replacement for native wxMenu popups).
///
/// The item model is built from an existing wxMenu (labels, shortcuts,
/// enabled/checked flags, separators), so application code does not change.
/// Selecting an item emits a regular `wxEVT_MENU` command event on the owner
/// window, exactly like a native menu would.
///
/// Styling uses the `StyledMenu` selector with sub-controls `::item`
/// (`:hover`, `:disabled`) and `::separator`.
///
/// Limitations (v1): submenus are rendered with an arrow but do not open.
class StyledMenu : public wxPopupTransientWindow {
public:
    /// @p owner receives the wxEVT_MENU events; @p sheet provides the styles.
    StyledMenu(wxWindow* owner, StyleSheet* sheet);

    /// Mirror the items of @p menu (labels, shortcuts, enabled, checked,
    /// separators). Does not take ownership.
    void BuildFromMenu(const wxMenu* menu);

    /// Show the menu with its top-left corner at @p screenPos.
    void PopupAt(const wxPoint& screenPos);

    /// Called when the popup is dismissed (selection or click outside).
    void SetDismissHandler(std::function<void()> handler);

    size_t GetItemCount() const { return m_items.size(); }

protected:
    void OnDismiss() override;

    // Exposed for tests.
    struct Item {
        wxString label;
        wxString shortcut;
        int id = 0;
        bool enabled = true;
        bool separator = false;
        bool checkable = false;
        bool checked = false;
        bool submenu = false;
    };

    const Item& GetItem(int index) const { return m_items[index]; }
    wxRect GetItemRect(int index) const;
    int HitTestItem(const wxPoint& pt) const;
    Style GetItemStyle(int index) const;
    virtual void DrawMenu(wxDC& dc, const wxRect& rect);
    int GetHoverIndex() const { return m_hovered; }

    void OnPaint(wxPaintEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnKeyDown(wxKeyEvent& evt);

    /// Activate an item: emit wxEVT_MENU on the owner and dismiss.
    virtual void SelectItem(int index);

private:
    Style ResolveStyle(const wxString& subControl, const wxString& state) const;
    wxFont GetItemFont(const Style& itemStyle) const;
    int GetItemHeight(int index) const;
    wxSize CalcMenuSize() const;
    void MoveHover(int delta);
    int NextSelectable(int from, int delta) const;
    void SetHovered(int index);

    wxWindow* m_owner;
    StyleSheet* m_sheet;
    std::vector<Item> m_items;
    int m_hovered = -1;
    std::function<void()> m_dismissHandler;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
