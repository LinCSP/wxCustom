#include <gtest/gtest.h>

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>
#include <wx/sizer.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/widgets/StyledGroupBox.h"
#include "wxCustomization/widgets/StyledPanel.h"

extern wxFrame* gTestFrame;

using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledButton;
using wxCustomization::StyledGroupBox;
using wxCustomization::StyledPanel;

namespace {

wxColour GetPixel(const wxBitmap& bmp, int x, int y)
{
    wxImage image = bmp.ConvertToImage();
    return wxColour(image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y));
}

class RenderableGroupBox : public StyledGroupBox {
public:
    RenderableGroupBox(wxWindow* parent, const wxString& title)
        : StyledGroupBox(parent, wxID_ANY, title)
    {
    }

    void Render(wxDC& dc, const wxRect& rect) { DrawGroupBox(dc, rect); }
};

} // namespace

TEST(StyledGroupBox, StoresTitle)
{
    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");

    EXPECT_EQ(box->GetTitle(), "Options");
    EXPECT_EQ(box->GetStyledControlType(), "StyledGroupBox");
}

TEST(StyledGroupBox, SetTitleUpdates)
{
    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Old");
    box->SetTitle("New");

    EXPECT_EQ(box->GetTitle(), "New");
    // Accessible name follows the title while it was not customized.
    EXPECT_EQ(box->GetAccessibleLabel(), "New");
}

TEST(StyledGroupBox, SetTitleKeepsCustomAccessibleLabel)
{
    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Old");
    box->SetAccessibleLabel("Custom name");
    box->SetTitle("New");

    EXPECT_EQ(box->GetAccessibleLabel(), "Custom name");
}

TEST(StyledGroupBox, AccessibleRoleIsGrouping)
{
    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    EXPECT_EQ(box->GetAccessibleRole(), wxROLE_SYSTEM_GROUPING);
}

class TestGroupBox : public StyledGroupBox {
public:
    TestGroupBox(wxWindow* parent, const wxString& title)
        : StyledGroupBox(parent, wxID_ANY, title)
    {
    }

    bool TestAcceptsFocus() const { return AcceptsFocus(); }
    bool TestAcceptsFocusFromKeyboard() const { return AcceptsFocusFromKeyboard(); }
};

TEST(StyledGroupBox, DoesNotAcceptFocus)
{
    TestGroupBox* box = new TestGroupBox(gTestFrame, "Options");
    EXPECT_FALSE(box->TestAcceptsFocus());
    EXPECT_FALSE(box->TestAcceptsFocusFromKeyboard());
}

TEST(StyledGroupBox, TitleHeightDependsOnTitle)
{
    StyledGroupBox* withTitle = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    StyledGroupBox* withoutTitle = new StyledGroupBox(gTestFrame, wxID_ANY);

    EXPECT_GT(withTitle->GetTitleHeight(), 0);
    EXPECT_EQ(withoutTitle->GetTitleHeight(), 0);
}

TEST(StyledGroupBox, ResolvesTitleSubControlStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledGroupBox { border-width: 1px; border-color: #bdc3c7; border-style: solid; }\n"
        "StyledGroupBox::title { color: #123456; font-size: 14px; padding: 0px 6px; }"));

    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    box->SetStyleSheet(&sheet);

    const Style titleStyle = box->GetSubControlStyle("title");
    EXPECT_EQ(titleStyle.color, wxColour(0x12, 0x34, 0x56));
    EXPECT_EQ(titleStyle.paddingLeft, 6);
    EXPECT_EQ(titleStyle.paddingRight, 6);
    EXPECT_TRUE(titleStyle.font.IsOk());
}

TEST(StyledGroupBox, MainStyleDoesNotLeakToSubControl)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledGroupBox { border-width: 5px; border-color: #ff0000; border-style: solid; }\n"
        "StyledGroupBox::title { color: #00ff00; }"));

    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    box->SetStyleSheet(&sheet);

    const Style titleStyle = box->GetSubControlStyle("title");
    EXPECT_EQ(titleStyle.borderWidth, 0);
    EXPECT_EQ(titleStyle.borderStyle, wxCustomization::BorderStyle::None);
    EXPECT_EQ(titleStyle.color, wxColour(0x00, 0xff, 0x00));
}

TEST(StyledGroupBox, ActsAsContainer)
{
    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    StyledButton* button = new StyledButton(box, wxID_ANY, "OK");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(button, 0, wxALL, 0);
    box->SetSizer(sizer);

    ASSERT_EQ(box->GetChildren().GetCount(), 1u);
    EXPECT_EQ(button->GetParent(), box);

    const wxSize size = box->GetBestSize();
    const wxSize buttonSize = button->GetBestSize();
    EXPECT_GE(size.x, buttonSize.x);
    EXPECT_GE(size.y, buttonSize.y);
}

TEST(StyledGroupBox, BestSizeWithoutChildrenIncludesTitle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledGroupBox { border-width: 2px; padding: 8px; }"));

    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    box->SetStyleSheet(&sheet);

    const wxSize size = box->GetBestSize();
    EXPECT_GT(size.x, 0);
    EXPECT_GE(size.y, box->GetTitleHeight());
}

TEST(StyledGroupBox, BestSizeHonoursMinSize)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledGroupBox { min-width: 200px; min-height: 100px; }"));

    StyledGroupBox* box = new StyledGroupBox(gTestFrame, wxID_ANY, "Options");
    box->SetStyleSheet(&sheet);

    const wxSize size = box->GetBestSize();
    EXPECT_GE(size.x, 200);
    EXPECT_GE(size.y, 100);
}

TEST(StyledGroupBox, FrameIsCutOutUnderTitle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledPanel { background-color: #ffffff; }\n"
        "StyledGroupBox { border-width: 2px; border-color: #ff0000; border-style: solid; }\n"
        "StyledGroupBox::title { color: #000000; font-size: 12px; padding: 0px 4px; }"));

    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    panel->SetStyleSheet(&sheet);

    RenderableGroupBox* box = new RenderableGroupBox(panel, "Group Title");
    box->SetStyleSheet(&sheet);
    box->SetSize(0, 0, 300, 100);

    wxBitmap bitmap(300, 100, 24);
    {
        wxMemoryDC dc(bitmap);
        dc.SetBackground(wxBrush(wxColour(255, 255, 255)));
        dc.Clear();
        box->Render(dc, wxRect(0, 0, 300, 100));
    }

    const int frameTop = box->GetTitleHeight() / 2;
    const int edgeY = frameTop + 1; // centre row of the 2px top border

    // The frame's top edge away from the title uses the border colour.
    const wxColour edgeColour = GetPixel(bitmap, 250, edgeY);
    EXPECT_GT(edgeColour.Red(), 200);
    EXPECT_LT(edgeColour.Green(), 60);

    // Underneath the title the border is cut out: no border-coloured pixels
    // on the same row within the title strip (glyph pixels are black).
    int borderPixels = 0;
    for (int x = 6; x < 46; ++x) {
        const wxColour c = GetPixel(bitmap, x, edgeY);
        if (c.Red() > 200 && c.Green() < 60 && c.Blue() < 60) {
            ++borderPixels;
        }
    }
    EXPECT_EQ(borderPixels, 0);
}
