#pragma once

#include "wxCustomization/StyledControl.h"

namespace wxCustomization {

/// A styled text label.
///
/// Supports color, font-size, text-align, text-decoration, background-color,
/// border and padding.
class StyledLabel : public StyledControl {
public:
    StyledLabel(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxString& label = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxControlNameStr);

    void SetLabel(const wxString& label);
    wxString GetLabel() const;

    wxString GetStyledControlType() const override { return "StyledLabel"; }

    /// A label should not take focus itself.
    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    wxSize DoGetBestSize() const override;

private:
    wxString m_label;
};

} // namespace wxCustomization
