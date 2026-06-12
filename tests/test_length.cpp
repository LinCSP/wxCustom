#include <gtest/gtest.h>
#include "wxCustomization/Length.h"

#include <wx/frame.h>
#include <cmath>

extern wxFrame* gTestFrame;

using wxCustomization::Length;

TEST(Length, ParsePx)
{
    Length l = Length::Parse("12px");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Px);
    EXPECT_DOUBLE_EQ(l.GetValue(), 12.0);
    EXPECT_EQ(l.ToPixels(nullptr), 12);
}

TEST(Length, ParseDip)
{
    Length l = Length::Parse("16dip");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Dip);
    EXPECT_DOUBLE_EQ(l.GetValue(), 16.0);
    EXPECT_EQ(l.ToPixels(gTestFrame), gTestFrame->FromDIP(16));
}

TEST(Length, ParseDp)
{
    Length l = Length::Parse("8dp");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Dp);
    EXPECT_DOUBLE_EQ(l.GetValue(), 8.0);
    EXPECT_EQ(l.ToPixels(gTestFrame), gTestFrame->FromDIP(8));
}

TEST(Length, ParsePt)
{
    Length l = Length::Parse("12pt");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Pt);
    const int expected = static_cast<int>(std::round(12.0 * gTestFrame->GetDPI().GetHeight() / 72.0));
    EXPECT_EQ(l.ToPixels(gTestFrame), expected);
}

TEST(Length, ParseEm)
{
    Length l = Length::Parse("1.5em");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Em);
    EXPECT_EQ(l.ToPixels(nullptr, 20), 30);
}

TEST(Length, ParsePercent)
{
    Length l = Length::Parse("50%");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Percent);
    EXPECT_EQ(l.ToPixels(nullptr, 0, 200), 100);
}

TEST(Length, ParseNoUnitDefaultsToPx)
{
    Length l = Length::Parse("42");
    ASSERT_TRUE(l.IsValid());
    EXPECT_EQ(l.GetUnit(), Length::Px);
    EXPECT_EQ(l.ToPixels(nullptr), 42);
}

TEST(Length, InvalidInput)
{
    EXPECT_FALSE(Length::Parse("abc").IsValid());
    EXPECT_FALSE(Length::Parse("px").IsValid());
    EXPECT_FALSE(Length::Parse("").IsValid());
}

TEST(Length, Equality)
{
    EXPECT_EQ(Length::Parse("10px"), Length::Parse("10px"));
    EXPECT_NE(Length::Parse("10px"), Length::Parse("11px"));
    EXPECT_NE(Length::Parse("10px"), Length::Parse("10dip"));
}
