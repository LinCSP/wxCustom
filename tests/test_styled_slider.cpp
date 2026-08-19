#include <gtest/gtest.h>

#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledSlider.h"

extern wxFrame* gTestFrame;

using wxCustomization::Property;
using wxCustomization::Style;
using wxCustomization::StyleSheet;
using wxCustomization::StyledSlider;

namespace {

class EventCatcher {
public:
    int eventCount = 0;
    int lastValue = 0;

    void OnSlider(wxCommandEvent& evt)
    {
        ++eventCount;
        lastValue = evt.GetInt();
    }
};

class TestSlider : public StyledSlider {
public:
    TestSlider(wxWindow* parent,
               wxWindowID id = wxID_ANY,
               int value = 0,
               int minValue = 0,
               int maxValue = 100,
               long style = wxSL_HORIZONTAL)
        : StyledSlider(parent, id, value, minValue, maxValue,
                       wxDefaultPosition, wxDefaultSize, style)
    {
    }

    void PaintToPublic(wxDC& dc) { PaintTo(dc); }
};

} // namespace

TEST(StyledSlider, DefaultValueAndRange)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY);
    EXPECT_EQ(slider->GetValue(), 0);
    EXPECT_EQ(slider->GetMin(), 0);
    EXPECT_EQ(slider->GetMax(), 100);
}

TEST(StyledSlider, SetValueClamps)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY, 50, 0, 100);
    EXPECT_EQ(slider->GetValue(), 50);

    slider->SetValue(150);
    EXPECT_EQ(slider->GetValue(), 100);

    slider->SetValue(-10);
    EXPECT_EQ(slider->GetValue(), 0);
}

TEST(StyledSlider, SetRange)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY, 50, 0, 100);
    slider->SetRange(10, 90);

    EXPECT_EQ(slider->GetMin(), 10);
    EXPECT_EQ(slider->GetMax(), 90);
    EXPECT_EQ(slider->GetValue(), 50);

    slider->SetValue(5);
    EXPECT_EQ(slider->GetValue(), 10);
}

TEST(StyledSlider, SetRangeSwapsMinMax)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY, 0, 100, 0);
    EXPECT_EQ(slider->GetMin(), 0);
    EXPECT_EQ(slider->GetMax(), 100);
}

TEST(StyledSlider, SetValueDoesNotEmitEvent)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY, 0, 0, 100);
    EventCatcher catcher;
    slider->Bind(wxEVT_SLIDER, &EventCatcher::OnSlider, &catcher);

    slider->SetValue(50);
    EXPECT_EQ(catcher.eventCount, 0);
}

TEST(StyledSlider, KeyboardArrowChangesValue)
{
    TestSlider* slider = new TestSlider(gTestFrame, wxID_ANY, 50, 0, 100);
    EventCatcher catcher;
    slider->Bind(wxEVT_SLIDER, &EventCatcher::OnSlider, &catcher);

    wxKeyEvent event(wxEVT_KEY_DOWN);
    event.m_keyCode = WXK_RIGHT;
    slider->GetEventHandler()->ProcessEvent(event);

    EXPECT_EQ(slider->GetValue(), 51);
    EXPECT_EQ(catcher.eventCount, 1);
    EXPECT_EQ(catcher.lastValue, 51);
}

TEST(StyledSlider, KeyboardHomeAndEnd)
{
    TestSlider* slider = new TestSlider(gTestFrame, wxID_ANY, 50, 0, 100);

    wxKeyEvent homeEvent(wxEVT_KEY_DOWN);
    homeEvent.m_keyCode = WXK_HOME;
    slider->GetEventHandler()->ProcessEvent(homeEvent);
    EXPECT_EQ(slider->GetValue(), 0);

    wxKeyEvent endEvent(wxEVT_KEY_DOWN);
    endEvent.m_keyCode = WXK_END;
    slider->GetEventHandler()->ProcessEvent(endEvent);
    EXPECT_EQ(slider->GetValue(), 100);
}

TEST(StyledSlider, KeyboardPageUpPageDown)
{
    TestSlider* slider = new TestSlider(gTestFrame, wxID_ANY, 50, 0, 100);

    wxKeyEvent pageUpEvent(wxEVT_KEY_DOWN);
    pageUpEvent.m_keyCode = WXK_PAGEUP;
    slider->GetEventHandler()->ProcessEvent(pageUpEvent);
    EXPECT_EQ(slider->GetValue(), 60);

    wxKeyEvent pageDownEvent(wxEVT_KEY_DOWN);
    pageDownEvent.m_keyCode = WXK_PAGEDOWN;
    slider->GetEventHandler()->ProcessEvent(pageDownEvent);
    EXPECT_EQ(slider->GetValue(), 50);
}

