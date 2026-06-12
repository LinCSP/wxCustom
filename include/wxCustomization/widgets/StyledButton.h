#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/bitmap.h>
#include <wx/string.h>

namespace wxCustomization {

/// Push button with full state support and optional icon.
class StyledButton : public StyledControl {
public:
    StyledButton(wxWindow* parent, wxWindowID id,
                 const wxString& label = wxEmptyString,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize);

    void SetLabel(const wxString& label);
    wxString GetLabel() const;

    void SetIcon(const wxBitmap& icon);
    wxBitmap GetIcon() const;

    void SetIconSize(const wxSize& size);
    wxSize GetIconSize() const;

    wxString GetStyledControlType() const override { return "StyledButton"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    void Activate() override;
    wxSize DoGetBestSize() const override;

private:
    wxBitmap GetEffectiveIcon(const Style& style) const;
    wxSize GetEffectiveIconSize(const Style& style, const wxBitmap& icon) const;

    wxBitmap m_icon;
    wxSize m_iconSize;
};

} // namespace wxCustomization
