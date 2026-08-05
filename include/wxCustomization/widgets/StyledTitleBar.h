#pragma once

#include "wxCustomization/StyledControl.h"

class wxMenu;
class wxTopLevelWindow;

namespace wxCustomization {

/// A styled window title bar used by StyledFrame (client-side decorations).
///
/// Draws the window title (sub-control `::title`) and the caption buttons
/// minimize/maximize/close (sub-controls `::minimize-button`,
/// `::maximize-button`, `::close-button`, sharing the generic
/// `::caption-button`). Handles window dragging by the bar and maximize on
/// double-click. The caption buttons act on the top-level window the bar
/// belongs to (Iconize/Maximize/Close).
///
/// Menus can be embedded on the left side of the bar (VSCode-style):
/// AddMenu() adds a drawn menu button (sub-control `::menu-button`) which
/// opens its wxMenu on click.
class StyledTitleBar : public StyledControl {
public:
    StyledTitleBar(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxControlNameStr);

    void SetTitle(const wxString& title);
    wxString GetTitle() const { return m_title; }

    /// Add a menu button with the given @p label on the left side of the bar.
    /// The @p menu is shown via PopupMenu() when the button is clicked.
    /// The bar does not take ownership of the menu.
    void AddMenu(const wxString& label, wxMenu* menu);

    /// Number of embedded menus.
    size_t GetMenuCount() const { return m_menus.size(); }

    wxString GetStyledControlType() const override { return "StyledTitleBar"; }

    /// The bar itself is not focusable; caption buttons are mouse-driven.
    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

protected:
    /// Caption button indices; Close is the rightmost one.
    enum CaptionButton {
        BtnMinimize = 0,
        BtnMaximize = 1,
        BtnClose = 2,
        BtnCount = 3
    };

    void OnPaint(wxPaintEvent& evt) override;
    /// Renders the bar into @p rect. Separated from OnPaint so tests can
    /// paint into an arbitrary DC.
    virtual void DrawTitleBar(wxDC& dc, const wxRect& rect);

    void OnLeftDown(wxMouseEvent& evt) override;
    void OnLeftUp(wxMouseEvent& evt) override;
    void OnMotion(wxMouseEvent& evt) override;
    void OnMouseLeave(wxMouseEvent& evt) override;
    void OnLeftDClick(wxMouseEvent& evt);

    wxSize DoGetBestSize() const override;

    /// Return the caption button at @p pt (client coords) or -1.
    int HitTestCaptionButton(const wxPoint& pt) const;
    wxRect GetCaptionButtonRect(int index) const;
    Style GetCaptionButtonStyle(int index) const;

    /// Return the menu button at @p pt (client coords) or -1.
    int HitTestMenuButton(const wxPoint& pt) const;
    wxRect GetMenuButtonRect(int index) const;
    Style GetMenuButtonStyle(int index) const;

    /// Action for a caption button; virtual so tests can intercept it.
    virtual void ActivateCaptionButton(int index);

    /// Open the popup menu for the menu button @p index. The default
    /// implementation shows the native wxMenu below the button; virtual so
    /// tests can intercept it.
    virtual void ShowMenu(int index);

    static wxString CaptionButtonKind(int index);

protected:
    struct MenuEntry {
        wxString label;
        wxMenu* menu;
    };

    wxString m_title;
    int m_hoveredButton = -1;
    int m_pressedButton = -1;

    std::vector<MenuEntry> m_menus;
    int m_hoveredMenuButton = -1;
    int m_openMenuButton = -1;

private:
    wxTopLevelWindow* GetTopLevelWindow() const;
    wxFont GetTitleFont() const;
    int GetCaptionButtonsWidth() const;
    int GetMenuButtonsWidth() const;
    void DrawCaptionButton(wxDC& dc, int index);
    void DrawMenuButton(wxDC& dc, int index);

    bool m_dragging = false;
    wxPoint m_dragOffset;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
