#pragma once

#include "wxCustomization/StyledControl.h"

namespace wxCustomization {

/// A styled toggle button.
///
/// Supports the same visual states as StyledButton plus `:checked`.
/// Clicking or pressing Space/Enter toggles the value and emits
/// `wxEVT_TOGGLEBUTTON`.
class StyledToggleButton : public StyledControl {
public:
    StyledToggleButton(wxWindow* parent,
                       wxWindowID id = wxID_ANY,
                       const wxString& label = wxEmptyString,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       long style = 0,
                       const wxString& name = wxControlNameStr);

    void SetValue(bool value);
    bool GetValue() const { return IsChecked(); }

    wxString GetStyledControlType() const override { return "StyledToggleButton"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    void Activate() override;
    wxSize DoGetBestSize() const override;

private:
    wxString m_label;
};

} // namespace wxCustomization
