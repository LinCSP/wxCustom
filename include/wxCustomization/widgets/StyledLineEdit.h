#pragma once

#include "wxCustomization/StyledControl.h"

#include <wx/timer.h>

namespace wxCustomization {

/// A styled single-line text input field.
///
/// Supports text entry, caret, selection, clipboard operations, password mode
/// and read-only mode. Emits `wxEVT_TEXT` when the value changes and
/// `wxEVT_TEXT_ENTER` when Enter is pressed.
class StyledLineEdit : public StyledControl {
public:
    StyledLineEdit(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxString& value = wxEmptyString,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxControlNameStr);

    ~StyledLineEdit() override;

    void SetValue(const wxString& value);
    wxString GetValue() const { return m_text; }

    void SetPasswordMode(bool password);
    bool GetPasswordMode() const { return m_password; }

    void SetReadOnly(bool readOnly);
    bool GetReadOnly() const { return m_readOnly; }

    void SetSelection(long from, long to);
    void SelectAll();
    long GetInsertionPoint() const { return m_caretPos; }
    void SetInsertionPoint(long pos);

    wxString GetStyledControlType() const override { return "StyledLineEdit"; }
    bool IsReadOnly() const override { return m_readOnly; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override;
    wxSize DoGetBestSize() const override;

    void OnChar(wxKeyEvent& evt) override;
    void OnKeyDown(wxKeyEvent& evt) override;
    void OnLeftDown(wxMouseEvent& evt) override;
    void OnLeftUp(wxMouseEvent& evt) override;
    void OnMotion(wxMouseEvent& evt) override;
    void OnSetFocus(wxFocusEvent& evt) override;
    void OnKillFocus(wxFocusEvent& evt) override;

private:
    void InsertText(const wxString& text);
    void DeleteSelection();
    void DeleteRange(long from, long to);

    void MoveCaretLeft(bool extendSelection);
    void MoveCaretRight(bool extendSelection);
    void MoveCaretHome(bool extendSelection);
    void MoveCaretEnd(bool extendSelection);

    long PositionFromPoint(const wxPoint& pt) const;
    int GetCaretPixelOffset(long pos) const;
    wxString GetDisplayText() const;
    void EnsureCaretVisible();
    void UpdateCaret();
    void OnCaretTimer(wxTimerEvent& evt);

    void EmitTextEvent();
    void EmitTextEnterEvent();

    wxString m_text;
    long m_caretPos = 0;
    long m_selectionStart = 0;
    long m_selectionEnd = 0;
    int m_scrollOffset = 0;
    bool m_password = false;
    bool m_readOnly = false;

    bool m_mouseSelecting = false;
    long m_mouseSelectionAnchor = 0;

    bool m_caretVisible = true;
    wxTimer m_caretTimer;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
