#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/gauge.h>
#include <wx/timer.h>

namespace wxCustomization {

/// A styled progress bar widget with customizable groove and chunk.
///
/// Supports horizontal (wxGA_HORIZONTAL) and vertical (wxGA_VERTICAL)
/// orientations, an indeterminate (animated) mode and an optional percentage
/// text label.
class StyledProgressBar : public StyledControl {
public:
    StyledProgressBar(wxWindow* parent,
                      wxWindowID id = wxID_ANY,
                      int value = 0,
                      int minValue = 0,
                      int maxValue = 100,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxGA_HORIZONTAL,
                      const wxString& name = wxControlNameStr);

    /// Set the current value. The value is clamped to the current range.
    void SetValue(int value);

    /// Return the current value.
    int GetValue() const { return m_value; }

    /// Set the allowed value range.
    void SetRange(int minValue, int maxValue);

    /// Return the minimum allowed value.
    int GetMin() const { return m_min; }

    /// Return the maximum allowed value.
    int GetMax() const { return m_max; }

    /// Return true if the progress bar is vertical.
    bool IsVertical() const { return (GetWindowStyle() & wxGA_VERTICAL) != 0; }

    /// Enable or disable the indeterminate (animated) mode.
    void SetIndeterminate(bool indeterminate);

    /// Return true if the progress bar is in indeterminate mode.
    bool IsIndeterminate() const { return m_indeterminate; }

    /// Show or hide the percentage text label.
    void ShowText(bool show);

    /// Return true if the percentage text is shown.
    bool ShowsText() const { return m_showText; }

    wxString GetStyledControlType() const override { return "StyledProgressBar"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    wxSize DoGetBestSize() const override;

    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

    void OnTimer(wxTimerEvent& evt);

private:
    void SetValueInternal(int value);

    wxRect GetGrooveRect(const wxRect& contentRect) const;
    wxRect GetChunkRect(const wxRect& contentRect) const;

    wxString GetPercentageText() const;

    int m_value = 0;
    int m_min = 0;
    int m_max = 100;

    bool m_indeterminate = false;
    bool m_showText = true;
    int m_indeterminateOffset = 0;

    wxTimer m_timer;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
