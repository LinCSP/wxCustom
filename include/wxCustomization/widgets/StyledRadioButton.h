#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/string.h>

namespace wxCustomization {

/// A styled radio button with a round `::indicator` sub-control.
///
/// Radio buttons are grouped together with standard wxWidgets logic:
/// consecutive `StyledRadioButton` siblings belong to the same group,
/// unless a button is created with the `wxRB_GROUP` style, which starts
/// a new group. Selecting one button automatically unchecks the others
/// in its group.
class StyledRadioButton : public StyledControl {
public:
    StyledRadioButton(wxWindow* parent,
                      wxWindowID id = wxID_ANY,
                      const wxString& label = wxEmptyString,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = 0,
                      const wxString& name = wxControlNameStr);

    void SetValue(bool value);
    bool GetValue() const { return IsChecked(); }

    void SetLabel(const wxString& label);
    wxString GetLabel() const;

    wxString GetStyledControlType() const override { return "StyledRadioButton"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    void Activate() override;
    void OnChar(wxKeyEvent& evt) override;
    wxSize DoGetBestSize() const override;

private:
    std::vector<StyledRadioButton*> GetGroupSiblings() const;
    void MoveSelectionInGroup(int delta);

    wxSize GetIndicatorSize(const Style& style) const;
    wxRect GetIndicatorRect(const wxRect& contentRect, const wxSize& indicatorSize) const;
    wxRect GetLabelRect(const wxRect& contentRect, const wxRect& indicatorRect) const;
    void DrawIndicator(wxDC& dc, const wxRect& rect);

    wxString m_label;
};

} // namespace wxCustomization
