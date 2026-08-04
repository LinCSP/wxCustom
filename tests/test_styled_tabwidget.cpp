#include <gtest/gtest.h>

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>
#include <wx/sizer.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/widgets/StyledPanel.h"
#include "wxCustomization/widgets/StyledTabWidget.h"

extern wxFrame* gTestFrame;

using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledButton;
using wxCustomization::StyledPanel;
using wxCustomization::StyledTabWidget;

namespace {

wxColour GetPixel(const wxBitmap& bmp, int x, int y)
{
    wxImage image = bmp.ConvertToImage();
    return wxColour(image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y));
}

class TestTabWidget : public StyledTabWidget {
public:
    explicit TestTabWidget(wxWindow* parent)
        : StyledTabWidget(parent, wxID_ANY)
    {
    }

    using StyledTabWidget::DrawTabWidget;
    using StyledTabWidget::GetPaneRect;
    using StyledTabWidget::GetTabBarRect;
    using StyledTabWidget::GetTabRect;
    using StyledTabWidget::GetTabStyle;
    using StyledTabWidget::HitTestTab;
};

struct PageChangedCatcher {
    void OnPageChanged(wxBookCtrlEvent& evt)
    {
        ++eventCount;
        lastSelection = evt.GetSelection();
        lastOldSelection = evt.GetOldSelection();
    }

    int eventCount = 0;
    int lastSelection = -2;
    int lastOldSelection = -2;
};

TestTabWidget* CreateWithPages(wxWindow* parent, int pageCount)
{
    TestTabWidget* tabs = new TestTabWidget(parent);
    for (int i = 0; i < pageCount; ++i) {
        StyledPanel* page = new StyledPanel(tabs, wxID_ANY);
        tabs->AddPage(page, wxString::Format("Tab %d", i + 1));
    }
    return tabs;
}

} // namespace

TEST(StyledTabWidget, StoresPages)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 2);

    ASSERT_EQ(tabs->GetPageCount(), 2u);
    EXPECT_EQ(tabs->GetPageTitle(0), "Tab 1");
    EXPECT_EQ(tabs->GetPageTitle(1), "Tab 2");
    EXPECT_NE(tabs->GetPage(0), nullptr);
    EXPECT_EQ(tabs->GetPage(5), nullptr);
    EXPECT_EQ(tabs->GetStyledControlType(), "StyledTabWidget");
}

TEST(StyledTabWidget, FirstPageIsSelectedByDefault)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 2);

    EXPECT_EQ(tabs->GetSelection(), 0);
    EXPECT_TRUE(tabs->GetPage(0)->IsShown());
    EXPECT_FALSE(tabs->GetPage(1)->IsShown());
}

TEST(StyledTabWidget, AddPageWithSelectSelectsNewPage)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 1);
    StyledPanel* second = new StyledPanel(tabs, wxID_ANY);
    tabs->AddPage(second, "Second", true);

    EXPECT_EQ(tabs->GetSelection(), 1);
    EXPECT_TRUE(second->IsShown());
    EXPECT_FALSE(tabs->GetPage(0)->IsShown());
}

TEST(StyledTabWidget, SetSelectionReturnsOldSelectionAndSwitchesVisibility)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 3);

    EXPECT_EQ(tabs->SetSelection(2), 0);
    EXPECT_EQ(tabs->GetSelection(), 2);
    EXPECT_FALSE(tabs->GetPage(0)->IsShown());
    EXPECT_FALSE(tabs->GetPage(1)->IsShown());
    EXPECT_TRUE(tabs->GetPage(2)->IsShown());

    // Out-of-range index is ignored.
    EXPECT_EQ(tabs->SetSelection(9), 2);
    EXPECT_EQ(tabs->GetSelection(), 2);
}

TEST(StyledTabWidget, SetSelectionEmitsNotebookEvent)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 3);
    PageChangedCatcher catcher;
    tabs->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &PageChangedCatcher::OnPageChanged, &catcher);

    tabs->SetSelection(1);
    EXPECT_EQ(catcher.eventCount, 1);
    EXPECT_EQ(catcher.lastSelection, 1);
    EXPECT_EQ(catcher.lastOldSelection, 0);

    // Re-selecting the current page emits nothing.
    tabs->SetSelection(1);
    EXPECT_EQ(catcher.eventCount, 1);
}

TEST(StyledTabWidget, SetPageTitleUpdates)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 1);
    tabs->SetPageTitle(0, "New title");
    EXPECT_EQ(tabs->GetPageTitle(0), "New title");
}

