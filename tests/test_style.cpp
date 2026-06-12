#include <gtest/gtest.h>
#include "wxCustomization/Style.h"

using wxCustomization::Property;
using wxCustomization::Style;

TEST(Style, IsSetInitiallyFalse)
{
    Style s;
    EXPECT_FALSE(s.IsSet(Property::Color));
    EXPECT_FALSE(s.IsSet(Property::BorderWidth));
}

TEST(Style, SetAndIsSet)
{
    Style s;
    s.Set(Property::Color);
    EXPECT_TRUE(s.IsSet(Property::Color));
    EXPECT_FALSE(s.IsSet(Property::BackgroundColor));
}

TEST(Style, MergeOverridesOnlySetProperties)
{
    Style base;
    base.color = wxColour(0, 0, 0);
    base.borderWidth = 2;
    base.Set(Property::Color);
    base.Set(Property::BorderWidth);

    Style override;
    override.color = wxColour(255, 0, 0);
    override.backgroundColor = wxColour(128, 128, 128);
    override.Set(Property::Color);
    override.Set(Property::BackgroundColor);

    base.Merge(override);

    EXPECT_EQ(base.color, wxColour(255, 0, 0));
    EXPECT_EQ(base.backgroundColor, wxColour(128, 128, 128));
    EXPECT_EQ(base.borderWidth, 2);

    EXPECT_TRUE(base.IsSet(Property::Color));
    EXPECT_TRUE(base.IsSet(Property::BackgroundColor));
    EXPECT_TRUE(base.IsSet(Property::BorderWidth));
}

TEST(Style, MergeDoesNotTouchUnsetProperties)
{
    Style base;
    base.borderRadius = 10;
    base.Set(Property::BorderRadius);

    Style override;
    override.paddingTop = 8;
    override.Set(Property::PaddingTop);

    base.Merge(override);

    EXPECT_EQ(base.borderRadius, 10);
    EXPECT_EQ(base.paddingTop, 8);
    EXPECT_FALSE(base.IsSet(Property::Color));
}
