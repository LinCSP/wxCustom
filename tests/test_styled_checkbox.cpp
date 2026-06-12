#include <gtest/gtest.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledCheckBox.h"

#include <wx/checkbox.h>
#include <wx/event.h>
#include <wx/frame.h>

extern wxFrame* gTestFrame;

using wxCustomization::CheckState;
using wxCustomization::StyledCheckBox;
using wxCustomization::StyleSheet;

namespace {

class TestCheckBox : public StyledCheckBox {
public:
    TestCheckBox(wxWindow* parent, wxWindowID id, const wxString& label)
        : StyledCheckBox(parent, id, label)
    {
    }

    void PublicActivate() { Activate(); }
};

void SimulateClick(StyledCheckBox* box)
{
    wxMouseEvent enter(wxEVT_ENTER_WINDOW);
    box->ProcessWindowEvent(enter);

    wxMouseEvent leftDown(wxEVT_LEFT_DOWN);
    box->ProcessWindowEvent(leftDown);

    wxMouseEvent leftUp(wxEVT_LEFT_UP);
    box->ProcessWindowEvent(leftUp);
}

} // namespace

TEST(StyledCheckBox, SetValueTogglesBetweenUncheckedAndChecked)
{
    StyledCheckBox* box = new StyledCheckBox(gTestFrame, wxID_ANY, "Check me");

    EXPECT_FALSE(box->GetValue());

    box->SetValue(true);
    EXPECT_TRUE(box->GetValue());
    EXPECT_EQ(box->Get3StateValue(), CheckState::Checked);

    box->SetValue(false);
    EXPECT_FALSE(box->GetValue());
    EXPECT_EQ(box->Get3StateValue(), CheckState::Unchecked);
}

TEST(StyledCheckBox, ThreeStateValuePreservesIndeterminate)
{
    StyledCheckBox* box = new StyledCheckBox(gTestFrame, wxID_ANY, "Check me");

    box->Set3StateValue(CheckState::Indeterminate);
    EXPECT_FALSE(box->GetValue());
    EXPECT_EQ(box->Get3StateValue(), CheckState::Indeterminate);
    EXPECT_TRUE(box->IsIndeterminate());

    box->Set3StateValue(CheckState::Checked);
    EXPECT_TRUE(box->GetValue());
    EXPECT_FALSE(box->IsIndeterminate());
}

TEST(StyledCheckBox, GeneratesCheckboxEventOnActivate)
{
    TestCheckBox* box = new TestCheckBox(gTestFrame, wxID_ANY, "Check me");

    bool eventReceived = false;
    int eventValue = -1;
    box->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& evt) {
        eventReceived = true;
        eventValue = evt.GetInt();
    });

    box->PublicActivate();

    EXPECT_TRUE(eventReceived);
    EXPECT_EQ(eventValue, static_cast<int>(CheckState::Checked));
    EXPECT_TRUE(box->GetValue());

    box->PublicActivate();
    EXPECT_EQ(box->Get3StateValue(), CheckState::Unchecked);
}

TEST(StyledCheckBox, MouseClickTogglesValue)
{
    StyledCheckBox* box = new StyledCheckBox(gTestFrame, wxID_ANY, "Check me");

    EXPECT_FALSE(box->GetValue());

    SimulateClick(box);
    EXPECT_TRUE(box->GetValue());

    SimulateClick(box);
    EXPECT_FALSE(box->GetValue());
}

TEST(StyledCheckBox, ResolvesIndicatorStates)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledCheckBox::indicator { background-color: #ffffff; }\n"
        "StyledCheckBox::indicator:checked { background-color: #ff0000; }\n"
        "StyledCheckBox::indicator:indeterminate { background-color: #00ff00; }\n"
        "StyledCheckBox::indicator:unchecked { background-color: #0000ff; }"));

    StyledCheckBox* box = new StyledCheckBox(gTestFrame, wxID_ANY, "Check me");
    box->SetStyleSheet(&sheet);

    EXPECT_EQ(box->GetSubControlStyle("indicator").backgroundColor,
              wxColour(0, 0, 0xff));

    box->SetValue(true);
    EXPECT_EQ(box->GetSubControlStyle("indicator").backgroundColor,
              wxColour(0xff, 0, 0));

    box->Set3StateValue(CheckState::Indeterminate);
    EXPECT_EQ(box->GetSubControlStyle("indicator").backgroundColor,
              wxColour(0, 0xff, 0));
}

TEST(StyledCheckBox, BestSizeAccountsForIndicatorAndLabel)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledCheckBox { padding: 6px; border-width: 2px; spacing: 6px; }\n"
        "StyledCheckBox::indicator { width: 16px; height: 16px; }"));

    StyledCheckBox* box = new StyledCheckBox(gTestFrame, wxID_ANY, "Option");
    box->SetStyleSheet(&sheet);

    const wxSize size = box->GetBestSize();
    EXPECT_GT(size.x, 40);
    EXPECT_GT(size.y, 16);
}
