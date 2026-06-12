#include <gtest/gtest.h>
#include "wxCustomization/StyleValue.h"

using wxCustomization::Gradient;
using wxCustomization::GradientType;
using wxCustomization::Length;
using wxCustomization::StyleValue;

TEST(StyleValue, DefaultIsInvalid)
{
    StyleValue v;
    EXPECT_FALSE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::None);
}

TEST(StyleValue, Color)
{
    StyleValue v = StyleValue::FromColor(wxColour(10, 20, 30));
    ASSERT_TRUE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::Color);
    wxColour c = v.AsColor();
    EXPECT_EQ(c.Red(), 10);
    EXPECT_EQ(c.Green(), 20);
    EXPECT_EQ(c.Blue(), 30);
}

TEST(StyleValue, Length)
{
    StyleValue v = StyleValue::FromLength(Length::Parse("16px"));
    ASSERT_TRUE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::Length);
    EXPECT_EQ(v.AsLength(nullptr), 16);
}

TEST(StyleValue, Number)
{
    StyleValue v = StyleValue::FromNumber(3.14);
    ASSERT_TRUE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::Number);
    EXPECT_DOUBLE_EQ(v.AsNumber(), 3.14);
}

TEST(StyleValue, String)
{
    StyleValue v = StyleValue::FromString("hello");
    ASSERT_TRUE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::String);
    EXPECT_EQ(v.AsString(), "hello");
}

TEST(StyleValue, Enum)
{
    StyleValue v = StyleValue::FromEnum("solid");
    ASSERT_TRUE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::Enum);
    EXPECT_EQ(v.AsEnum(), "solid");
}

TEST(StyleValue, ImageReturnsDefaultWhenNotImage)
{
    StyleValue v = StyleValue::FromNumber(1.0);
    EXPECT_FALSE(v.AsImage().IsOk());
}

TEST(StyleValue, Gradient)
{
    Gradient g;
    g.type = GradientType::Linear;
    g.startColor = wxColour(0, 0, 0);
    g.endColor = wxColour(255, 255, 255);

    StyleValue v = StyleValue::FromGradient(g);
    ASSERT_TRUE(v.IsValid());
    EXPECT_EQ(v.GetType(), StyleValue::Type::Gradient);
    EXPECT_EQ(v.AsGradient(), g);
}

TEST(StyleValue, WrongTypeReturnsDefault)
{
    StyleValue v = StyleValue::FromNumber(42.0);
    EXPECT_EQ(v.AsColor(), wxNullColour);
    EXPECT_EQ(v.AsString(), wxString());
    EXPECT_EQ(v.AsEnum(), wxString());
}
