#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/statbox.h>

namespace wxCustomization {

/// A styled container with a title and a frame around its children.
///
/// StyledGroupBox works as a parent for other widgets. The frame's top edge
/// passes through the middle of the title text and is cut out underneath it,
/// like QGroupBox or wxStaticBox.
///
/// The title text is styled through the `::title` sub-control.
///
/// Children are laid out from the client origin (as with StyledPanel), so the
/// widget's sizer should reserve space for the title, e.g. with a spacer of
/// GetTitleHeight().
class StyledGroupBox : public StyledControl {
public:
    StyledGroupBox(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxString& title = wxEmptyString,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxStaticBoxNameStr);

    void SetTitle(const wxString& title);
    wxString GetTitle() const { return m_title; }

    /// Height in pixels of the title strip at the top of the widget.
    /// Returns 0 when there is no title.
    int GetTitleHeight() const;

    wxString GetStyledControlType() const override { return "StyledGroupBox"; }

    /// A group box is a layout container; it should not take focus itself.
    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

protected:
    void OnPaint(wxPaintEvent& evt) override;
    /// Renders the frame, the title and the border cut-out into @p rect.
    /// Separated from OnPaint so tests can paint into an arbitrary DC.
    virtual void DrawGroupBox(wxDC& dc, const wxRect& rect);
    wxSize DoGetBestSize() const override;

private:
    wxFont GetTitleFont() const;
    wxRect GetTitleRect(const wxRect& rect) const;
    wxColour GetBehindColour() const;

    wxString m_title;
};

} // namespace wxCustomization