TEST(StyledTabWidget, AccessibleRoleIsPageTabList)
{
    StyledTabWidget* tabs = new StyledTabWidget(gTestFrame, wxID_ANY);
    EXPECT_EQ(tabs->GetAccessibleRole(), wxROLE_SYSTEM_PAGETABLIST);
}

TEST(StyledTabWidget, HasMultiplePagesIsTrue)
{
    StyledTabWidget* tabs = new StyledTabWidget(gTestFrame, wxID_ANY);
    EXPECT_TRUE(tabs->HasMultiplePages());
}

TEST(StyledTabWidget, ResolvesSubControlStyles)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTabWidget::tab-bar { background-color: #101010; padding: 2px 8px; }\n"
        "StyledTabWidget::tab { background-color: #202020; color: #303030; padding: 4px 8px; }\n"
        "StyledTabWidget::tab:selected { background-color: #404040; color: #505050; }\n"
        "StyledTabWidget::pane { background-color: #606060; padding: 12px; }"));

    TestTabWidget* tabs = CreateWithPages(gTestFrame, 2);
    tabs->SetStyleSheet(&sheet);

    const Style barStyle = tabs->GetSubControlStyle("tab-bar");
    EXPECT_EQ(barStyle.backgroundColor, wxColour(0x10, 0x10, 0x10));
    EXPECT_EQ(barStyle.paddingLeft, 8);

    const Style tabStyle = tabs->GetSubControlStyle("tab");
    EXPECT_EQ(tabStyle.backgroundColor, wxColour(0x20, 0x20, 0x20));
    EXPECT_EQ(tabStyle.color, wxColour(0x30, 0x30, 0x30));

    const Style paneStyle = tabs->GetSubControlStyle("pane");
    EXPECT_EQ(paneStyle.backgroundColor, wxColour(0x60, 0x60, 0x60));
    EXPECT_EQ(paneStyle.paddingLeft, 12);
}

TEST(StyledTabWidget, SelectedTabUsesSelectedStateStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTabWidget::tab { background-color: #202020; }\n"
        "StyledTabWidget::tab:selected { background-color: #404040; }"));

    TestTabWidget* tabs = CreateWithPages(gTestFrame, 2);
    tabs->SetStyleSheet(&sheet);

    EXPECT_EQ(tabs->GetTabStyle(0).backgroundColor, wxColour(0x40, 0x40, 0x40));
    EXPECT_EQ(tabs->GetTabStyle(1).backgroundColor, wxColour(0x20, 0x20, 0x20));

    tabs->SetSelection(1);
    EXPECT_EQ(tabs->GetTabStyle(0).backgroundColor, wxColour(0x20, 0x20, 0x20));
    EXPECT_EQ(tabs->GetTabStyle(1).backgroundColor, wxColour(0x40, 0x40, 0x40));
}

TEST(StyledTabWidget, KeyboardArrowsChangeSelection)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 3);
    PageChangedCatcher catcher;
    tabs->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &PageChangedCatcher::OnPageChanged, &catcher);

    wxKeyEvent rightEvent(wxEVT_KEY_DOWN);
    rightEvent.m_keyCode = WXK_RIGHT;
    tabs->GetEventHandler()->ProcessEvent(rightEvent);
    EXPECT_EQ(tabs->GetSelection(), 1);
    EXPECT_EQ(catcher.eventCount, 1);

    wxKeyEvent leftEvent(wxEVT_KEY_DOWN);
    leftEvent.m_keyCode = WXK_LEFT;
    tabs->GetEventHandler()->ProcessEvent(leftEvent);
    EXPECT_EQ(tabs->GetSelection(), 0);

    // Arrows wrap around.
    tabs->GetEventHandler()->ProcessEvent(leftEvent);
    EXPECT_EQ(tabs->GetSelection(), 2);
    EXPECT_EQ(catcher.eventCount, 3);
}

TEST(StyledTabWidget, CtrlTabNavigationChangesSelection)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 3);

    wxNavigationKeyEvent forwardEvent;
    forwardEvent.SetWindowChange(true);
    forwardEvent.SetDirection(true);
    tabs->GetEventHandler()->ProcessEvent(forwardEvent);
    EXPECT_EQ(tabs->GetSelection(), 1);

    wxNavigationKeyEvent backwardEvent;
    backwardEvent.SetWindowChange(true);
    backwardEvent.SetDirection(false);
    tabs->GetEventHandler()->ProcessEvent(backwardEvent);
    EXPECT_EQ(tabs->GetSelection(), 0);
}

