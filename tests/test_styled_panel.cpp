#include <gtest/gtest.h>

#include "wxCustomization/widgets/StyledPanel.h"
#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/frame.h>
#include <wx/sizer.h>

extern wxFrame* gTestFrame;

using wxCustomization::StyledButton;
using wxCustomization::StyledPanel;
using wxCustomization::StyleSheet;

TEST(StyledPanel, IsStyledControl)
{
    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    EXPECT_NE(panel, nullptr);
    EXPECT_EQ(panel->GetStyledControlType(), "StyledPanel");
}

TEST(StyledPanel, DoesNotAcceptFocus)
{
    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    EXPECT_FALSE(panel->AcceptsFocus());
    EXPECT_FALSE(panel->AcceptsFocusFromKeyboard());
}

TEST(StyledPanel, ResolvesPanelStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledPanel { padding: 10px; border-width: 2px; }"));

    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    panel->SetStyleSheet(&sheet);

    const auto& style = panel->GetCurrentStyle();
    EXPECT_EQ(style.paddingTop, 10);
    EXPECT_EQ(style.paddingLeft, 10);
    EXPECT_EQ(style.borderTopWidth, 2);
}

TEST(StyledPanel, BestSizeIncludesChildrenAndPadding)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledPanel { padding: 8px; border-width: 2px; }"
        "StyledButton { padding: 4px; border-width: 1px; min-width: 60px; min-height: 24px; }"));

    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    StyledButton* button = new StyledButton(panel, wxID_ANY, "OK");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(button, 0, wxALL, 0);
    panel->SetSizer(sizer);

    panel->SetStyleSheet(&sheet);
    button->SetStyleSheet(&sheet);

    const wxSize size = panel->GetBestSize();

    // Panel best size must be larger than the child's natural size because
    // of panel padding and border.
    EXPECT_GT(size.x, 40);
    EXPECT_GT(size.y, 24);
}
