#include <gtest/gtest.h>
#include "wxCustomization/Style.h"
#include "wxCustomization/StyledControl.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/frame.h>

extern wxFrame* gTestFrame;

using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledControl;

namespace {

class TestStyledControl : public StyledControl {
public:
    TestStyledControl(wxWindow* parent, wxWindowID id,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize)
        : StyledControl(parent, id, pos, size)
    {
    }

    wxString GetStyledControlType() const override { return "TestWidget"; }
    wxString GetStyledControlId() const override { return "test"; }

    int activateCount = 0;

    bool PublicAcceptsFocus() const { return AcceptsFocus(); }
    bool PublicAcceptsFocusFromKeyboard() const { return AcceptsFocusFromKeyboard(); }
    wxRect PublicGetContentRect() const { return GetContentRect(); }

protected:
    void Activate() override { ++activateCount; }
};

} // namespace

TEST(StyledControl, InheritsFromWxControl)
{
    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    EXPECT_TRUE(ctrl != nullptr);
    EXPECT_TRUE(ctrl->IsKindOf(wxCLASSINFO(wxControl)));
}

TEST(StyledControl, StyleSheetResolution)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "TestWidget { background-color: #ff0000; }\n"
        "TestWidget:hover { background-color: #00ff00; }"));

    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    ctrl->SetStyleSheet(&sheet);

    EXPECT_EQ(ctrl->GetCurrentStyle().backgroundColor, wxColour(255, 0, 0));

    ctrl->ApplyStyle("hover");
    EXPECT_EQ(ctrl->GetCurrentStyle().backgroundColor, wxColour(0, 255, 0));
}

TEST(StyledControl, ClassSelector)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        ".active { color: #123456; }"));

    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    ctrl->AddStyleClass("active");
    ctrl->SetStyleSheet(&sheet);

    EXPECT_EQ(ctrl->GetCurrentStyle().color, wxColour(0x12, 0x34, 0x56));
}

TEST(StyledControl, DynamicPropertySelector)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "TestWidget[kind=\"danger\"] { color: red; }"));

    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    ctrl->SetStyleSheet(&sheet);
    EXPECT_NE(ctrl->GetCurrentStyle().color, wxColour(255, 0, 0));

    ctrl->SetStyleProperty("kind", "danger");
    EXPECT_EQ(ctrl->GetCurrentStyle().color, wxColour(255, 0, 0));
}

TEST(StyledControl, PseudoStateFocused)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "TestWidget:focused { outline-width: 2px; }"));

    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    ctrl->SetStyleSheet(&sheet);

    EXPECT_FALSE(ctrl->GetCurrentStyle().IsSet(Property::OutlineWidth));

    ctrl->ApplyStyle("focused");
    EXPECT_TRUE(ctrl->GetCurrentStyle().IsSet(Property::OutlineWidth));
}

TEST(StyledControl, Focusability)
{
    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    EXPECT_TRUE(ctrl->PublicAcceptsFocus());
    EXPECT_TRUE(ctrl->PublicAcceptsFocusFromKeyboard());

    ctrl->Enable(false);
    EXPECT_FALSE(ctrl->PublicAcceptsFocus());
    EXPECT_FALSE(ctrl->PublicAcceptsFocusFromKeyboard());
}

TEST(StyledControl, AccessibilityLabelAndRole)
{
    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY);
    ctrl->SetLabel("Original Label");
    ctrl->SetAccessibleLabel("Accessible Label");
    ctrl->SetAccessibleRole(wxROLE_SYSTEM_PUSHBUTTON);

    EXPECT_EQ(ctrl->GetAccessibleLabel(), "Accessible Label");
    EXPECT_EQ(ctrl->GetAccessibleRole(), wxROLE_SYSTEM_PUSHBUTTON);
}

TEST(StyledControl, ContentRectAccountsForBorderAndPadding)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "TestWidget { border-width: 4px; padding: 6px; }"));

    TestStyledControl* ctrl = new TestStyledControl(gTestFrame, wxID_ANY,
                                                     wxDefaultPosition, wxSize(200, 200));
    ctrl->SetStyleSheet(&sheet);

    const Style& style = ctrl->GetCurrentStyle();
    EXPECT_EQ(style.borderTopWidth, 4);
    EXPECT_EQ(style.paddingTop, 6);

    const wxRect client = ctrl->GetClientRect();
    const wxRect content = ctrl->PublicGetContentRect();

    const int expectedX = client.x + style.borderLeftWidth + style.paddingLeft;
    const int expectedY = client.y + style.borderTopWidth + style.paddingTop;
    const int expectedW = client.width - style.borderLeftWidth - style.borderRightWidth
                          - style.paddingLeft - style.paddingRight;
    const int expectedH = client.height - style.borderTopWidth - style.borderBottomWidth
                          - style.paddingTop - style.paddingBottom;

    EXPECT_EQ(content.x, expectedX);
    EXPECT_EQ(content.y, expectedY);
    EXPECT_EQ(content.width, expectedW);
    EXPECT_EQ(content.height, expectedH);
}
