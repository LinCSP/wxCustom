#include <gtest/gtest.h>

#include "wxCustomization/widgets/StyledLabel.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/frame.h>

extern wxFrame* gTestFrame;

using wxCustomization::StyledLabel;
using wxCustomization::StyleSheet;
using wxCustomization::TextAlign;
using wxCustomization::TextDecoration;

TEST(StyledLabel, IsStyledControl)
{
    StyledLabel* label = new StyledLabel(gTestFrame, wxID_ANY, "Hello");
    EXPECT_NE(label, nullptr);
    EXPECT_EQ(label->GetStyledControlType(), "StyledLabel");
}

TEST(StyledLabel, DoesNotAcceptFocus)
{
    StyledLabel* label = new StyledLabel(gTestFrame, wxID_ANY, "Hello");
    EXPECT_FALSE(label->AcceptsFocus());
    EXPECT_FALSE(label->AcceptsFocusFromKeyboard());
}

TEST(StyledLabel, SetAndGetLabel)
{
    StyledLabel* label = new StyledLabel(gTestFrame, wxID_ANY, "A");
    EXPECT_EQ(label->GetLabel(), "A");

    label->SetLabel("B");
    EXPECT_EQ(label->GetLabel(), "B");
}

TEST(StyledLabel, BestSizeAccountsForPaddingAndBorder)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledLabel { padding: 6px; border-width: 2px; }"));

    StyledLabel* label = new StyledLabel(gTestFrame, wxID_ANY, "OK");
    label->SetStyleSheet(&sheet);

    const wxSize size = label->GetBestSize();
    EXPECT_GT(size.x, 20);
    EXPECT_GT(size.y, 16);

    const auto& style = label->GetCurrentStyle();
    EXPECT_EQ(style.paddingTop, 6);
    EXPECT_EQ(style.borderTopWidth, 2);
}

TEST(StyledLabel, TextAlignAndDecorationResolved)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledLabel { text-align: right; text-decoration: underline; }"));

    StyledLabel* label = new StyledLabel(gTestFrame, wxID_ANY, "OK");
    label->SetStyleSheet(&sheet);

    const auto& style = label->GetCurrentStyle();
    EXPECT_EQ(style.textAlign, TextAlign::Right);
    EXPECT_EQ(style.textDecoration, TextDecoration::Underline);
}
