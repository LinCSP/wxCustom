#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/notebook.h>

namespace wxCustomization {

/// A styled tab container widget (replacement for wxNotebook).
///
/// Pages are ordinary child windows managed by the widget: the selected page
/// is shown and laid out in the pane area, the other pages are hidden. The
/// widget draws the tab bar and the tabs itself and supports sub-control
/// styling via `::tab-bar`, `::tab` and `::pane`.
///
/// Selection changes (mouse click, arrow keys, Ctrl+Tab or SetSelection)
/// emit a `wxEVT_NOTEBOOK_PAGE_CHANGED` event with the new and the old
/// selection, like wxNotebook does.
class StyledTabWidget : public StyledControl {
public:
    StyledTabWidget(wxWindow* parent,
                    wxWindowID id = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0,
                    const wxString& name = wxNotebookNameStr);

    /// Add a page with the given title. The page must be a child of this
    /// widget (as with wxNotebook). The first added page is selected
    /// automatically; pass @p select = true to select the new page.
    bool AddPage(wxWindow* page, const wxString& title, bool select = false);

    /// Return the number of pages.
    size_t GetPageCount() const { return m_pages.size(); }

    /// Return the page window at @p index or nullptr when out of range.
    wxWindow* GetPage(size_t index) const;

    wxString GetPageTitle(size_t index) const;
    void SetPageTitle(size_t index, const wxString& title);

    /// Return the index of the selected page or -1 when there are no pages.
    int GetSelection() const { return m_selection; }

    /// Select the page at @p index. Returns the previous selection.
    /// Emits `wxEVT_NOTEBOOK_PAGE_CHANGED` when the selection changes.
    int SetSelection(size_t index);

    wxString GetStyledControlType() const override { return "StyledTabWidget"; }

    /// Lets the wxWidgets keyboard navigation code route Ctrl+Tab to this
    /// widget even when the focus is outside of it (same as wxBookCtrlBase).
    bool HasMultiplePages() const override { return true; }

protected:
    void OnPaint(wxPaintEvent& evt) override;
    /// Renders the tab bar, the tabs and the pane into @p rect.
    /// Separated from OnPaint so tests can paint into an arbitrary DC.
    virtual void DrawTabWidget(wxDC& dc, const wxRect& rect);

    void OnLeftDown(wxMouseEvent& evt) override;
    void OnLeftUp(wxMouseEvent& evt) override;
    void OnMotion(wxMouseEvent& evt) override;
    void OnMouseLeave(wxMouseEvent& evt) override;
    void OnKeyDown(wxKeyEvent& evt) override;
    void OnNavigationKey(wxNavigationKeyEvent& evt) override;
    void OnSize(wxSizeEvent& evt);

    wxSize DoGetBestSize() const override;

    /// Return the index of the tab at @p pt (client coordinates) or -1.
    int HitTestTab(const wxPoint& pt) const;

    /// Geometry helpers, all based on the current content rect.
    wxRect GetTabBarRect() const;
    wxRect GetTabRect(int index) const;
    wxRect GetPaneRect() const;
    int GetTabBarHeight() const;

    /// Resolve the style of a single tab with its current transient states
    /// (`:hover`, `:pressed`, `:selected`, `:disabled`) merged in.
    Style GetTabStyle(int index) const;

    /// Move the selection @p delta pages forward/backward (wraps around).
    void ChangeSelection(int delta, bool sendEvent);

private:
    int SetSelectionInternal(int index, bool sendEvent);
    void UpdatePageVisibility();
    void LayoutPage();
    wxSize GetTabSize(int index) const;
    wxFont GetTabFont(int index) const;
    /// Colour behind unstyled areas (pane, tab bar): the style's own
    /// background, then the widget's, then the parent's.
    wxColour GetBehindColour(const Style& style) const;
    /// Width of the tab bar's bottom border (the baseline), 0 when not styled.
    int GetBaselineWidth(const Style& barStyle) const;
    void DrawTabBar(wxDC& dc, const wxRect& rect);
    /// Draw a single tab: rounded top corners, square bottom edge, borders on
    /// top/left/right only. The selected tab extends @p baselineWidth pixels
    /// into the pane so it covers the tab bar's baseline; inactive tabs stop
    /// above it.
    void DrawTab(wxDC& dc, const wxRect& tabRect, const wxString& title,
                 const Style& tabStyle, bool selected, int baselineWidth);
    void SendPageChanged(int oldSelection, int newSelection);

protected:
    struct Page {
        wxWindow* window;
        wxString title;
    };

    std::vector<Page> m_pages;
    int m_selection = -1;
    int m_hoveredTab = -1;
    int m_pressedTab = -1;

private:
    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