TEST(StyledSlider, BestSizeIsNonZero)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY);
    const wxSize size = slider->GetBestSize();

    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

TEST(StyledSlider, VerticalOrientation)
{
    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY, 0, 0, 100,
                                            wxDefaultPosition, wxDefaultSize,
                                            wxSL_VERTICAL);
    EXPECT_TRUE(slider->IsVertical());

    const wxSize size = slider->GetBestSize();
    EXPECT_LT(size.x, size.y);
}

TEST(StyledSlider, PaintCoversWholeSurface)
{
    // Регрессия «призрака» на wxMSW: у контрола без background в стиле фон
    // обязан полностью перекрывать старые пиксели поверхности (их видно
    // сквозь слайдер под графиком симуляции). Рисуем в DC, заполненный
    // маркерным цветом: после отрисовки маркера остаться не должно.
    TestSlider* slider = new TestSlider(gTestFrame, wxID_ANY, 50, 0, 100);
    slider->SetSize(0, 0, 200, 24);

    const wxColour marker(0xde, 0xad, 0xbe);
    wxBitmap bmp(200, 24, 24);
    {
        wxMemoryDC dc(bmp);
        dc.SetBackground(wxBrush(marker));
        dc.Clear();
        slider->PaintToPublic(dc);
    }

    const wxImage img = bmp.ConvertToImage();
    int markerPixels = 0;
    for (int y = 0; y < img.GetHeight(); ++y) {
        for (int x = 0; x < img.GetWidth(); ++x) {
            if (img.GetRed(x, y) == marker.Red()
                && img.GetGreen(x, y) == marker.Green()
                && img.GetBlue(x, y) == marker.Blue()) {
                ++markerPixels;
            }
        }
    }
    EXPECT_EQ(markerPixels, 0);
}

TEST(StyledSlider, ResolvesSubControlStyles)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledSlider::groove { background-color: #111111; height: 6dip; }\n"
        "StyledSlider::sub-page { background-color: #222222; }\n"
        "StyledSlider::handle { width: 20dip; height: 20dip; background-color: #333333; }\n"
        "StyledSlider::handle:hover { background-color: #444444; }\n"
        "StyledSlider::handle:pressed { background-color: #555555; }"));

    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY);
    slider->SetStyleSheet(&sheet);

    const Style grooveStyle = slider->GetSubControlStyle("groove");
    EXPECT_EQ(grooveStyle.backgroundColor, wxColour(0x11, 0x11, 0x11));
    EXPECT_EQ(grooveStyle.height, 6);

    const Style subPageStyle = slider->GetSubControlStyle("sub-page");
    EXPECT_EQ(subPageStyle.backgroundColor, wxColour(0x22, 0x22, 0x22));

    const Style handleStyle = slider->GetSubControlStyle("handle");
    EXPECT_EQ(handleStyle.backgroundColor, wxColour(0x33, 0x33, 0x33));
    EXPECT_EQ(handleStyle.width, 20);
    EXPECT_EQ(handleStyle.height, 20);

    const Style handleHoverStyle = slider->GetSubControlStyle("handle", "hover");
    EXPECT_EQ(handleHoverStyle.backgroundColor, wxColour(0x44, 0x44, 0x44));

    const Style handlePressedStyle = slider->GetSubControlStyle("handle", "pressed");
    EXPECT_EQ(handlePressedStyle.backgroundColor, wxColour(0x55, 0x55, 0x55));
}

TEST(StyledSlider, MainStyleDoesNotLeakToSubControl)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString(
        "StyledSlider { border-width: 5px; border-color: #ff0000; border-style: solid; }\n"
        "StyledSlider::groove { background-color: #ffffff; }"));

    StyledSlider* slider = new StyledSlider(gTestFrame, wxID_ANY);
    slider->SetStyleSheet(&sheet);

    const Style grooveStyle = slider->GetSubControlStyle("groove");
    EXPECT_EQ(grooveStyle.borderWidth, 0);
    EXPECT_EQ(grooveStyle.borderStyle, wxCustomization::BorderStyle::None);
    EXPECT_EQ(grooveStyle.backgroundColor, wxColour(0xff, 0xff, 0xff));
}
