#include <gtest/gtest.h>
#include <iostream>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledButton.h"

#include <wx/frame.h>

extern wxFrame* gTestFrame;

using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledButton;

namespace {

class EventCatcher {
public:
    int clickCount = 0;
    void OnClick(wxCommandEvent&) { ++clickCount; }
};

class TestButton : public StyledButton {
public:
    TestButton(wxWindow* parent, wxWindowID id, const wxString& label)
        : StyledButton(parent, id, label)
    {
    }

    void PublicActivate() { Activate(); }
};

} // namespace

TEST(StyledButton, GeneratesButtonEventOnActivate)
{
    TestButton* button = new TestButton(gTestFrame, wxID_ANY, "Click me");
    EventCatcher catcher;
    button->Bind(wxEVT_BUTTON, &EventCatcher::OnClick, &catcher);

    button->PublicActivate();

    EXPECT_EQ(catcher.clickCount, 1);
}

TEST(StyledButton, ResolvesHoverState)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { background-color: #ffffff; }\n"
        "StyledButton:hover { background-color: #000000; }"));

    StyledButton* button = new StyledButton(gTestFrame, wxID_ANY, "Test");
    button->SetStyleSheet(&sheet);

    EXPECT_EQ(button->GetCurrentStyle().backgroundColor, wxColour(0xff, 0xff, 0xff));

    button->ApplyStyle("hover");
    EXPECT_EQ(button->GetCurrentStyle().backgroundColor, wxColour(0, 0, 0));
}

TEST(StyledButton, ResolvesPressedAndFocusedStates)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { background-color: #ffffff; }\n"
        "StyledButton:pressed { background-color: #111111; }\n"
        "StyledButton:focused { outline-width: 3px; outline-color: #222222; }"));

    StyledButton* button = new StyledButton(gTestFrame, wxID_ANY, "Test");
    button->SetStyleSheet(&sheet);

    button->ApplyStyle("pressed");
    EXPECT_EQ(button->GetCurrentStyle().backgroundColor, wxColour(0x11, 0x11, 0x11));

    button->ApplyStyle("focused");
    EXPECT_EQ(button->GetCurrentStyle().outlineWidth, 3);
    EXPECT_EQ(button->GetCurrentStyle().outlineColor, wxColour(0x22, 0x22, 0x22));
}

TEST(StyledButton, ClickGeneratesButtonEvent)
{
    TestButton* button = new TestButton(gTestFrame, wxID_ANY, "Click me");
    button->SetSize(0, 0, 100, 40);
    EventCatcher catcher;
    button->Bind(wxEVT_BUTTON, &EventCatcher::OnClick, &catcher);

    const wxPoint center(50, 20);
    wxMouseEvent enterEvent(wxEVT_ENTER_WINDOW);
    button->GetEventHandler()->ProcessEvent(enterEvent);

    wxMouseEvent downEvent(wxEVT_LEFT_DOWN);
    downEvent.SetPosition(center);
    button->GetEventHandler()->ProcessEvent(downEvent);

    wxMouseEvent upEvent(wxEVT_LEFT_UP);
    upEvent.SetPosition(center);
    button->GetEventHandler()->ProcessEvent(upEvent);

    EXPECT_EQ(catcher.clickCount, 1);
}

TEST(StyledButton, SpaceAndEnterGenerateButtonEvent)
{
    TestButton* button = new TestButton(gTestFrame, wxID_ANY, "Click me");
    EventCatcher catcher;
    button->Bind(wxEVT_BUTTON, &EventCatcher::OnClick, &catcher);

    wxKeyEvent spaceEvent(wxEVT_CHAR);
    spaceEvent.m_keyCode = WXK_SPACE;
    button->GetEventHandler()->ProcessEvent(spaceEvent);

    wxKeyEvent enterEvent(wxEVT_CHAR);
    enterEvent.m_keyCode = WXK_RETURN;
    button->GetEventHandler()->ProcessEvent(enterEvent);

    EXPECT_EQ(catcher.clickCount, 2);
}

TEST(StyledButton, DisabledState)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { color: #000000; }\n"
        "StyledButton:disabled { color: #888888; }"));

    StyledButton* button = new StyledButton(gTestFrame, wxID_ANY, "Test");
    button->SetStyleSheet(&sheet);

    EXPECT_EQ(button->GetCurrentStyle().color, wxColour(0, 0, 0));

    button->Enable(false);
    EXPECT_EQ(button->GetCurrentStyle().color, wxColour(0x88, 0x88, 0x88));
}

TEST(StyledButton, BestSizeAccountsForLabelAndPadding)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledButton { padding: 6px; border-width: 2px; }"));

    StyledButton* button = new StyledButton(gTestFrame, wxID_ANY, "OK");
    button->SetStyleSheet(&sheet);

    const wxSize size = button->GetBestSize();
    EXPECT_GT(size.x, 28);
    EXPECT_GT(size.y, 24);

    const Style& style = button->GetCurrentStyle();
    EXPECT_GT(style.borderWidth, 0);
    EXPECT_GT(style.paddingTop, 0);
    EXPECT_GT(style.paddingLeft, 0);
}
