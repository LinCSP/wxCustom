#include <gtest/gtest.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledButton.h"

#include <wx/frame.h>

extern wxFrame* gTestFrame;

using wxCustomization::Property;
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
        "StyledButton { padding: 8px; border-width: 2px; }"));

    StyledButton* button = new StyledButton(gTestFrame, wxID_ANY, "OK");
    button->SetStyleSheet(&sheet);

    const wxSize size = button->GetBestSize();
    EXPECT_GT(size.x, 20);
    EXPECT_GT(size.y, 20);
}