TEST(StyledTabWidget, MouseClickSelectsTab)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 3);
    tabs->SetSize(0, 0, 400, 200);
    PageChangedCatcher catcher;
    tabs->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &PageChangedCatcher::OnPageChanged, &catcher);

    const wxRect secondTab = tabs->GetTabRect(1);
    ASSERT_GT(secondTab.width, 0);
    const wxPoint center = wxPoint(secondTab.x + secondTab.width / 2,
                                   secondTab.y + secondTab.height / 2);
    EXPECT_EQ(tabs->HitTestTab(center), 1);

    wxMouseEvent downEvent(wxEVT_LEFT_DOWN);
    downEvent.SetPosition(center);
    tabs->GetEventHandler()->ProcessEvent(downEvent);
    EXPECT_EQ(tabs->GetSelection(), 0); // selection changes on release

    wxMouseEvent upEvent(wxEVT_LEFT_UP);
    upEvent.SetPosition(center);
    tabs->GetEventHandler()->ProcessEvent(upEvent);
    EXPECT_EQ(tabs->GetSelection(), 1);
    EXPECT_EQ(catcher.eventCount, 1);
    EXPECT_EQ(catcher.lastSelection, 1);
}

TEST(StyledTabWidget, MouseReleaseOutsideTabDoesNotSelect)
{
    TestTabWidget* tabs = CreateWithPages(gTestFrame, 2);
    tabs->SetSize(0, 0, 400, 200);

    const wxRect secondTab = tabs->GetTabRect(1);
    const wxPoint center = wxPoint(secondTab.x + secondTab.width / 2,
                                   secondTab.y + secondTab.height / 2);

    wxMouseEvent downEvent(wxEVT_LEFT_DOWN);
    downEvent.SetPosition(center);
    tabs->GetEventHandler()->ProcessEvent(downEvent);

    wxMouseEvent upEvent(wxEVT_LEFT_UP);
    upEvent.SetPosition(wxPoint(390, 190)); // pane area, far from the tab
    tabs->GetEventHandler()->ProcessEvent(upEvent);
    EXPECT_EQ(tabs->GetSelection(), 0);
}

TEST(StyledTabWidget, BestSizeIncludesTabBarAndPage)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTabWidget::tab { padding: 4px 8px; }\n"
        "StyledTabWidget::pane { padding: 10px; }"));

    TestTabWidget* tabs = new TestTabWidget(gTestFrame);
    tabs->SetStyleSheet(&sheet);

    StyledPanel* page = new StyledPanel(tabs, wxID_ANY);
    StyledButton* button = new StyledButton(page, wxID_ANY, "Content");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(button, 0, wxALL, 0);
    page->SetSizer(sizer);
    tabs->AddPage(page, "Tab 1");

    const wxSize size = tabs->GetBestSize();
    const wxSize buttonSize = button->GetBestSize();

    EXPECT_GT(size.y, buttonSize.y + tabs->GetTabBarRect().height);
    EXPECT_GT(size.x, buttonSize.x);
}

TEST(StyledTabWidget, SelectedTabIsPaintedWithSelectedColour)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTabWidget::tab-bar { background-color: #cccccc; }\n"
        "StyledTabWidget::tab { background-color: #202020; color: #ffffff; padding: 4px 8px; }\n"
        "StyledTabWidget::tab:selected { background-color: #404040; color: #ffffff; }\n"
        "StyledTabWidget::pane { background-color: #606060; }"));

    TestTabWidget* tabs = CreateWithPages(gTestFrame, 2);
    tabs->SetStyleSheet(&sheet);
    tabs->SetSize(0, 0, 400, 200);

    wxBitmap bitmap(400, 200, 24);
    {
        wxMemoryDC dc(bitmap);
        dc.SetBackground(wxBrush(wxColour(255, 255, 255)));
        dc.Clear();
        tabs->DrawTabWidget(dc, wxRect(0, 0, 400, 200));
    }

    // Selected tab (index 0) uses the :selected background.
    const wxRect firstTab = tabs->GetTabRect(0);
    const wxColour selectedPixel = GetPixel(bitmap, firstTab.x + 3, firstTab.y + 2);
    EXPECT_EQ(selectedPixel, wxColour(0x40, 0x40, 0x40));

    // Unselected tab uses the base background.
    const wxRect secondTab = tabs->GetTabRect(1);
    const wxColour tabPixel = GetPixel(bitmap, secondTab.x + 3, secondTab.y + 2);
    EXPECT_EQ(tabPixel, wxColour(0x20, 0x20, 0x20));

    // Pane area uses the pane background.
    const wxRect pane = tabs->GetPaneRect();
    ASSERT_GT(pane.height, 4);
    const wxColour panePixel = GetPixel(bitmap, pane.x + 2, pane.y + 2);
    EXPECT_EQ(panePixel, wxColour(0x60, 0x60, 0x60));
}
