#pragma once

#include "wxCustomization/StyledControl.h"

namespace wxCustomization {

/// A styled container widget.
///
/// StyledPanel works as a parent for other widgets and supports the same
/// visual properties as the rest of the library: background, border,
/// border-radius, padding and margin.
class StyledPanel : public StyledControl {
public:
    StyledPanel(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxPanelNameStr);

    wxString GetStyledControlType() const override { return "StyledPanel"; }

    /// A panel is a layout container; it should not take focus itself.
    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

protected:
    wxSize DoGetBestSize() const override;
};

} // namespace wxCustomization
