#pragma once

#include <wx/dialog.h>
#include <wx/msgdlg.h>

#include "wxCustomization/StyleSheet.h"

namespace wxCustomization {

class StyledTitleBar;

/// A styled replacement for wxMessageBox.
/// The dialog is fully drawn by the library, including its title bar
/// (client-side decorations, like StyledFrame) — no native caption.
class StyledMessageDialog : public wxDialog {
public:
    StyledMessageDialog(wxWindow* parent,
                        const wxString& message,
                        const wxString& caption = wxMessageBoxCaptionStr,
                        long style = wxOK | wxICON_INFORMATION,
                        const wxString& name = wxDialogNameStr);

    /// Apply a stylesheet to the title bar, dialog content and the OK button.
    void SetStyleSheet(StyleSheet* sheet);

    /// The styled title bar (caption + close button; draggable).
    StyledTitleBar* GetTitleBar() const { return m_titleBar; }

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

    StyledTitleBar* m_titleBar = nullptr;
    ContentPanel* m_content = nullptr;
    class StyledButton* m_okButton = nullptr;
};

} // namespace wxCustomization
