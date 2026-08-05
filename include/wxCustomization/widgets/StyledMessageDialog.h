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

    /// Shows the dialog modally, dimming the parent window with a scrim so
    /// the dialog never blends into it. The scrim is our own window, so this
    /// works identically on all platforms (no compositor tricks).
    int ShowModal() override;

    /// Convenience wrapper that creates, styles and shows the dialog modally.
    static int Show(wxWindow* parent,
                    const wxString& message,
                    const wxString& caption = wxMessageBoxCaptionStr,
                    long style = wxOK | wxICON_INFORMATION,
                    StyleSheet* sheet = nullptr);

private:
    class ContentPanel;
    class ScrimWindow;

    void OnOk(wxCommandEvent& evt);
    void OnCharHook(wxKeyEvent& evt);
    void ShowDimScrim();
    void HideDimScrim();

    StyledTitleBar* m_titleBar = nullptr;
    class StyledControl* m_background = nullptr;
    ContentPanel* m_content = nullptr;
    class StyledButton* m_okButton = nullptr;
    ScrimWindow* m_scrim = nullptr;
};

} // namespace wxCustomization
