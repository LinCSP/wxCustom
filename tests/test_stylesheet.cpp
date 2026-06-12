#include <gtest/gtest.h>
#include "wxCustomization/StyleSheet.h"

using wxCustomization::StyleSheet;
using wxCustomization::StyleRule;
using wxCustomization::SelectorSequence;
using wxCustomization::SimpleSelector;
using wxCustomization::Declaration;

TEST(StyleSheet, ParseEmpty)
{
    StyleSheet sheet;
    EXPECT_TRUE(sheet.LoadFromString(""));
    EXPECT_TRUE(sheet.GetRules().empty());
}

TEST(StyleSheet, ParseSimpleRule)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString("StyledButton { color: red; }"));
    ASSERT_EQ(sheet.GetRules().size(), 1u);

    const StyleRule& rule = sheet.GetRules()[0];
    ASSERT_EQ(rule.selectors.size(), 1u);
    EXPECT_EQ(rule.selectors[0].parts[0].type, "StyledButton");

    ASSERT_EQ(rule.declarations.size(), 1u);
    EXPECT_EQ(rule.declarations[0].property, "color");
    EXPECT_EQ(rule.declarations[0].value, "red");
}

TEST(StyleSheet, ParseMultipleDeclarations)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton {\n"
        "    background-color: #3498db;\n"
        "    border: 1px solid #2980b9;\n"
        "}\n"));

    ASSERT_EQ(sheet.GetRules().size(), 1u);
    const StyleRule& rule = sheet.GetRules()[0];
    ASSERT_EQ(rule.declarations.size(), 2u);
    EXPECT_EQ(rule.declarations[0].property, "background-color");
    EXPECT_EQ(rule.declarations[0].value, "#3498db");
    EXPECT_EQ(rule.declarations[1].property, "border");
    EXPECT_EQ(rule.declarations[1].value, "1px solid #2980b9");
}

TEST(StyleSheet, ParseSelectors)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton, .primary, #ok { padding: 8dip; }"));

    const StyleRule& rule = sheet.GetRules()[0];
    ASSERT_EQ(rule.selectors.size(), 3u);
    EXPECT_EQ(rule.selectors[0].parts[0].type, "StyledButton");
    EXPECT_EQ(rule.selectors[1].parts[0].className, "primary");
    EXPECT_EQ(rule.selectors[2].parts[0].id, "ok");
}

TEST(StyleSheet, ParsePseudoAndSubControl)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledCheckBox:hover::indicator { background-color: #fff; }"));

    const StyleRule& rule = sheet.GetRules()[0];
    ASSERT_EQ(rule.selectors.size(), 1u);
    const SimpleSelector& sel = rule.selectors[0].parts[0];
    EXPECT_EQ(sel.type, "StyledCheckBox");
    EXPECT_EQ(sel.pseudo, "hover");
    EXPECT_EQ(sel.subControl, "indicator");
}

TEST(StyleSheet, ParseAttributeSelector)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton[kind=\"danger\"] { color: red; }"));

    const StyleRule& rule = sheet.GetRules()[0];
    const SimpleSelector& sel = rule.selectors[0].parts[0];
    EXPECT_EQ(sel.type, "StyledButton");
    EXPECT_EQ(sel.attrName, "kind");
    EXPECT_EQ(sel.attrValue, "danger");
}

TEST(StyleSheet, ParseVariables)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        ":root {\n"
        "    --primary: #3498db;\n"
        "    --text: #2c3e50;\n"
        "}\n"
        "StyledButton { background-color: var(--primary); }"));

    EXPECT_EQ(sheet.GetVariable("--primary"), "#3498db");
    EXPECT_EQ(sheet.GetVariable("--text"), "#2c3e50");
    ASSERT_EQ(sheet.GetRules().size(), 2u);
}

TEST(StyleSheet, ParseNestedOrInvalidFails)
{
    StyleSheet sheet;
    EXPECT_FALSE(sheet.LoadFromString("StyledButton { color: red"));
}
