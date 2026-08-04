#include "wxCustomization/widgets/StyledProgressBar.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/utils.h>

#include <algorithm>

namespace wxCustomization {

namespace {

constexpr int DEFAULT_HORIZONTAL_LENGTH_DIP = 120;
constexpr int DEFAULT_VERTICAL_LENGTH_DIP = 120;
constexpr int DEFAULT_THICKNESS_DIP = 16;
constexpr int INDETERMINATE_TIMER_INTERVAL_MS = 25;
// Chunk speed in pixels per second; the offset is computed from the elapsed
// time so the motion stays constant-speed even if a timer tick is late.
constexpr int INDETERMINATE_SPEED_PX_PER_SEC = 40;

} // namespace

wxBEGIN_EVENT_TABLE(StyledProgressBar, StyledControl)
    EVT_TIMER(wxID_ANY, StyledProgressBar::OnTimer)
wxEND_EVENT_TABLE()

StyledProgressBar::StyledProgressBar(wxWindow* parent,
                                     wxWindowID id,
                                     int value,
                                     int minValue,
                                     int maxValue,
                                     const wxPoint& pos,
                                     const wxSize& size,
                                     long style,
                                     const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_timer(this)
    , m_value(value)
    , m_min(minValue)
    , m_max(maxValue)
{
    if (m_min > m_max) {
        std::swap(m_min, m_max);
    }

    SetAccessibleRole(wxROLE_SYSTEM_PROGRESSBAR);
    SetValueInternal(m_value);
}

void StyledProgressBar::SetValue(int value)
{
    SetValueInternal(value);
}

void StyledProgressBar::SetRange(int minValue, int maxValue)
{
    if (minValue > maxValue) {
        std::swap(minValue, maxValue);
    }

    m_min = minValue;
    m_max = maxValue;
    SetValueInternal(m_value);
}

void StyledProgressBar::SetIndeterminate(bool indeterminate)
{
    if (m_indeterminate == indeterminate) {
        return;
    }

    m_indeterminate = indeterminate;
    if (m_indeterminate) {
        m_indeterminateOffset = 0;
        m_indeterminateStartMs = wxGetUTCTimeMillis().GetValue();
        m_timer.Start(INDETERMINATE_TIMER_INTERVAL_MS);
    } else {
        m_timer.Stop();
    }

    Refresh();
}

void StyledProgressBar::ShowText(bool show)
{
    if (m_showText == show) {
        return;
    }

    m_showText = show;
    Refresh();
}

void StyledProgressBar::DrawContent(wxDC& dc, const wxRect& rect)
{
    const wxRect grooveRect = GetGrooveRect(rect);

    // Groove: full track.
    Style grooveStyle = GetSubControlStyle("groove");
    if (!grooveStyle.backgroundColor.IsOk()) {
        grooveStyle.backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
    }

    Painter painter;
    painter.Paint(dc, grooveRect, grooveStyle, this);

    // Chunk: filled portion.
    const wxRect chunkRect = GetChunkRect(rect);
    if (chunkRect.width > 0 && chunkRect.height > 0) {
        Style chunkStyle = GetSubControlStyle("chunk");
        if (!chunkStyle.backgroundColor.IsOk()) {
            chunkStyle.backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
        }

        painter.Paint(dc, chunkRect, chunkStyle, this);
    }

    // Percentage text.
    if (m_showText && !m_indeterminate) {
        const Style& style = GetCurrentStyle();

        dc.SetFont(style.font.IsOk() ? style.font
                                     : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        dc.SetTextForeground(style.color.IsOk() ? style.color : *wxBLACK);

        const wxString text = GetPercentageText();
        int textWidth = 0;
        int textHeight = 0;
        dc.GetTextExtent(text, &textWidth, &textHeight);

        const int x = rect.x + (rect.width - textWidth) / 2;
        const int y = rect.y + (rect.height - textHeight) / 2;

        dc.DrawText(text, x, y);
    }
}

wxSize StyledProgressBar::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const bool vertical = IsVertical();

    int width = vertical ? FromDIP(DEFAULT_THICKNESS_DIP)
                         : FromDIP(DEFAULT_HORIZONTAL_LENGTH_DIP);
    int height = vertical ? FromDIP(DEFAULT_VERTICAL_LENGTH_DIP)
                          : FromDIP(DEFAULT_THICKNESS_DIP);

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

    return wxSize(width, height);
}

void StyledProgressBar::OnTimer(wxTimerEvent& /*evt*/)
{
    if (!m_indeterminate) {
        return;
    }

    // Time-based offset: a delayed tick skips ahead instead of slowing the
    // animation down, so the motion does not accumulate jitter under load.
    const long long elapsed = wxGetUTCTimeMillis().GetValue() - m_indeterminateStartMs;
    m_indeterminateOffset =
        static_cast<int>(elapsed * INDETERMINATE_SPEED_PX_PER_SEC / 1000);
    Refresh();
}

void StyledProgressBar::SetValueInternal(int value)
{
    value = std::clamp(value, m_min, m_max);
    if (value == m_value && !m_indeterminate) {
        return;
    }

    m_value = value;
    Refresh();
}

wxRect StyledProgressBar::GetGrooveRect(const wxRect& contentRect) const
{
    const Style grooveStyle = GetSubControlStyle("groove");
    const bool vertical = IsVertical();

    if (vertical) {
        int width = contentRect.width;
        if (grooveStyle.IsSet(Property::Width) && grooveStyle.width > 0) {
            width = grooveStyle.width;
        }
        const int x = contentRect.x + (contentRect.width - width) / 2;
        return wxRect(x, contentRect.y, width, contentRect.height);
    }

    int height = contentRect.height;
    if (grooveStyle.IsSet(Property::Height) && grooveStyle.height > 0) {
        height = grooveStyle.height;
    }
    const int y = contentRect.y + (contentRect.height - height) / 2;
    return wxRect(contentRect.x, y, contentRect.width, height);
}

wxRect StyledProgressBar::GetChunkRect(const wxRect& contentRect) const
{
    const wxRect grooveRect = GetGrooveRect(contentRect);
    const bool vertical = IsVertical();

    if (m_indeterminate) {
        const int trackLength = vertical ? grooveRect.height : grooveRect.width;
        const int chunkLength = std::max(trackLength / 4, 2);
        const int period = std::max(trackLength * 2, 1);

        int offset = m_indeterminateOffset % period;
        if (offset > trackLength) {
            offset = period - offset;
        }

        if (vertical) {
            return wxRect(grooveRect.x,
                          grooveRect.y + grooveRect.height - chunkLength - offset,
                          grooveRect.width,
                          chunkLength);
        }

        return wxRect(grooveRect.x + offset,
                      grooveRect.y,
                      chunkLength,
                      grooveRect.height);
    }

    const int range = m_max - m_min;
    double ratio = 0.0;
    if (range > 0) {
        ratio = static_cast<double>(m_value - m_min) / range;
    }

    if (vertical) {
        const int height = static_cast<int>(grooveRect.height * ratio);
        return wxRect(grooveRect.x,
                      grooveRect.y + grooveRect.height - height,
                      grooveRect.width,
                      height);
    }

    const int width = static_cast<int>(grooveRect.width * ratio);
    return wxRect(grooveRect.x, grooveRect.y, width, grooveRect.height);
}

wxString StyledProgressBar::GetPercentageText() const
{
    const int range = m_max - m_min;
    int percent = 0;
    if (range > 0) {
        percent = static_cast<int>(100.0 * (m_value - m_min) / range);
    }
    return wxString::Format("%d%%", percent);
}

} // namespace wxCustomization
