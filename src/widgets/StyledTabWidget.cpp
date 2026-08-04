#include "wxCustomization/widgets/StyledTabWidget.h"

#include "wxCustomization/Painter.h"

#include <wx/bookctrl.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/sizer.h>

#include <algorithm>

namespace wxCustomization {

wxBEGIN_EVENT_TABLE(StyledTabWidget, StyledControl)
    EVT_SIZE(StyledTabWidget::OnSize)
wxEND_EVENT_TABLE()

StyledTabWidget::StyledTabWidget(wxWindow* parent,
                                 wxWindowID id,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 long style,
                                 const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
{
    SetAccessibleRole(wxROLE_SYSTEM_PAGETABLIST);
}

bool StyledTabWidget::AddPage(wxWindow* page, const wxString& title, bool select)
{
    wxCHECK_MSG(page != nullptr, false, "StyledTabWidget::AddPage: null page");
    wxCHECK_MSG(page->GetParent() == this, false,
                "StyledTabWidget pages must be created as children of the tab widget");

    m_pages.push_back({page, title});

    if (m_selection < 0 || select) {
        SetSelectionInternal(static_cast<int>(m_pages.size()) - 1, true);
    } else {
        page->Show(false);
    }

    InvalidateBestSize();
    Refresh();
    return true;
}

wxWindow* StyledTabWidget::GetPage(size_t index) const
{
    return index < m_pages.size() ? m_pages[index].window : nullptr;
}

wxString StyledTabWidget::GetPageTitle(size_t index) const
{
    return index < m_pages.size() ? m_pages[index].title : wxString();
}

void StyledTabWidget::SetPageTitle(size_t index, const wxString& title)
{
    if (index >= m_pages.size() || m_pages[index].title == title) {
        return;
    }
    m_pages[index].title = title;
    InvalidateBestSize();
    Refresh();
}

int StyledTabWidget::SetSelection(size_t index)
{
    return SetSelectionInternal(static_cast<int>(index), true);
}

int StyledTabWidget::SetSelectionInternal(int index, bool sendEvent)
{
    if (index < 0 || index >= static_cast<int>(m_pages.size()) || index == m_selection) {
        return m_selection;
    }

    const int oldSelection = m_selection;
    m_selection = index;
    UpdatePageVisibility();

    if (sendEvent) {
        SendPageChanged(oldSelection, index);
    }
    return oldSelection;
}

void StyledTabWidget::ChangeSelection(int delta, bool sendEvent)
{
    const int count = static_cast<int>(m_pages.size());
    if (count == 0 || m_selection < 0) {
        return;
    }
    const int next = ((m_selection + delta) % count + count) % count;
    SetSelectionInternal(next, sendEvent);
}

void StyledTabWidget::UpdatePageVisibility()
{
    for (size_t i = 0; i < m_pages.size(); ++i) {
        m_pages[i].window->Show(static_cast<int>(i) == m_selection);
    }
    LayoutPage();
    Refresh();
}

void StyledTabWidget::LayoutPage()
{
    if (m_selection < 0) {
        return;
    }

    const Style paneStyle = GetSubControlStyle("pane");
    wxRect pageRect = GetPaneRect();
    pageRect.x += paneStyle.paddingLeft;
    pageRect.y += paneStyle.paddingTop;
    pageRect.width -= paneStyle.paddingLeft + paneStyle.paddingRight;
    pageRect.height -= paneStyle.paddingTop + paneStyle.paddingBottom;

    if (pageRect.width > 0 && pageRect.height > 0) {
        m_pages[m_selection].window->SetSize(pageRect);
    }
}

void StyledTabWidget::SendPageChanged(int oldSelection, int newSelection)
{
    wxBookCtrlEvent event(wxEVT_NOTEBOOK_PAGE_CHANGED, GetId(), newSelection, oldSelection);
    event.SetEventObject(this);
    ProcessWindowEvent(event);
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

wxSize StyledTabWidget::GetTabSize(int index) const
{
    const Style tabStyle = GetTabStyle(index);
    const wxFont font = GetTabFont(index);

    int textWidth = 0;
    int textHeight = 0;
    GetTextExtent(m_pages[index].title, &textWidth, &textHeight, nullptr, nullptr,
                  const_cast<wxFont*>(&font));

    wxSize size(textWidth + tabStyle.borderLeftWidth + tabStyle.borderRightWidth
                    + tabStyle.paddingLeft + tabStyle.paddingRight,
                textHeight + tabStyle.borderTopWidth + tabStyle.borderBottomWidth
                    + tabStyle.paddingTop + tabStyle.paddingBottom);

    if (tabStyle.IsSet(Property::MinWidth) && size.x < tabStyle.minWidth) {
        size.x = tabStyle.minWidth;
    }
    if (tabStyle.IsSet(Property::MinHeight) && size.y < tabStyle.minHeight) {
        size.y = tabStyle.minHeight;
    }
    return size;
}

int StyledTabWidget::GetTabBarHeight() const
{
    if (m_pages.empty()) {
        return 0;
    }

    const Style barStyle = GetSubControlStyle("tab-bar");

    int height = 0;
    for (size_t i = 0; i < m_pages.size(); ++i) {
        height = std::max(height, GetTabSize(static_cast<int>(i)).y);
    }
    return height + barStyle.paddingTop + barStyle.paddingBottom;
}

wxRect StyledTabWidget::GetTabBarRect() const
{
    const wxRect content = GetContentRect();
    return wxRect(content.x, content.y, content.width, GetTabBarHeight());
}

wxRect StyledTabWidget::GetTabRect(int index) const
{
    const wxRect barRect = GetTabBarRect();
    const Style barStyle = GetSubControlStyle("tab-bar");

    int x = barRect.x + barStyle.paddingLeft;
    for (int i = 0; i < index; ++i) {
        x += GetTabSize(i).x;
    }
    return wxRect(x, barRect.y + barStyle.paddingTop, GetTabSize(index).x,
                  barRect.height - barStyle.paddingTop - barStyle.paddingBottom);
}

wxRect StyledTabWidget::GetPaneRect() const
{
    const wxRect content = GetContentRect();
    const int barHeight = GetTabBarHeight();
    return wxRect(content.x, content.y + barHeight, content.width,
                  std::max(0, content.height - barHeight));
}

int StyledTabWidget::HitTestTab(const wxPoint& pt) const
{
    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (GetTabRect(static_cast<int>(i)).Contains(pt)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

void StyledTabWidget::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    DrawTabWidget(dc, GetClientRect());
}

Style StyledTabWidget::GetTabStyle(int index) const
{
    Style style = GetSubControlStyle("tab");

    if (!IsEnabled()) {
        style.Merge(GetSubControlStyle("tab", "disabled"));
        return style;
    }
    if (index == m_hoveredTab) {
        style.Merge(GetSubControlStyle("tab", "hover"));
    }
    if (index == m_pressedTab) {
        style.Merge(GetSubControlStyle("tab", "pressed"));
    }
    if (index == m_selection) {
        // Selected wins over hover/pressed, like the later QSS rule would.
        style.Merge(GetSubControlStyle("tab", "selected"));
    }
    return style;
}

wxFont StyledTabWidget::GetTabFont(int index) const
{
    const Style tabStyle = GetTabStyle(index);
    if (tabStyle.font.IsOk()) {
        return tabStyle.font;
    }
    const Style& style = GetCurrentStyle();
    if (style.font.IsOk()) {
        return style.font;
    }
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

wxColour StyledTabWidget::GetPaneColour(const Style& paneStyle) const
{
    if (paneStyle.IsSet(Property::BackgroundColor) && paneStyle.backgroundColor.IsOk()) {
        return paneStyle.backgroundColor;
    }
    const Style& style = GetCurrentStyle();
    if (style.IsSet(Property::BackgroundColor) && style.backgroundColor.IsOk()) {
        return style.backgroundColor;
    }
    // The pane area belongs visually to this widget; fall back to the
    // parent's styled background so no stale pixels remain behind the page.
    if (const StyledControl* styledParent = dynamic_cast<const StyledControl*>(GetParent())) {
        const Style& parentStyle = styledParent->GetCurrentStyle();
        if (parentStyle.IsSet(Property::BackgroundColor) && parentStyle.backgroundColor.IsOk()) {
            return parentStyle.backgroundColor;
        }
    }
    if (const wxWindow* parent = GetParent()) {
        return parent->GetBackgroundColour();
    }
    return wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
}

void StyledTabWidget::DrawTabWidget(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();

    Painter painter;

    // Widget background, border and focus outline.
    painter.Paint(dc, rect, style, this);

    if (m_pages.empty()) {
        return;
    }

    // Pane: background colour is painted even when the style has none, so
    // that no stale pixels remain behind the (possibly smaller) page window.
    const wxRect paneRect = GetPaneRect();
    const Style paneStyle = GetSubControlStyle("pane");
    if (paneRect.width > 0 && paneRect.height > 0) {
        painter.Paint(dc, paneRect, paneStyle, this);
        if (!paneStyle.IsSet(Property::BackgroundColor)) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(GetPaneColour(paneStyle));
            dc.DrawRectangle(paneRect);
        }
    }

    // Tab bar.
    const wxRect barRect = GetTabBarRect();
    const Style barStyle = GetSubControlStyle("tab-bar");
    painter.Paint(dc, barRect, barStyle, this);

    // Tabs.
    for (size_t i = 0; i < m_pages.size(); ++i) {
        const int index = static_cast<int>(i);
        const wxRect tabRect = GetTabRect(index);
        Style tabStyle = GetTabStyle(index);
        // Sub-controls inherit the widget font/colour when not styled.
        if (!tabStyle.font.IsOk()) {
            tabStyle.font = GetTabFont(index);
        }
        if (!tabStyle.color.IsOk() && style.color.IsOk()) {
            tabStyle.color = style.color;
        }
        painter.Paint(dc, tabRect, tabStyle, this);
        painter.DrawText(dc, tabRect, m_pages[i].title, tabStyle);
    }

    // Focus indicator around the selected tab.
    if (IsFocused() && m_selection >= 0) {
        painter.DrawOutline(dc, GetTabRect(m_selection), style);
    }
}

wxSize StyledTabWidget::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const Style barStyle = GetSubControlStyle("tab-bar");
    const Style paneStyle = GetSubControlStyle("pane");

    int barWidth = barStyle.paddingLeft + barStyle.paddingRight;
    for (size_t i = 0; i < m_pages.size(); ++i) {
        barWidth += GetTabSize(static_cast<int>(i)).x;
    }

    wxSize pageBest(0, 0);
    for (const Page& page : m_pages) {
        const wxSize best = page.window->GetBestSize();
        pageBest.x = std::max(pageBest.x, best.x);
        pageBest.y = std::max(pageBest.y, best.y);
    }

    wxSize size;
    size.x = std::max(barWidth,
                      pageBest.x + paneStyle.paddingLeft + paneStyle.paddingRight);
    size.y = GetTabBarHeight() + pageBest.y
             + paneStyle.paddingTop + paneStyle.paddingBottom;

    size.x += style.borderLeftWidth + style.borderRightWidth
              + style.paddingLeft + style.paddingRight;
    size.y += style.borderTopWidth + style.borderBottomWidth
              + style.paddingTop + style.paddingBottom;

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

void StyledTabWidget::OnLeftDown(wxMouseEvent& evt)
{
    const int tab = HitTestTab(evt.GetPosition());
    if (tab >= 0) {
        m_pressedTab = tab;
        SetFocus();
        Refresh();
        return;
    }
    evt.Skip();
}

void StyledTabWidget::OnLeftUp(wxMouseEvent& evt)
{
    const int tab = m_pressedTab;
    m_pressedTab = -1;

    if (tab >= 0) {
        if (HitTestTab(evt.GetPosition()) == tab) {
            SetSelectionInternal(tab, true);
        }
        Refresh();
        return;
    }
    evt.Skip();
}

void StyledTabWidget::OnMotion(wxMouseEvent& evt)
{
    const int tab = HitTestTab(evt.GetPosition());
    if (tab != m_hoveredTab) {
        m_hoveredTab = tab;
        Refresh();
    }
    evt.Skip();
}

void StyledTabWidget::OnMouseLeave(wxMouseEvent& evt)
{
    m_hoveredTab = -1;
    m_pressedTab = -1;
    StyledControl::OnMouseLeave(evt);
}

void StyledTabWidget::OnKeyDown(wxKeyEvent& evt)
{
    switch (evt.GetKeyCode()) {
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            ChangeSelection(-1, true);
            return;
        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            ChangeSelection(1, true);
            return;
        default:
            break;
    }
    evt.Skip();
}

void StyledTabWidget::OnNavigationKey(wxNavigationKeyEvent& evt)
{
    if (evt.IsWindowChange()) {
        // Ctrl+Tab / Ctrl+Shift+Tab cycle through the pages.
        ChangeSelection(evt.GetDirection() ? 1 : -1, true);
        return;
    }
    StyledControl::OnNavigationKey(evt);
}

void StyledTabWidget::OnSize(wxSizeEvent& evt)
{
    LayoutPage();
    evt.Skip();
}

} // namespace wxCustomization
