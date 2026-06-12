#include <gtest/gtest.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledRadioButton.h"

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/radiobut.h>

extern wxFrame* gTestFrame;

using wxCustomization::StyledRadioButton;
using wxCustomization::StyleSheet;

namespace {

class TestRadioButton : public StyledRadioButton {
public:
    TestRadioButton(wxWindow* parent, wxWindowID id, const wxString& label,
                    long style = 0)
        : StyledRadioButton(parent, id, label, wxDefaultPosition,
                            wxDefaultSize, style)
    {
    }

    void PublicActivate() { Activate(); }
};

} // namespace

TEST(StyledRadioButton, SetValueTrueUnchecksSiblings)
{
    StyledRadioButton* rb1 = new StyledRadioButton(gTestFrame, wxID_ANY, "One");
    StyledRadioButton* rb2 = new StyledRadioButton(gTestFrame, wxID_ANY, "Two");
    StyledRadioButton* rb3 = new StyledRadioButton(gTestFrame, wxID_ANY, "Three");

    rb1->SetValue(true);
    EXPECT_TRUE(rb1->GetValue());
    EXPECT_FALSE(rb2->GetValue());
    EXPECT_FALSE(rb3->GetValue());

    rb2->SetValue(true);
    EXPECT_FALSE(rb1->GetValue());
    EXPECT_TRUE(rb2->GetValue());
    EXPECT_FALSE(rb3->GetValue());

    rb3->SetValue(true);
    EXPECT_FALSE(rb1->GetValue());
    EXPECT_FALSE(rb2->GetValue());
    EXPECT_TRUE(rb3->GetValue());
}

TEST(StyledRadioButton, GroupBoundariesWithWxRbGroup)
{
    StyledRadioButton* rb1 = new StyledRadioButton(gTestFrame, wxID_ANY, "A1");
    StyledRadioButton* rb2 = new StyledRadioButton(gTestFrame, wxID_ANY, "A2");
    StyledRadioButton* rb3 = new StyledRadioButton(gTestFrame, wxID_ANY, "B1",
                                                    wxDefaultPosition, wxDefaultSize,
                                                    wxRB_GROUP);
    StyledRadioButton* rb4 = new StyledRadioButton(gTestFrame, wxID_ANY, "B2");

    rb1->SetValue(true);
    rb3->SetValue(true);

    EXPECT_TRUE(rb1->GetValue());
    EXPECT_FALSE(rb2->GetValue());
    EXPECT_TRUE(rb3->GetValue());
    EXPECT_FALSE(rb4->GetValue());

    rb2->SetValue(true);
    EXPECT_FALSE(rb1->GetValue());
    EXPECT_TRUE(rb2->GetValue());
    EXPECT_TRUE(rb3->GetValue());
    EXPECT_FALSE(rb4->GetValue());
}

TEST(StyledRadioButton, GeneratesEventOnActivate)
{
    TestRadioButton* rb = new TestRadioButton(gTestFrame, wxID_ANY, "One");

    bool eventReceived = false;
    rb->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent& /*evt*/) {
        eventReceived = true;
    });

    rb->PublicActivate();

    EXPECT_TRUE(eventReceived);
    EXPECT_TRUE(rb->GetValue());
}

TEST(StyledRadioButton, ActivateDoesNotGenerateEventWhenAlreadyChecked)
{
    TestRadioButton* rb = new TestRadioButton(gTestFrame, wxID_ANY, "One");
    rb->SetValue(true);

    int eventCount = 0;
    rb->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent& /*evt*/) { ++eventCount; });

    rb->PublicActivate();

    EXPECT_EQ(eventCount, 0);
    EXPECT_TRUE(rb->GetValue());
}

TEST(StyledRadioButton, ResolvesIndicatorCheckedState)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledRadioButton::indicator { background-color: #ffffff; }\n"
        "StyledRadioButton::indicator:checked { background-color: #3498db; }"));

    StyledRadioButton* rb = new StyledRadioButton(gTestFrame, wxID_ANY, "One");
    rb->SetStyleSheet(&sheet);

    EXPECT_EQ(rb->GetSubControlStyle("indicator").backgroundColor,
              wxColour(0xff, 0xff, 0xff));

    rb->SetValue(true);
    EXPECT_EQ(rb->GetSubControlStyle("indicator").backgroundColor,
              wxColour(0x34, 0x98, 0xdb));
}

TEST(StyledRadioButton, BestSizeAccountsForIndicatorAndLabel)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledRadioButton { padding: 6px; border-width: 2px; spacing: 6px; }\n"
        "StyledRadioButton::indicator { width: 16px; height: 16px; }"));

    StyledRadioButton* rb = new StyledRadioButton(gTestFrame, wxID_ANY, "Option");
    rb->SetStyleSheet(&sheet);

    const wxSize size = rb->GetBestSize();
    EXPECT_GT(size.x, 40);
    EXPECT_GT(size.y, 16);
}
