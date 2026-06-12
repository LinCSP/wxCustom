#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/string.h>

namespace wxCustomization {

/// Three possible checkbox states.
enum class CheckState {
    Unchecked,
    Checked,
    Indeterminate
};

/// A styled check box with a custom `::indicator` sub-control.
///
/// Supports two-state use via `SetValue()`/`GetValue()` and three-state use
/// via `Set3StateValue()`/`Get3StateValue()`. The indicator can be styled with
/// the pseudo-states `:unchecked`, `:checked`, `:indeterminate`, `:hover` and
/// `:pressed`.
class StyledCheckBox : public StyledControl {
public:
    StyledCheckBox(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxString& label = wxEmptyString,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxControlNameStr);

    void SetValue(bool value);
    bool GetValue() const { return Get3StateValue() == CheckState::Checked; }

    void Set3StateValue(CheckState state);
    CheckState Get3StateValue() const { return m_state; }

    void SetLabel(const wxString& label);
    wxString GetLabel() const;

    wxString GetStyledControlType() const override { return "StyledCheckBox"; }
    bool IsIndeterminate() const override { return m_state == CheckState::Indeterminate; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    void Activate() override;
    wxSize DoGetBestSize() const override;

private:
    wxSize GetIndicatorSize(const Style& style) const;
    wxRect GetIndicatorRect(const wxRect& contentRect, const wxSize& indicatorSize) const;
    wxRect GetLabelRect(const wxRect& contentRect, const wxRect& indicatorRect) const;
    void DrawIndicator(wxDC& dc, const wxRect& rect);

    wxString m_label;
    CheckState m_state = CheckState::Unchecked;
};

} // namespace wxCustomization
