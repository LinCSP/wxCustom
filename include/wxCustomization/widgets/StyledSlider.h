#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/slider.h>

namespace wxCustomization {

/// A styled slider widget with customizable groove, sub-page and handle.
///
/// Supports horizontal (wxSL_HORIZONTAL) and vertical (wxSL_VERTICAL)
/// orientations, mouse dragging, keyboard control and emits wxEVT_SLIDER
/// when the value changes through user interaction.
class StyledSlider : public StyledControl {
public:
    StyledSlider(wxWindow* parent,
                 wxWindowID id = wxID_ANY,
                 int value = 0,
                 int minValue = 0,
                 int maxValue = 100,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxSL_HORIZONTAL,
                 const wxString& name = wxControlNameStr);

    /// Set the current value. Does not emit wxEVT_SLIDER.
    void SetValue(int value);

    /// Return the current value.
    int GetValue() const { return m_value; }

    /// Set the allowed value range.
    void SetRange(int minValue, int maxValue);

    /// Return the minimum allowed value.
    int GetMin() const { return m_min; }

    /// Return the maximum allowed value.
    int GetMax() const { return m_max; }

    /// Return true if the slider is vertical.
    bool IsVertical() const { return (GetWindowStyle() & wxSL_VERTICAL) != 0; }

    wxString GetStyledControlType() const override { return "StyledSlider"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    wxSize DoGetBestSize() const override;

    void OnLeftDown(wxMouseEvent& evt) override;
    void OnLeftUp(wxMouseEvent& evt) override;
    void OnMotion(wxMouseEvent& evt) override;
    void OnMouseLeave(wxMouseEvent& evt) override;
    void OnKeyDown(wxKeyEvent& evt) override;

private:
    void SetValueInternal(int value, bool sendEvent);
    void SetValueFromPoint(const wxPoint& pt, const wxRect& contentRect, bool sendEvent);

    void UpdateHandleState(const wxPoint& pt);

    wxRect GetGrooveRect(const wxRect& contentRect) const;
    wxRect GetSubPageRect(const wxRect& contentRect) const;
    wxRect GetHandleRect(const wxRect& contentRect) const;

    int PositionFromValue(int value, const wxRect& contentRect) const;
    int ValueFromPosition(int pos, const wxRect& contentRect) const;

    wxSize GetHandleSize() const;

    int m_value = 0;
    int m_min = 0;
    int m_max = 100;

    bool m_dragging = false;
    bool m_handleHovered = false;
    bool m_handlePressed = false;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
