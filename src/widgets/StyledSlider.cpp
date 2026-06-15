#include "wxCustomization/widgets/StyledSlider.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/dc.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

namespace {

constexpr int DEFAULT_TRACK_THICKNESS_DIP = 4;
constexpr int DEFAULT_HANDLE_SIZE_DIP = 16;
constexpr int DEFAULT_HORIZONTAL_LENGTH_DIP = 120;
constexpr int DEFAULT_VERTICAL_LENGTH_DIP = 120;

} // namespace

wxBEGIN_EVENT_TABLE(StyledSlider, StyledControl)
wxEND_EVENT_TABLE()

StyledSlider::StyledSlider(wxWindow* parent,
                           wxWindowID id,
                           int value,
                           int minValue,
                           int maxValue,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_value(value)
    , m_min(minValue)
    , m_max(maxValue)
{
    if (m_min > m_max) {
        std::swap(m_min, m_max);
    }
    SetAccessibleRole(wxROLE_SYSTEM_SLIDER);
    SetValueInternal(m_value, false);
}

void StyledSlider::SetValue(int value)
{
    SetValueInternal(value, false);
}

void StyledSlider::SetRange(int minValue, int maxValue)
{
    if (minValue > maxValue) {
        std::swap(minValue, maxValue);
    }

    m_min = minValue;
    m_max = maxValue;
    SetValueInternal(m_value, false);
}

void StyledSlider::DrawContent(wxDC& dc, const wxRect& rect)
{
    const wxRect grooveRect = GetGrooveRect(rect);
    const wxRect subPageRect = GetSubPageRect(rect);
    const wxRect handleRect = GetHandleRect(rect);

    // Groove: full track.
    Style grooveStyle = GetSubControlStyle("groove");
    if (!grooveStyle.backgroundColor.IsOk()) {
        grooveStyle.backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
    }

    Painter painter;
    painter.Paint(dc, grooveRect, grooveStyle, this);

    // Sub-page: filled portion from the minimum value to the current value.
    Style subPageStyle = GetSubControlStyle("sub-page");
    if (!subPageStyle.backgroundColor.IsOk()) {
        subPageStyle.backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    }

    painter.Paint(dc, subPageRect, subPageStyle, this);

    // Handle.
    wxString handleState;
    if (m_handlePressed) {
        handleState = "pressed";
    } else if (m_handleHovered) {
        handleState = "hover";
    }

    Style handleStyle = GetSubControlStyle("handle", handleState);
    if (!handleStyle.backgroundColor.IsOk()) {
        handleStyle.backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    }

    painter.Paint(dc, handleRect, handleStyle, this);
}

wxSize StyledSlider::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const bool vertical = IsVertical();
    const wxSize handleSize = GetHandleSize();

    int width = vertical ? handleSize.x : FromDIP(DEFAULT_HORIZONTAL_LENGTH_DIP);
    int height = vertical ? FromDIP(DEFAULT_VERTICAL_LENGTH_DIP) : handleSize.y;

    if (style.IsSet(Property::Width) && style.width > 0) {
        width = style.width;
    }
    if (style.IsSet(Property::Height) && style.height > 0) {
        height = style.height;
    }

    if (style.IsSet(Property::MinWidth) && width < style.minWidth) {
        width = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && height < style.minHeight) {
        height = style.minHeight;
    }

    return wxSize(std::max(width, handleSize.x), std::max(height, handleSize.y));
}

void StyledSlider::OnLeftDown(wxMouseEvent& evt)
{
    if (!IsEnabled()) {
        evt.Skip();
        return;
    }

    SetFocus();

    m_dragging = true;
    m_handlePressed = true;

    if (!HasCapture()) {
        CaptureMouse();
    }

    const wxRect contentRect = GetContentRect();
    SetValueFromPoint(evt.GetPosition(), contentRect, true);
    Refresh();
}

void StyledSlider::OnLeftUp(wxMouseEvent& evt)
{
    if (m_dragging) {
        if (HasCapture()) {
            ReleaseMouse();
        }
        m_dragging = false;
        m_handlePressed = false;
        UpdateHandleState(evt.GetPosition());
        Refresh();
        return;
    }

    StyledControl::OnLeftUp(evt);
}

void StyledSlider::OnMotion(wxMouseEvent& evt)
{
    if (m_dragging) {
        SetValueFromPoint(evt.GetPosition(), GetContentRect(), true);
    } else {
        const bool wasHovered = m_handleHovered;
        UpdateHandleState(evt.GetPosition());
        if (m_handleHovered != wasHovered) {
            Refresh();
        }
    }

    evt.Skip();
}

void StyledSlider::OnMouseLeave(wxMouseEvent& evt)
{
    if (!m_dragging && m_handleHovered) {
        m_handleHovered = false;
        Refresh();
    }

    StyledControl::OnMouseLeave(evt);
}

void StyledSlider::OnKeyDown(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();
    const int range = m_max - m_min;
    const int pageSize = std::max(1, range / 10);

    switch (key) {
        case WXK_UP:
        case WXK_RIGHT:
        case WXK_NUMPAD_UP:
        case WXK_NUMPAD_RIGHT:
            SetValueInternal(m_value + 1, true);
            return;

        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_NUMPAD_DOWN:
        case WXK_NUMPAD_LEFT:
            SetValueInternal(m_value - 1, true);
            return;

        case WXK_PAGEUP:
        case WXK_NUMPAD_PAGEUP:
            SetValueInternal(m_value + pageSize, true);
            return;

        case WXK_PAGEDOWN:
        case WXK_NUMPAD_PAGEDOWN:
            SetValueInternal(m_value - pageSize, true);
            return;

        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            SetValueInternal(m_min, true);
            return;

        case WXK_END:
        case WXK_NUMPAD_END:
            SetValueInternal(m_max, true);
            return;

        default:
            break;
    }

    evt.Skip();
}

