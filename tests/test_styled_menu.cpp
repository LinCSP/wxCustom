#include <gtest/gtest.h>

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>
#include <wx/menu.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledMenu.h"

extern wxFrame* gTestFrame;

using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledMenu;

namespace {

class TestStyledMenu : public StyledMenu {
public:
    TestStyledMenu(wxWindow* owner, StyleSheet* sheet)
        : StyledMenu(owner, sheet)
    {
    }

    using StyledMenu::DrawMenu;
    using StyledMenu::GetHoverIndex;
    using StyledMenu::GetItem;
    using StyledMenu::GetItemRect;
    using StyledMenu::GetItemStyle;
    using StyledMenu::HitTestItem;
    using StyledMenu::SelectItem;
};

// Persistent catcher: bindings on gTestFrame must not reference dead objects.
struct MenuEventCatcher {
    void OnMenu(wxCommandEvent& evt)
    {
        ++count;
        lastId = evt.GetId();
    }

    int count = 0;
    int lastId = -1;
};

MenuEventCatcher g_menuCatcher;

wxMenu* CreateSampleMenu()
{
    wxMenu* menu = new wxMenu();
    menu->Append(101, "New\tCtrl+N");
    menu->AppendCheckItem(102, "Enabled option");
    menu->Check(102, true);
    menu->AppendSeparator();
    menu->Append(103, "Disabled item");
    menu->Enable(103, false);
    menu->Append(104, "Quit\tCtrl+Q");
    return menu;
}

StyleSheet* CreateMenuSheet()
{
    static StyleSheet sheet;
    static bool loaded = false;
    if (!loaded) {
        const bool ok = sheet.LoadFromString(
            "StyledMenu { background-color: #ffffff; color: #101010; padding: 4px; }\n"
            "StyledMenu::item { color: #101010; padding: 5px 12px; }\n"
            "StyledMenu::item:hover { background-color: #0066cc; color: #ffffff; }\n"
            "StyledMenu::item:disabled { color: #999999; }\n"
            "StyledMenu::separator { background-color: #cccccc; }");
        EXPECT_TRUE(ok);
        loaded = true;
    }
    return &sheet;
}

wxColour GetPixel(const wxBitmap& bmp, int x, int y)
{
    wxImage image = bmp.ConvertToImage();
    return wxColour(image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y));
}

} // namespace

TEST(StyledMenu, BuildsModelFromWxMenu)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);

    ASSERT_EQ(menu.GetItemCount(), 5u);

    EXPECT_EQ(menu.GetItem(0).label, "New");
    EXPECT_EQ(menu.GetItem(0).shortcut, "Ctrl+N");
    EXPECT_EQ(menu.GetItem(0).id, 101);
    EXPECT_TRUE(menu.GetItem(0).enabled);

    EXPECT_TRUE(menu.GetItem(1).checkable);
    EXPECT_TRUE(menu.GetItem(1).checked);

    EXPECT_TRUE(menu.GetItem(2).separator);

    EXPECT_EQ(menu.GetItem(3).label, "Disabled item");
    EXPECT_FALSE(menu.GetItem(3).enabled);

    EXPECT_EQ(menu.GetItem(4).shortcut, "Ctrl+Q");

    delete source;
}

TEST(StyledMenu, ItemRectsAreStacked)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);
    menu.SetSize(200, 160);

    const wxRect first = menu.GetItemRect(0);
    const wxRect second = menu.GetItemRect(1);

    EXPECT_EQ(second.y, first.y + first.height);
    EXPECT_GT(first.height, 10);
    EXPECT_GT(first.width, 100);

    delete source;
}

TEST(StyledMenu, HitTestFindsItems)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);
    menu.SetSize(200, 160);

    const wxRect rect = menu.GetItemRect(1);
    EXPECT_EQ(menu.HitTestItem(wxPoint(rect.x + 2, rect.y + 2)), 1);
    EXPECT_EQ(menu.HitTestItem(wxPoint(-50, -50)), -1);

    delete source;
}

