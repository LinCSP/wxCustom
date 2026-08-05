#include <gtest/gtest.h>

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledFrame.h"
#include "wxCustomization/widgets/StyledPanel.h"
#include "wxCustomization/widgets/StyledTitleBar.h"

extern wxFrame* gTestFrame;

using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledFrame;
using wxCustomization::StyledTitleBar;

namespace {

wxColour GetPixel(const wxBitmap& bmp, int x, int y)
{
    wxImage image = bmp.ConvertToImage();
    return wxColour(image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y));
}

class TestTitleBar : public StyledTitleBar {
public:
    explicit TestTitleBar(wxWindow* parent)
        : StyledTitleBar(parent, wxID_ANY)
    {
    }

    using StyledTitleBar::BtnClose;
    using StyledTitleBar::BtnMaximize;
    using StyledTitleBar::BtnMinimize;
    using StyledTitleBar::DrawTitleBar;
    using StyledTitleBar::GetCaptionButtonRect;
    using StyledTitleBar::GetCaptionButtonStyle;
    using StyledTitleBar::HitTestCaptionButton;

    bool TestAcceptsFocus() const { return AcceptsFocus(); }
    bool TestAcceptsFocusFromKeyboard() const { return AcceptsFocusFromKeyboard(); }
    void SetHoveredButton(int index) { m_hoveredButton = index; }

    void ActivateCaptionButton(int index) override { activated.push_back(index); }
    std::vector<int> activated;
};

void ClickAt(TestTitleBar* bar, const wxPoint& pt)
{
    wxMouseEvent enterEvent(wxEVT_ENTER_WINDOW);
    bar->GetEventHandler()->ProcessEvent(enterEvent);
    wxMouseEvent downEvent(wxEVT_LEFT_DOWN);
    downEvent.SetPosition(pt);
    bar->GetEventHandler()->ProcessEvent(downEvent);
    wxMouseEvent upEvent(wxEVT_LEFT_UP);
    upEvent.SetPosition(pt);
    bar->GetEventHandler()->ProcessEvent(upEvent);
}

} // namespace

TEST(StyledTitleBar, StoresTitle)
{
    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetTitle("My App");

    EXPECT_EQ(bar->GetTitle(), "My App");
    EXPECT_EQ(bar->GetStyledControlType(), "StyledTitleBar");
}

TEST(StyledTitleBar, DoesNotAcceptFocus)
{
    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    EXPECT_FALSE(bar->TestAcceptsFocus());
    EXPECT_FALSE(bar->TestAcceptsFocusFromKeyboard());
}

TEST(StyledTitleBar, AccessibleRoleIsTitleBar)
{
    StyledTitleBar* bar = new StyledTitleBar(gTestFrame, wxID_ANY);
    EXPECT_EQ(bar->GetAccessibleRole(), wxROLE_SYSTEM_TITLEBAR);
}

TEST(StyledTitleBar, ResolvesSubControlStyles)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTitleBar::title { color: #101010; }\n"
        "StyledTitleBar::caption-button { color: #202020; width: 40px; }\n"
        "StyledTitleBar::close-button:hover { background-color: #e81123; color: #ffffff; }"));

    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetStyleSheet(&sheet);

    EXPECT_EQ(bar->GetSubControlStyle("title").color, wxColour(0x10, 0x10, 0x10));

    const Style closeHover = bar->GetSubControlStyle("close-button", "hover");
    EXPECT_EQ(closeHover.backgroundColor, wxColour(0xe8, 0x11, 0x23));
}

TEST(StyledTitleBar, CaptionButtonsAreRightAlignedWithCloseRightmost)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTitleBar::caption-button { width: 40px; }"));

    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetStyleSheet(&sheet);
    bar->SetSize(0, 0, 400, 32);

    const wxRect minRect = bar->GetCaptionButtonRect(TestTitleBar::BtnMinimize);
    const wxRect maxRect = bar->GetCaptionButtonRect(TestTitleBar::BtnMaximize);
    const wxRect closeRect = bar->GetCaptionButtonRect(TestTitleBar::BtnClose);

    EXPECT_EQ(closeRect.x + closeRect.width, 400);
    EXPECT_EQ(maxRect.x + maxRect.width, closeRect.x);
    EXPECT_EQ(minRect.x + minRect.width, maxRect.x);
    EXPECT_EQ(closeRect.width, 40);
}

TEST(StyledTitleBar, HitTestCaptionButton)
{
    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetSize(0, 0, 400, 32);

    const wxRect closeRect = bar->GetCaptionButtonRect(TestTitleBar::BtnClose);
    EXPECT_EQ(bar->HitTestCaptionButton(wxPoint(closeRect.x + 2, closeRect.y + 2)),
              TestTitleBar::BtnClose);
    EXPECT_EQ(bar->HitTestCaptionButton(wxPoint(10, 5)), -1);
}

TEST(StyledTitleBar, ClickCaptionButtonActivatesIt)
{
    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetSize(0, 0, 400, 32);

    const wxRect closeRect = bar->GetCaptionButtonRect(TestTitleBar::BtnClose);
    ClickAt(bar, wxPoint(closeRect.x + 5, closeRect.y + 5));

    ASSERT_EQ(bar->activated.size(), 1u);
    EXPECT_EQ(bar->activated[0], TestTitleBar::BtnClose);
}

