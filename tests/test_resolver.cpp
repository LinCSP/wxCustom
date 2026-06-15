#include <gtest/gtest.h>
#include "wxCustomization/StyleResolver.h"
#include "wxCustomization/StyleResolverContext.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/frame.h>

extern wxFrame* gTestFrame;

using wxCustomization::Style;
using wxCustomization::StyleResolver;
using wxCustomization::StyleResolverContext;
using wxCustomization::StyleSheet;

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

TEST(StyleResolver, SpecificityIdBeatsClass)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        ".btn { color: #000; }\n"
        "#ok { color: #fff; }"));

    TestContext ctx;
    ctx.type = "StyledButton";
    ctx.classes = {"btn"};
    ctx.id = "ok";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.color, wxColour(0xff, 0xff, 0xff));
}

TEST(StyleResolver, LaterRuleWinsWhenSpecificityEqual)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { color: #000; }\n"
        "StyledButton { color: #fff; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.color, wxColour(0xff, 0xff, 0xff));
}

TEST(StyleResolver, PseudoState)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { color: #000; }\n"
        "StyledButton:hover { color: #fff; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style normal = resolver.Resolve(sheet, ctx);
    EXPECT_EQ(normal.color, wxColour(0, 0, 0));

    Style hover = resolver.Resolve(sheet, ctx, wxEmptyString, "hover");
    EXPECT_EQ(hover.color, wxColour(0xff, 0xff, 0xff));
}

TEST(StyleResolver, DynamicProperty)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton[kind=\"danger\"] { color: red; }\n"
        "StyledButton { color: black; }"));

    TestContext ctx;
    ctx.type = "StyledButton";
    ctx.props["kind"] = "danger";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);
    EXPECT_EQ(style.color, wxColour(255, 0, 0));
}

TEST(StyleResolver, CssVariables)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        ":root { --primary: #3498db; }\n"
        "StyledButton { background-color: var(--primary); }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);
    EXPECT_EQ(style.backgroundColor, wxColour(0x34, 0x98, 0xdb));
}

TEST(StyleResolver, SubControl)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledCheckBox::indicator { background-color: white; }\n"
        "StyledCheckBox { background-color: black; }"));

    TestContext ctx;
    ctx.type = "StyledCheckBox";

    StyleResolver resolver;
    Style base = resolver.Resolve(sheet, ctx);
    EXPECT_EQ(base.backgroundColor, wxColour(0, 0, 0));

    Style indicator = resolver.Resolve(sheet, ctx, "indicator");
    EXPECT_EQ(indicator.backgroundColor, wxColour(0xff, 0xff, 0xff));
}

TEST(StyleResolver, PaddingShorthand)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { padding: 1px 2px 3px 4px; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.paddingTop, 1);
    EXPECT_EQ(style.paddingRight, 2);
    EXPECT_EQ(style.paddingBottom, 3);
    EXPECT_EQ(style.paddingLeft, 4);
}

TEST(StyleResolver, MarginShorthand)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { margin: 5px 10px; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.marginTop, 5);
    EXPECT_EQ(style.marginRight, 10);
    EXPECT_EQ(style.marginBottom, 5);
    EXPECT_EQ(style.marginLeft, 10);
}

TEST(StyleResolver, SpacingProperty)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { spacing: 12px; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.spacing, 12);
}

TEST(StyleResolver, BorderRadiusPercentageIsStoredAsNegative)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { border-radius: 50%; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.borderRadius, -50);
}

TEST(StyleResolver, BorderRadiusPixelValueIsStoredAsPixels)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { border-radius: 8px; }"));

    TestContext ctx;
    ctx.type = "StyledButton";

    StyleResolver resolver;
    Style style = resolver.Resolve(sheet, ctx);

    EXPECT_EQ(style.borderRadius, 8);
}
