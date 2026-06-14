#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/arrstr.h>

namespace wxCustomization {

class StyledComboBoxPopup;

/// A styled drop-down list widget.
///
/// Replaces wxComboBox for single-selection scenarios. Supports sub-control
/// styling via `::drop-down` and `::down-arrow`, keyboard navigation and
/// emits `wxEVT_COMBOBOX` when the selection changes.
class StyledComboBox : public StyledControl {
public:
    StyledComboBox(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxArrayString& choices = wxArrayString(),
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxControlNameStr);

    ~StyledComboBox() override;

    /// Add a new choice to the end of the list.
    void Append(const wxString& item);

    /// Remove all choices.
    void Clear();

    /// Return the number of choices.
    size_t GetCount() const { return m_choices.GetCount(); }

    /// Set the current selection. Pass -1 to clear the selection.
    void SetSelection(int n);

    /// Return the index of the current selection or -1 if none.
    int GetSelection() const { return m_selection; }

    /// Return the text of the current selection.
    wxString GetValue() const;

    wxString GetStyledControlType() const override { return "StyledComboBox"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    wxSize DoGetBestSize() const override;

    void OnLeftUp(wxMouseEvent& evt) override;
    void OnKeyDown(wxKeyEvent& evt) override;
    void OnChar(wxKeyEvent& evt) override;
    void OnKillFocus(wxFocusEvent& evt) override;
    void Activate() override;

private:
    friend class StyledComboBoxPopup;

    void ShowPopup();
    void HidePopup();
    void TogglePopup();
    void OnPopupSelection(int index);
    void OnPopupClosed();

    void SelectItem(int index, bool sendEvent);

protected:
    void MoveSelection(int delta);

    wxRect GetDropDownRect(const wxRect& contentRect) const;
    wxRect GetTextRect(const wxRect& contentRect, const wxRect& dropDownRect) const;
    wxSize GetDropDownSize(const wxRect& contentRect) const;

    void DrawDropDownArrow(wxDC& dc, const wxRect& arrowRect);

    wxArrayString m_choices;
    int m_selection = -1;
    bool m_popupOpen = false;

    StyledComboBoxPopup* m_popup = nullptr;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