TEST(StyledTitleBar, ClickOutsideButtonsActivatesNothing)
{
    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetSize(0, 0, 400, 32);

    ClickAt(bar, wxPoint(50, 5));
    EXPECT_TRUE(bar->activated.empty());
}

TEST(StyledTitleBar, OnlyHoveredButtonUsesHoverStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTitleBar::caption-button { background-color: #000000; }\n"
        "StyledTitleBar::caption-button:hover { background-color: #111111; }\n"
        "StyledTitleBar::close-button:hover { background-color: #e81123; }"));

    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetStyleSheet(&sheet);
    bar->SetSize(0, 0, 400, 32);

    // The bar itself is hovered (mouse over the minimize button): the
    // widget-level hover must not turn every button's `:hover` style on.
    wxMouseEvent enterEvent(wxEVT_ENTER_WINDOW);
    bar->GetEventHandler()->ProcessEvent(enterEvent);
    bar->SetHoveredButton(TestTitleBar::BtnMinimize);

    EXPECT_EQ(bar->GetCaptionButtonStyle(TestTitleBar::BtnMinimize).backgroundColor,
              wxColour(0x11, 0x11, 0x11));
    EXPECT_EQ(bar->GetCaptionButtonStyle(TestTitleBar::BtnMaximize).backgroundColor,
              wxColour(0, 0, 0));
    // Most importantly: close is NOT red while minimize is hovered.
    EXPECT_EQ(bar->GetCaptionButtonStyle(TestTitleBar::BtnClose).backgroundColor,
              wxColour(0, 0, 0));

    bar->SetHoveredButton(TestTitleBar::BtnClose);
    EXPECT_EQ(bar->GetCaptionButtonStyle(TestTitleBar::BtnClose).backgroundColor,
              wxColour(0xe8, 0x11, 0x23));
}

TEST(StyledTitleBar, CloseButtonHoverUsesHoverStyle)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTitleBar::caption-button { background-color: #000000; }\n"
        "StyledTitleBar::close-button:hover { background-color: #e81123; }"));

    TestTitleBar* bar = new TestTitleBar(gTestFrame);
    bar->SetStyleSheet(&sheet);
    bar->SetSize(0, 0, 400, 32);
    bar->SetHoveredButton(TestTitleBar::BtnClose);

    const Style style = bar->GetCaptionButtonStyle(TestTitleBar::BtnClose);
    EXPECT_EQ(style.backgroundColor, wxColour(0xe8, 0x11, 0x23));
}

TEST(StyledFrame, HasTitleBarAndClientPanel)
{
    StyledFrame* frame = new StyledFrame(gTestFrame, wxID_ANY, "Frame Title");

    ASSERT_NE(frame->GetTitleBar(), nullptr);
    ASSERT_NE(frame->GetClientPanel(), nullptr);
    EXPECT_EQ(frame->GetTitleBar()->GetTitle(), "Frame Title");

    frame->Destroy();
}

TEST(StyledFrame, HasNoWindowManagerDecorations)
{
    StyledFrame* frame = new StyledFrame(gTestFrame, wxID_ANY, "Decorations");

    const long style = frame->GetWindowStyleFlag();
    EXPECT_EQ(style & wxCAPTION, 0);
    EXPECT_EQ(style & wxRESIZE_BORDER, 0);
    EXPECT_EQ(style & wxSYSTEM_MENU, 0);

    frame->Destroy();
}

TEST(StyledFrame, SetTitleSyncsTitleBar)
{
    StyledFrame* frame = new StyledFrame(gTestFrame, wxID_ANY, "Old");
    frame->SetTitle("New Title");

    EXPECT_EQ(frame->GetTitle(), "New Title");
    EXPECT_EQ(frame->GetTitleBar()->GetTitle(), "New Title");

    frame->Destroy();
}

TEST(StyledFrame, SetStyleSheetReachesTitleBarAndClientPanel)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledTitleBar { background-color: #123456; }\n"
        "StyledPanel { background-color: #654321; }"));

    StyledFrame* frame = new StyledFrame(gTestFrame, wxID_ANY, "Styled");
    frame->SetStyleSheet(&sheet);

    EXPECT_EQ(frame->GetTitleBar()->GetStyleSheet(), &sheet);
    EXPECT_EQ(frame->GetClientPanel()->GetStyleSheet(), &sheet);

    frame->Destroy();
}

TEST(StyledFrame, HitTestResizeEdges)
{
    const wxSize size(200, 100);
    const int margin = 6;

    // Edges.
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(3, 50), size, margin),
              StyledFrame::ResizeW);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(197, 50), size, margin),
              StyledFrame::ResizeE);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(100, 2), size, margin),
              StyledFrame::ResizeN);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(100, 97), size, margin),
              StyledFrame::ResizeS);

    // Corners.
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(2, 2), size, margin),
              StyledFrame::ResizeNW);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(198, 2), size, margin),
              StyledFrame::ResizeNE);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(2, 98), size, margin),
              StyledFrame::ResizeSW);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(198, 98), size, margin),
              StyledFrame::ResizeSE);

    // Outside the zones.
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(100, 50), size, margin),
              StyledFrame::ResizeNone);
    EXPECT_EQ(StyledFrame::HitTestResizeEdges(wxPoint(10, 10), size, margin),
              StyledFrame::ResizeNone);
}
