#include <gtest/gtest.h>
#include "wxCustomization/Color.h"

using wxCustomization::Color;

TEST(Color, HexRgb)
{
    wxColour c = Color::Parse("#abc");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 0xaa);
    EXPECT_EQ(c.Green(), 0xbb);
    EXPECT_EQ(c.Blue(), 0xcc);
    EXPECT_EQ(c.Alpha(), wxALPHA_OPAQUE);
}

TEST(Color, HexRrggbb)
{
    wxColour c = Color::Parse("#1a2b3c");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 0x1a);
    EXPECT_EQ(c.Green(), 0x2b);
    EXPECT_EQ(c.Blue(), 0x3c);
}

TEST(Color, HexAarrggbb)
{
    wxColour c = Color::Parse("#801a2b3c");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Alpha(), 0x80);
    EXPECT_EQ(c.Red(), 0x1a);
    EXPECT_EQ(c.Green(), 0x2b);
    EXPECT_EQ(c.Blue(), 0x3c);
}

TEST(Color, RgbFunction)
{
    wxColour c = Color::Parse("rgb(10, 20, 30)");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 10);
    EXPECT_EQ(c.Green(), 20);
    EXPECT_EQ(c.Blue(), 30);
}

TEST(Color, RgbPercent)
{
    wxColour c = Color::Parse("rgb(100%, 50%, 0%)");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 255);
    EXPECT_EQ(c.Green(), 128);
    EXPECT_EQ(c.Blue(), 0);
}

TEST(Color, RgbaFunction)
{
    wxColour c = Color::Parse("rgba(10, 20, 30, 0.5)");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 10);
    EXPECT_EQ(c.Green(), 20);
    EXPECT_EQ(c.Blue(), 30);
    EXPECT_EQ(c.Alpha(), 128);
}

TEST(Color, HsvRed)
{
    wxColour c = Color::Parse("hsv(0, 100%, 100%)");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 255);
    EXPECT_EQ(c.Green(), 0);
    EXPECT_EQ(c.Blue(), 0);
}

TEST(Color, HsvGreen)
{
    wxColour c = Color::Parse("hsv(120, 100%, 100%)");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 0);
    EXPECT_EQ(c.Green(), 255);
    EXPECT_EQ(c.Blue(), 0);
}

TEST(Color, HsvBlue)
{
    wxColour c = Color::Parse("hsv(240, 100%, 100%)");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 0);
    EXPECT_EQ(c.Green(), 0);
    EXPECT_EQ(c.Blue(), 255);
}

TEST(Color, NamedRed)
{
    wxColour c = Color::Parse("red");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Red(), 255);
    EXPECT_EQ(c.Green(), 0);
    EXPECT_EQ(c.Blue(), 0);
}

TEST(Color, NamedTransparent)
{
    wxColour c = Color::Parse("transparent");
    ASSERT_TRUE(c.IsOk());
    EXPECT_EQ(c.Alpha(), 0);
}

TEST(Color, InvalidInput)
{
    EXPECT_FALSE(Color::Parse("").IsOk());
    EXPECT_FALSE(Color::Parse("notacolor").IsOk());
    EXPECT_FALSE(Color::Parse("#zzzzzz").IsOk());
    EXPECT_FALSE(Color::Parse("rgb(1, 2)").IsOk());
}
