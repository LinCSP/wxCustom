#pragma once

#include <wx/dialog.h>
#include <wx/msgdlg.h>

#include "wxCustomization/StyleSheet.h"

namespace wxCustomization {

/// A styled replacement for wxMessageBox.
/// The dialog's content area and OK button are rendered using the
/// wxCustomization style system.
class StyledMessageDialog : public wxDialog {
public:
    StyledMessageDialog(wxWindow* parent,
                        const wxString& message,
                        const wxString& caption = wxMessageBoxCaptionStr,
                        long style = wxOK | wxICON_INFORMATION,
                        const wxString& name = wxDialogNameStr);

    /// Apply a stylesheet to both the dialog content and the OK button.
    void SetStyleSheet(StyleSheet* sheet);

    /// Convenience wrapper that creates, styles and shows the dialog modally.
    static int Show(wxWindow* parent,
                    const wxString& message,
                    const wxString& caption = wxMessageBoxCaptionStr,
                    long style = wxOK | wxICON_INFORMATION,
                    StyleSheet* sheet = nullptr);

private:
    class ContentPanel;

    void OnOk(wxCommandEvent& evt);
    void OnCharHook(wxKeyEvent& evt);

    ContentPanel* m_content = nullptr;
    class StyledButton* m_okButton = nullptr;
};

} // namespace wxCustomization
