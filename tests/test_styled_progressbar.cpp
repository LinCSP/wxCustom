#include <gtest/gtest.h>

#include <wx/frame.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledProgressBar.h"

extern wxFrame* gTestFrame;

using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledProgressBar;

TEST(StyledProgressBar, DefaultValueAndRange)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY);

    EXPECT_EQ(bar->GetValue(), 0);
    EXPECT_EQ(bar->GetMin(), 0);
    EXPECT_EQ(bar->GetMax(), 100);
    EXPECT_FALSE(bar->IsVertical());
}

TEST(StyledProgressBar, SetValueClamps)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY, 50, 0, 100);
    EXPECT_EQ(bar->GetValue(), 50);

    bar->SetValue(150);
    EXPECT_EQ(bar->GetValue(), 100);

    bar->SetValue(-10);
    EXPECT_EQ(bar->GetValue(), 0);
}

TEST(StyledProgressBar, SetRange)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY, 50, 0, 100);
    bar->SetRange(10, 90);

    EXPECT_EQ(bar->GetMin(), 10);
    EXPECT_EQ(bar->GetMax(), 90);
    EXPECT_EQ(bar->GetValue(), 50);

    bar->SetValue(5);
    EXPECT_EQ(bar->GetValue(), 10);
}

TEST(StyledProgressBar, SetRangeSwapsMinMax)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY, 0, 100, 0);
    EXPECT_EQ(bar->GetMin(), 0);
    EXPECT_EQ(bar->GetMax(), 100);
}

TEST(StyledProgressBar, BestSizeIsNonZero)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY);
    const wxSize size = bar->GetBestSize();

    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

TEST(StyledProgressBar, VerticalOrientation)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY, 0, 0, 100,
                                                   wxDefaultPosition, wxDefaultSize,
                                                   wxGA_VERTICAL);
    EXPECT_TRUE(bar->IsVertical());

    const wxSize size = bar->GetBestSize();
    EXPECT_GT(size.y, size.x);
}

TEST(StyledProgressBar, ResolvesSubControlStyles)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledProgressBar::groove { background-color: #111111; height: 8dip; }\n"
        "StyledProgressBar::chunk { background-color: #222222; }"));

    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY);
    bar->SetStyleSheet(&sheet);

    const Style grooveStyle = bar->GetSubControlStyle("groove");
    EXPECT_EQ(grooveStyle.backgroundColor, wxColour(0x11, 0x11, 0x11));
    EXPECT_EQ(grooveStyle.height, 8);

    const Style chunkStyle = bar->GetSubControlStyle("chunk");
    EXPECT_EQ(chunkStyle.backgroundColor, wxColour(0x22, 0x22, 0x22));
}

TEST(StyledProgressBar, MainStyleDoesNotLeakToSubControl)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledProgressBar { border-width: 5px; border-color: #ff0000; border-style: solid; }\n"
        "StyledProgressBar::groove { background-color: #ffffff; }"));

    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY);
    bar->SetStyleSheet(&sheet);

    const Style grooveStyle = bar->GetSubControlStyle("groove");
    EXPECT_EQ(grooveStyle.borderWidth, 0);
    EXPECT_EQ(grooveStyle.borderStyle, wxCustomization::BorderStyle::None);
    EXPECT_EQ(grooveStyle.backgroundColor, wxColour(0xff, 0xff, 0xff));
}

TEST(StyledProgressBar, IndeterminateFlag)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY);
    EXPECT_FALSE(bar->IsIndeterminate());

    bar->SetIndeterminate(true);
    EXPECT_TRUE(bar->IsIndeterminate());

    bar->SetIndeterminate(false);
    EXPECT_FALSE(bar->IsIndeterminate());
}

TEST(StyledProgressBar, ShowTextFlag)
{
    StyledProgressBar* bar = new StyledProgressBar(gTestFrame, wxID_ANY);
    EXPECT_TRUE(bar->ShowsText());

    bar->ShowText(false);
    EXPECT_FALSE(bar->ShowsText());

    bar->ShowText(true);
    EXPECT_TRUE(bar->ShowsText());
}

class TestProgressBar : public StyledProgressBar {
public:
    TestProgressBar(wxWindow* parent)
        : StyledProgressBar(parent, wxID_ANY)
    {
    }

    bool TestAcceptsFocus() const { return AcceptsFocus(); }
    bool TestAcceptsFocusFromKeyboard() const { return AcceptsFocusFromKeyboard(); }
};

TEST(StyledProgressBar, DoesNotAcceptFocus)
{
    TestProgressBar* bar = new TestProgressBar(gTestFrame);
    EXPECT_FALSE(bar->TestAcceptsFocus());
    EXPECT_FALSE(bar->TestAcceptsFocusFromKeyboard());
}
