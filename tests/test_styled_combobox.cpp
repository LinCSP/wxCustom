#include <gtest/gtest.h>

#include <wx/frame.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledComboBox.h"

extern wxFrame* gTestFrame;

using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledComboBox;

namespace {

class EventCatcher {
public:
    int selectionCount = 0;
    int lastSelection = -1;
    wxString lastValue;

    void OnSelect(wxCommandEvent& evt)
    {
        ++selectionCount;
        lastSelection = evt.GetInt();
        lastValue = evt.GetString();
    }
};

class TestComboBox : public StyledComboBox {
public:
    TestComboBox(wxWindow* parent,
                 wxWindowID id,
                 const wxArrayString& choices = wxArrayString())
        : StyledComboBox(parent, id, choices)
    {
    }

    void PublicMoveSelection(int delta) { MoveSelection(delta); }
};

} // namespace

TEST(StyledComboBox, AppendAndCount)
{
    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY);
    EXPECT_EQ(combo->GetCount(), 0u);

    combo->Append("First");
    EXPECT_EQ(combo->GetCount(), 1u);

    combo->Append("Second");
    EXPECT_EQ(combo->GetCount(), 2u);
}

TEST(StyledComboBox, Clear)
{
    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY);
    combo->Append("One");
    combo->Append("Two");
    combo->SetSelection(1);

    combo->Clear();

    EXPECT_EQ(combo->GetCount(), 0u);
    EXPECT_EQ(combo->GetSelection(), -1);
    EXPECT_TRUE(combo->GetValue().empty());
}

TEST(StyledComboBox, SetSelectionAndGetValue)
{
    wxArrayString choices;
    choices.Add("Alpha");
    choices.Add("Beta");
    choices.Add("Gamma");

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY, choices);
    EXPECT_EQ(combo->GetSelection(), -1);
    EXPECT_TRUE(combo->GetValue().empty());

    combo->SetSelection(1);
    EXPECT_EQ(combo->GetSelection(), 1);
    EXPECT_EQ(combo->GetValue(), "Beta");

    combo->SetSelection(-1);
    EXPECT_EQ(combo->GetSelection(), -1);
}

TEST(StyledComboBox, SetSelectionBounds)
{
    wxArrayString choices;
    choices.Add("Only");

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY, choices);
    combo->SetSelection(5);
    EXPECT_EQ(combo->GetSelection(), -1);

    combo->SetSelection(0);
    EXPECT_EQ(combo->GetSelection(), 0);
}

TEST(StyledComboBox, GeneratesComboBoxEvent)
{
    wxArrayString choices;
    choices.Add("One");
    choices.Add("Two");

    TestComboBox* combo = new TestComboBox(gTestFrame, wxID_ANY, choices);
    EventCatcher catcher;
    combo->Bind(wxEVT_COMBOBOX, &EventCatcher::OnSelect, &catcher);

    combo->SetSelection(0);
    combo->PublicMoveSelection(1);

    EXPECT_EQ(catcher.selectionCount, 1);
    EXPECT_EQ(catcher.lastSelection, 1);
    EXPECT_EQ(catcher.lastValue, "Two");
}

TEST(StyledComboBox, NoEventOnSameSelection)
{
    wxArrayString choices;
    choices.Add("One");
    choices.Add("Two");

    TestComboBox* combo = new TestComboBox(gTestFrame, wxID_ANY, choices);
    EventCatcher catcher;
    combo->Bind(wxEVT_COMBOBOX, &EventCatcher::OnSelect, &catcher);

    combo->SetSelection(1);
    EXPECT_EQ(catcher.selectionCount, 0);

    combo->PublicMoveSelection(-1);
    EXPECT_EQ(catcher.selectionCount, 1);
    EXPECT_EQ(catcher.lastSelection, 0);

    // Moving to the same selection should not emit another event.
    combo->PublicMoveSelection(0);
    EXPECT_EQ(catcher.selectionCount, 1);
}

TEST(StyledComboBox, ResolvesDropDownSubControlStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledComboBox::drop-down { background-color: #123456; width: 30px; }\n"
        "StyledComboBox::down-arrow { color: #abcdef; }"));

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY);
    combo->SetStyleSheet(&sheet);

    const Style dropDownStyle = combo->GetSubControlStyle("drop-down");
    EXPECT_EQ(dropDownStyle.backgroundColor, wxColour(0x12, 0x34, 0x56));
    EXPECT_EQ(dropDownStyle.width, 30);

    const Style arrowStyle = combo->GetSubControlStyle("down-arrow");
    EXPECT_EQ(arrowStyle.color, wxColour(0xab, 0xcd, 0xef));
}

TEST(StyledComboBox, ResolvesItemSubControlStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledComboBox::item { background-color: #ffffff; color: #000000; }\n"
        "StyledComboBox::item:hover { background-color: #eeeeee; }\n"
        "StyledComboBox::item:selected { background-color: #3498db; color: #ffffff; }"));

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY);
    combo->SetStyleSheet(&sheet);

    const Style itemStyle = combo->GetSubControlStyle("item");
    EXPECT_EQ(itemStyle.backgroundColor, wxColour(0xff, 0xff, 0xff));
    EXPECT_EQ(itemStyle.color, wxColour(0, 0, 0));

    const Style hoverStyle = combo->GetSubControlStyle("item", "hover");
    EXPECT_EQ(hoverStyle.backgroundColor, wxColour(0xee, 0xee, 0xee));

    const Style selectedStyle = combo->GetSubControlStyle("item", "selected");
    EXPECT_EQ(selectedStyle.backgroundColor, wxColour(0x34, 0x98, 0xdb));
    EXPECT_EQ(selectedStyle.color, wxColour(0xff, 0xff, 0xff));
}

TEST(StyledComboBox, BestSizeIsNonZero)
{
    wxArrayString choices;
    choices.Add("Short");
    choices.Add("A much longer choice");

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY, choices);
    const wxSize size = combo->GetBestSize();

    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

TEST(StyledComboBox, BestSizeAccountsForDropDown)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledComboBox::drop-down { width: 40px; }"));

    wxArrayString choices;
    choices.Add("Text");

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY, choices);
    combo->SetStyleSheet(&sheet);

    const wxSize sizeWithWideDropDown = combo->GetBestSize();

    StyleSheet sheetDefault;
    ASSERT_TRUE(sheetDefault.LoadFromString("StyledComboBox { }"));

    StyledComboBox* comboDefault = new StyledComboBox(gTestFrame, wxID_ANY, choices);
    comboDefault->SetStyleSheet(&sheetDefault);

    const wxSize sizeDefault = comboDefault->GetBestSize();

    EXPECT_GT(sizeWithWideDropDown.x, sizeDefault.x);
}

TEST(StyledComboBox, MainStyleDoesNotLeakToSubControl)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledComboBox { border-width: 5px; border-color: #ff0000; border-style: solid; }\n"
        "StyledComboBox::drop-down { background-color: #ffffff; }"));

    StyledComboBox* combo = new StyledComboBox(gTestFrame, wxID_ANY);
    combo->SetStyleSheet(&sheet);

    const Style dropDownStyle = combo->GetSubControlStyle("drop-down");
    EXPECT_EQ(dropDownStyle.borderWidth, 0);
    EXPECT_EQ(dropDownStyle.borderStyle, wxCustomization::BorderStyle::None);
    EXPECT_EQ(dropDownStyle.backgroundColor, wxColour(0xff, 0xff, 0xff));
}