void StyledSlider::SetValueInternal(int value, bool sendEvent)
{
    value = std::clamp(value, m_min, m_max);
    if (value == m_value) {
        return;
    }

    m_value = value;
    Refresh();

    if (sendEvent) {
        wxCommandEvent event(wxEVT_SLIDER, GetId());
        event.SetEventObject(this);
        event.SetInt(m_value);
        ProcessWindowEvent(event);
    }
}

void StyledSlider::SetValueFromPoint(const wxPoint& pt, const wxRect& contentRect, bool sendEvent)
{
    const bool vertical = IsVertical();
    const int pos = vertical ? pt.y : pt.x;
    SetValueInternal(ValueFromPosition(pos, contentRect), sendEvent);
}

void StyledSlider::UpdateHandleState(const wxPoint& pt)
{
    if (!IsEnabled()) {
        m_handleHovered = false;
        return;
    }

    m_handleHovered = GetHandleRect(GetContentRect()).Contains(pt);
}

wxRect StyledSlider::GetGrooveRect(const wxRect& contentRect) const
{
    const Style grooveStyle = GetSubControlStyle("groove");
    const bool vertical = IsVertical();

    if (vertical) {
        int width = FromDIP(DEFAULT_TRACK_THICKNESS_DIP);
        if (grooveStyle.IsSet(Property::Width) && grooveStyle.width > 0) {
            width = grooveStyle.width;
        }
        const int x = contentRect.x + (contentRect.width - width) / 2;
        return wxRect(x, contentRect.y, width, contentRect.height);
    }

    int height = FromDIP(DEFAULT_TRACK_THICKNESS_DIP);
    if (grooveStyle.IsSet(Property::Height) && grooveStyle.height > 0) {
        height = grooveStyle.height;
    }
    const int y = contentRect.y + (contentRect.height - height) / 2;
    return wxRect(contentRect.x, y, contentRect.width, height);
}

wxRect StyledSlider::GetSubPageRect(const wxRect& contentRect) const
{
    const wxRect grooveRect = GetGrooveRect(contentRect);
    const bool vertical = IsVertical();
    const int handleCenter = PositionFromValue(m_value, contentRect);

    if (vertical) {
        const int y = handleCenter;
        const int height = grooveRect.y + grooveRect.height - y;
        return wxRect(grooveRect.x, y, grooveRect.width, std::max(height, 0));
    }

    const int width = handleCenter - grooveRect.x;
    return wxRect(grooveRect.x, grooveRect.y, std::max(width, 0), grooveRect.height);
}

wxRect StyledSlider::GetHandleRect(const wxRect& contentRect) const
{
    const wxSize handleSize = GetHandleSize();
    const bool vertical = IsVertical();
    const int center = PositionFromValue(m_value, contentRect);

    if (vertical) {
        return wxRect(contentRect.x + (contentRect.width - handleSize.x) / 2,
                      center - handleSize.y / 2,
                      handleSize.x,
                      handleSize.y);
    }

    return wxRect(center - handleSize.x / 2,
                  contentRect.y + (contentRect.height - handleSize.y) / 2,
                  handleSize.x,
                  handleSize.y);
}

int StyledSlider::PositionFromValue(int value, const wxRect& contentRect) const
{
    const bool vertical = IsVertical();
    const wxSize handleSize = GetHandleSize();
    value = std::clamp(value, m_min, m_max);

    if (m_max == m_min) {
        return vertical ? contentRect.y + contentRect.height / 2
                        : contentRect.x + contentRect.width / 2;
    }

    const double ratio = static_cast<double>(value - m_min) / (m_max - m_min);

    if (vertical) {
        const int trackLength = std::max(contentRect.height - handleSize.y, 1);
        return contentRect.y + handleSize.y / 2
               + static_cast<int>(trackLength * (1.0 - ratio));
    }

    const int trackLength = std::max(contentRect.width - handleSize.x, 1);
    return contentRect.x + handleSize.x / 2 + static_cast<int>(trackLength * ratio);
}

int StyledSlider::ValueFromPosition(int pos, const wxRect& contentRect) const
{
    const bool vertical = IsVertical();
    const wxSize handleSize = GetHandleSize();

    if (m_max == m_min) {
        return m_min;
    }

    if (vertical) {
        const int trackLength = std::max(contentRect.height - handleSize.y, 1);
        const int start = contentRect.y + handleSize.y / 2;
        double ratio = 1.0 - static_cast<double>(pos - start) / trackLength;
        ratio = std::clamp(ratio, 0.0, 1.0);
        return m_min + static_cast<int>(ratio * (m_max - m_min) + 0.5);
    }

    const int trackLength = std::max(contentRect.width - handleSize.x, 1);
    const int start = contentRect.x + handleSize.x / 2;
    double ratio = static_cast<double>(pos - start) / trackLength;
    ratio = std::clamp(ratio, 0.0, 1.0);
    return m_min + static_cast<int>(ratio * (m_max - m_min) + 0.5);
}

wxSize StyledSlider::GetHandleSize() const
{
    const Style handleStyle = GetSubControlStyle("handle");

    int width = FromDIP(DEFAULT_HANDLE_SIZE_DIP);
    int height = FromDIP(DEFAULT_HANDLE_SIZE_DIP);

    if (handleStyle.IsSet(Property::Width) && handleStyle.width > 0) {
        width = handleStyle.width;
    }
    if (handleStyle.IsSet(Property::Height) && handleStyle.height > 0) {
        height = handleStyle.height;
    }

    return wxSize(width, height);
}

} // namespace wxCustomization
