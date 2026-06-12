#include <gtest/gtest.h>
#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

using wxCustomization::Painter;
using wxCustomization::Property;
using wxCustomization::Style;

namespace {

wxColour GetPixel(const wxBitmap& bmp, int x, int y)
{
    wxImage image = bmp.ConvertToImage();
    return wxColour(image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y));
}

void FillWhite(wxBitmap& bmp)
{
    wxMemoryDC dc(bmp);
    dc.SetBackground(wxBrush(wxColour(255, 255, 255)));
    dc.Clear();
}

} // namespace

TEST(Painter, BackgroundColor)
{
    wxBitmap bmp(32, 32, 24);
    FillWhite(bmp);

    Style style;
    style.backgroundColor = wxColour(255, 0, 0);
    style.Set(Property::BackgroundColor);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.Paint(dc, wxRect(0, 0, 32, 32), style);

    EXPECT_EQ(GetPixel(bmp, 16, 16), wxColour(255, 0, 0));
}

TEST(Painter, Border)
{
    wxBitmap bmp(32, 32, 24);
    FillWhite(bmp);

    Style style;
    style.backgroundColor = wxColour(255, 255, 255);
    style.borderColor = wxColour(0, 0, 255);
    style.borderStyle = wxCustomization::BorderStyle::Solid;
    style.borderWidth = 4;
    style.borderTopWidth = 4;
    style.borderRightWidth = 4;
    style.borderBottomWidth = 4;
    style.borderLeftWidth = 4;
    style.borderRadius = 4;
    style.Set(Property::BackgroundColor);
    style.Set(Property::BorderColor);
    style.Set(Property::BorderStyle);
    style.Set(Property::BorderWidth);
    style.Set(Property::BorderTopWidth);
    style.Set(Property::BorderRightWidth);
    style.Set(Property::BorderBottomWidth);
    style.Set(Property::BorderLeftWidth);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.Paint(dc, wxRect(0, 0, 32, 32), style);

    EXPECT_EQ(GetPixel(bmp, 0, 16), wxColour(0, 0, 255));
    EXPECT_EQ(GetPixel(bmp, 16, 16), wxColour(255, 255, 255));
}

TEST(Painter, OutlineOutsideBorder)
{
    wxBitmap bmp(48, 48, 24);
    FillWhite(bmp);

    Style style;
    style.outlineColor = wxColour(0, 255, 0);
    style.outlineWidth = 4;
    style.outlineOffset = 2;
    style.Set(Property::OutlineColor);
    style.Set(Property::OutlineWidth);
    style.Set(Property::OutlineOffset);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.Paint(dc, wxRect(8, 8, 32, 32), style);

    EXPECT_EQ(GetPixel(bmp, 4, 24), wxColour(0, 255, 0));
    EXPECT_EQ(GetPixel(bmp, 24, 24), wxColour(255, 255, 255));
}

TEST(Painter, GradientLinear)
{
    wxBitmap bmp(64, 32, 24);
    FillWhite(bmp);

    wxCustomization::Gradient grad;
    grad.type = wxCustomization::GradientType::Linear;
    grad.direction = wxEAST;
    grad.startColor = wxColour(255, 0, 0);
    grad.endColor = wxColour(0, 0, 255);

    Style style;
    style.backgroundGradient = grad;
    style.Set(Property::BackgroundGradient);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.DrawBackground(dc, wxRect(0, 0, 64, 32), style);

    wxColour left = GetPixel(bmp, 4, 16);
    wxColour right = GetPixel(bmp, 59, 16);

    EXPECT_GT(left.Red(), 200);
    EXPECT_LT(left.Blue(), 50);
    EXPECT_GT(right.Blue(), 200);
    EXPECT_LT(right.Red(), 50);
}

TEST(Painter, OpacityBlendsWithBackground)
{
    wxBitmap bmp(32, 32, 24);
    FillWhite(bmp);

    Style style;
    style.backgroundColor = wxColour(255, 0, 0);
    style.opacity = 0.5;
    style.Set(Property::BackgroundColor);
    style.Set(Property::Opacity);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.DrawBackground(dc, wxRect(0, 0, 32, 32), style);

    wxColour center = GetPixel(bmp, 16, 16);
    EXPECT_EQ(center.Red(), 255);
    EXPECT_GT(center.Green(), 100);
    EXPECT_LT(center.Green(), 200);
    EXPECT_GT(center.Blue(), 100);
    EXPECT_LT(center.Blue(), 200);
}

TEST(Painter, BackgroundImageNoRepeat)
{
    wxBitmap imageBmp(4, 4, 24);
    {
        wxMemoryDC dc(imageBmp);
        dc.SetBackground(wxBrush(wxColour(255, 0, 0)));
        dc.Clear();
    }

    wxBitmap bmp(32, 32, 24);
    FillWhite(bmp);

    Style style;
    style.backgroundImage = imageBmp;
    style.backgroundPosition = wxPoint(4, 4);
    style.backgroundRepeat = wxCustomization::BackgroundRepeat::NoRepeat;
    style.Set(Property::BackgroundImage);
    style.Set(Property::BackgroundPosition);
    style.Set(Property::BackgroundRepeat);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.DrawBackground(dc, wxRect(0, 0, 32, 32), style);

    EXPECT_EQ(GetPixel(bmp, 5, 5), wxColour(255, 0, 0));
    EXPECT_EQ(GetPixel(bmp, 20, 20), wxColour(255, 255, 255));
}

TEST(Painter, DrawTextDoesNotCrash)
{
    wxBitmap bmp(64, 32, 24);
    FillWhite(bmp);

    Style style;
    style.color = wxColour(0, 0, 0);
    style.textAlign = wxCustomization::TextAlign::Center;
    style.Set(Property::Color);
    style.Set(Property::TextAlign);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.DrawText(dc, wxRect(0, 0, 64, 32), "Hello", style);
    SUCCEED();
}
