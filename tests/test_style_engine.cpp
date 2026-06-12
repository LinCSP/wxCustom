#include <gtest/gtest.h>

#include "wxCustomization/Color.h"
#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"
#include "wxCustomization/StyleResolver.h"
#include "wxCustomization/StyleResolverContext.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>

extern wxFrame* gTestFrame;

using wxCustomization::BackgroundRepeat;
using wxCustomization::BorderStyle;
using wxCustomization::Painter;
using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleResolver;
using wxCustomization::StyleResolverContext;
using wxCustomization::StyleSheet;

namespace {

class TestContext : public StyleResolverContext {
public:
    wxString type;
    wxString id;
    std::vector<wxString> classes;
    std::map<wxString, wxString> props;
    wxString state;

    wxString GetControlType() const override { return type; }
    wxString GetControlId() const override { return id; }
    std::vector<wxString> GetControlClasses() const override { return classes; }
    bool HasPseudoState(const wxString& s) const override { return s == state; }
    wxString GetDynamicProperty(const wxString& name) const override {
        auto it = props.find(name);
        return it != props.end() ? it->second : wxString();
    }
    const wxWindow* GetWindow() const override { return gTestFrame; }
};

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

TEST(StyleEngine, ParseAndResolveBasicProperties)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        ":root { --bg: #eeeeee; }\n"
        "StyledButton {\n"
        "  color: #111111;\n"
        "  background-color: var(--bg);\n"
        "  border-width: 2px;\n"
        "  border-color: #000000;\n"
        "  border-style: solid;\n"
        "  border-radius: 4px;\n"
        "  padding: 8px;\n"
        "  opacity: 0.9;\n"
        "}\n"
        "StyledButton:hover { background-color: #ffffff; }\n"
        "#ok { color: #222222; }"));

    TestContext ctx;
    ctx.type = "StyledButton";
    ctx.id = "ok";
    ctx.classes = {"btn"};

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.color, wxColour(0x22, 0x22, 0x22));
    EXPECT_EQ(style.backgroundColor, wxColour(0xee, 0xee, 0xee));
    EXPECT_EQ(style.borderWidth, 2);
    EXPECT_EQ(style.borderStyle, BorderStyle::Solid);
    EXPECT_EQ(style.borderRadius, 4);
    EXPECT_EQ(style.paddingTop, 8);
    EXPECT_EQ(style.opacity, 0.9);
}

TEST(StyleEngine, SpecificityAndOrder)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { color: #000000; }\n"
        ".btn { color: #111111; }\n"
        "StyledButton.btn { color: #222222; }\n"
        "#ok { color: #333333; }"));

    TestContext ctx;
    ctx.type = "StyledButton";
    ctx.id = "ok";
    ctx.classes = {"btn"};

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.color, wxColour(0x33, 0x33, 0x33));
}

TEST(StyleEngine, PseudoStateAndVariables)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        ":root { --accent: #3498db; }\n"
        "StyledButton { background-color: #ffffff; }\n"
        "StyledButton:hover { background-color: var(--accent); }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style normal = resolver.Resolve(sheet, ctx);
    EXPECT_EQ(normal.backgroundColor, wxColour(0xff, 0xff, 0xff));

    ctx.state = "hover";
    Style hover = resolver.Resolve(sheet, ctx, wxEmptyString, "hover");
    EXPECT_EQ(hover.backgroundColor, wxColour(0x34, 0x98, 0xdb));
}

TEST(StyleEngine, MergeOfIndependentProperties)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { color: #ff0000; }\n"
        "StyledButton { background-color: #00ff00; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.color, wxColour(0xff, 0, 0));
    EXPECT_EQ(style.backgroundColor, wxColour(0, 0xff, 0));
}

TEST(StyleEngine, FullPipelineWithPaint)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "TestWidget {\n"
        "  background-color: #123456;\n"
        "  border-width: 2px;\n"
        "  border-color: #abcdef;\n"
        "  border-style: solid;\n"
        "  outline-width: 0px;\n"
        "}"));

    TestContext ctx;
    ctx.type = "TestWidget";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    wxBitmap bmp(32, 32, 24);
    FillWhite(bmp);

    wxMemoryDC dc(bmp);
    Painter painter;
    painter.Paint(dc, wxRect(0, 0, 32, 32), style);

    EXPECT_EQ(GetPixel(bmp, 16, 16), wxColour(0x12, 0x34, 0x56));
    EXPECT_EQ(GetPixel(bmp, 0, 16), wxColour(0xab, 0xcd, 0xef));
}

TEST(StyleEngine, DynamicPropertySelector)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton[kind=\"primary\"] { color: #0000ff; }\n"
        "StyledButton { color: #000000; }"));

    TestContext ctx;
    ctx.type = "StyledButton";
    ctx.props["kind"] = "primary";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.color, wxColour(0, 0, 0xff));
}

TEST(StyleEngine, BorderAndOutlineProperties)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledPanel {\n"
        "  border-width: 2px;\n"
        "  border-style: dashed;\n"
        "  border-color: #ff0000;\n"
        "  outline-width: 3px;\n"
        "  outline-color: #00ff00;\n"
        "  outline-offset: 2px;\n"
        "}"));

    TestContext ctx;
    ctx.type = "StyledPanel";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.borderWidth, 2);
    EXPECT_EQ(style.borderStyle, BorderStyle::Dashed);
    EXPECT_EQ(style.borderColor, wxColour(0xff, 0, 0));
    EXPECT_EQ(style.outlineWidth, 3);
    EXPECT_EQ(style.outlineColor, wxColour(0, 0xff, 0));
    EXPECT_EQ(style.outlineOffset, 2);
}