TEST(StyledMenu, HoveredItemUsesHoverStyle)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);
    menu.SetSize(200, 160);

    // Hover via keyboard navigation: first selectable item.
    wxKeyEvent downEvent(wxEVT_KEY_DOWN);
    downEvent.m_keyCode = WXK_DOWN;
    menu.GetEventHandler()->ProcessEvent(downEvent);

    EXPECT_EQ(menu.GetHoverIndex(), 0);
    EXPECT_EQ(menu.GetItemStyle(0).backgroundColor, wxColour(0x00, 0x66, 0xcc));
    EXPECT_EQ(menu.GetItemStyle(1).color, wxColour(0x10, 0x10, 0x10));

    // Third Down lands on "Quit": the separator and the disabled item are
    // skipped by hover navigation.
    menu.GetEventHandler()->ProcessEvent(downEvent);
    EXPECT_EQ(menu.GetHoverIndex(), 1);
    menu.GetEventHandler()->ProcessEvent(downEvent);
    EXPECT_EQ(menu.GetHoverIndex(), 4);

    delete source;
}

TEST(StyledMenu, DisabledItemUsesDisabledStyle)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);

    EXPECT_EQ(menu.GetItemStyle(3).color, wxColour(0x99, 0x99, 0x99));

    delete source;
}

TEST(StyledMenu, SelectItemEmitsMenuEventOnOwner)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);

    g_menuCatcher.count = 0;
    g_menuCatcher.lastId = -1;
    gTestFrame->Bind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 101);

    menu.SelectItem(0);
    EXPECT_EQ(g_menuCatcher.count, 1);
    EXPECT_EQ(g_menuCatcher.lastId, 101);

    gTestFrame->Unbind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 101);
    delete source;
}

TEST(StyledMenu, SelectDisabledOrSeparatorDoesNothing)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);

    g_menuCatcher.count = 0;
    gTestFrame->Bind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 103);
    gTestFrame->Bind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 104);

    menu.SelectItem(2); // separator
    menu.SelectItem(3); // disabled
    EXPECT_EQ(g_menuCatcher.count, 0);

    gTestFrame->Unbind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 103);
    gTestFrame->Unbind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 104);
    delete source;
}

TEST(StyledMenu, EnterKeySelectsHoveredItem)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);

    g_menuCatcher.count = 0;
    g_menuCatcher.lastId = -1;
    gTestFrame->Bind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 101);

    wxKeyEvent downEvent(wxEVT_KEY_DOWN);
    downEvent.m_keyCode = WXK_DOWN;
    menu.GetEventHandler()->ProcessEvent(downEvent);
    EXPECT_EQ(menu.GetHoverIndex(), 0);

    wxKeyEvent enterEvent(wxEVT_KEY_DOWN);
    enterEvent.m_keyCode = WXK_RETURN;
    menu.GetEventHandler()->ProcessEvent(enterEvent);
    EXPECT_EQ(g_menuCatcher.lastId, 101);

    gTestFrame->Unbind(wxEVT_MENU, &MenuEventCatcher::OnMenu, &g_menuCatcher, 101);
    delete source;
}

TEST(StyledMenu, RendersHoverBackgroundAndSeparator)
{
    wxMenu* source = CreateSampleMenu();
    TestStyledMenu menu(gTestFrame, CreateMenuSheet());
    menu.BuildFromMenu(source);
    menu.SetSize(220, 160);

    wxKeyEvent downEvent(wxEVT_KEY_DOWN);
    downEvent.m_keyCode = WXK_DOWN;
    menu.GetEventHandler()->ProcessEvent(downEvent);
    ASSERT_EQ(menu.GetHoverIndex(), 0);

    wxBitmap bitmap(220, 160, 24);
    {
        wxMemoryDC dc(bitmap);
        dc.SetBackground(wxBrush(wxColour(255, 255, 255)));
        dc.Clear();
        menu.DrawMenu(dc, wxRect(0, 0, 220, 160));
    }

    // Hovered first item: hover background.
    const wxRect firstRect = menu.GetItemRect(0);
    const wxColour hoverPixel = GetPixel(bitmap, firstRect.x + 2, firstRect.y + 2);
    EXPECT_EQ(hoverPixel, wxColour(0x00, 0x66, 0xcc));

    // Separator line in the middle of the separator rect.
    const wxRect sepRect = menu.GetItemRect(2);
    const wxColour sepPixel = GetPixel(bitmap, sepRect.x + 20, sepRect.y + sepRect.height / 2);
    EXPECT_EQ(sepPixel, wxColour(0xcc, 0xcc, 0xcc));

    delete source;
}
