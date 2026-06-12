#include <gtest/gtest.h>

#include "wxCustomization/widgets/StyledToggleButton.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/frame.h>
#include <wx/tglbtn.h>
#include <wx/event.h>

extern wxFrame* gTestFrame;

using wxCustomization::StyledToggleButton;
using wxCustomization::StyleSheet;

namespace {

void SimulateClick(StyledToggleButton* button)
{
    // Drive the base-class input state machine: enter, press, release.
    wxMouseEvent enter(wxEVT_ENTER_WINDOW);
    button->ProcessWindowEvent(enter);

    wxMouseEvent leftDown(wxEVT_LEFT_DOWN);
    button->ProcessWindowEvent(leftDown);

    wxMouseEvent leftUp(wxEVT_LEFT_UP);
    button->ProcessWindowEvent(leftUp);
}

} // namespace

TEST(StyledToggleButton, TogglesValueOnActivate)
{
    StyledToggleButton* button = new StyledToggleButton(gTestFrame, wxID_ANY, "Toggle");

    EXPECT_FALSE(button->GetValue());

    bool eventReceived = false;
    bool eventValue = false;
    button->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent& evt) {
        eventReceived = true;
        eventValue = evt.GetInt() != 0;
    });

    SimulateClick(button);

    EXPECT_TRUE(button->GetValue());
    EXPECT_TRUE(eventReceived);
    EXPECT_TRUE(eventValue);

    SimulateClick(button);
    EXPECT_FALSE(button->GetValue());
}

TEST(StyledToggleButton, SetValueUpdatesState)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledToggleButton:checked { background-color: #3498db; }"));

    StyledToggleButton* button = new StyledToggleButton(gTestFrame, wxID_ANY, "Toggle");
    button->SetStyleSheet(&sheet);

    button->SetValue(true);
    EXPECT_TRUE(button->GetValue());
    EXPECT_EQ(button->GetCurrentStyle().backgroundColor, wxColour(0x34, 0x98, 0xdb));

    button->SetValue(false);
    EXPECT_FALSE(button->GetValue());
}

TEST(StyledToggleButton, BestSizeAccountsForLabelAndPadding)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledToggleButton { padding: 6px; border-width: 2px; }"));

    StyledToggleButton* button = new StyledToggleButton(gTestFrame, wxID_ANY, "ON");
    button->SetStyleSheet(&sheet);

    const wxSize size = button->GetBestSize();
    EXPECT_GT(size.x, 20);
    EXPECT_GT(size.y, 16);
}
