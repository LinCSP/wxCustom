#include "wxCustomization/widgets/StyledLineEdit.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/clipbrd.h>
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include <algorithm>
#include <cctype>

namespace wxCustomization {

namespace {

wxString MaskPassword(const wxString& text)
{
    return wxString(wxT('\u2022'), text.length());
}

} // namespace

wxBEGIN_EVENT_TABLE(StyledLineEdit, StyledControl)
    EVT_TIMER(wxID_ANY, StyledLineEdit::OnCaretTimer)
wxEND_EVENT_TABLE()

StyledLineEdit::StyledLineEdit(wxWindow* parent,
                               wxWindowID id,
                               const wxString& value,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_caretTimer(this)
{
    SetValue(value);
    SetAccessibleRole(wxROLE_SYSTEM_TEXT);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

StyledLineEdit::~StyledLineEdit()
{
    if (m_caretTimer.IsRunning()) {
        m_caretTimer.Stop();
    }
}

void StyledLineEdit::SetValue(const wxString& value)
{
    m_text = value;
    m_caretPos = m_text.length();
    m_selectionStart = 0;
    m_selectionEnd = 0;
    m_scrollOffset = 0;
    EnsureCaretVisible();
    InvalidateBestSize();
    Refresh();
}

void StyledLineEdit::SetPasswordMode(bool password)
{
    if (m_password != password) {
        m_password = password;
        EnsureCaretVisible();
        Refresh();
    }
}

void StyledLineEdit::SetReadOnly(bool readOnly)
{
    if (m_readOnly != readOnly) {
        m_readOnly = readOnly;
        ApplyStyle(wxEmptyString);
        Refresh();
    }
}

void StyledLineEdit::SetSelection(long from, long to)
{
    const long len = static_cast<long>(m_text.length());
    from = std::clamp(from, 0L, len);
    to = std::clamp(to, 0L, len);
    m_selectionStart = std::min(from, to);
    m_selectionEnd = std::max(from, to);
    m_caretPos = to;
    EnsureCaretVisible();
    Refresh();
}

void StyledLineEdit::SelectAll()
{
    SetSelection(0, static_cast<long>(m_text.length()));
}

void StyledLineEdit::GetSelection(long* from, long* to) const
{
    if (from != nullptr) {
        *from = m_selectionStart;
    }
    if (to != nullptr) {
        *to = m_selectionEnd;
    }
}

void StyledLineEdit::SetInsertionPoint(long pos)
{
    const long len = static_cast<long>(m_text.length());
    m_caretPos = std::clamp(pos, 0L, len);
    m_selectionStart = m_caretPos;
    m_selectionEnd = m_caretPos;
    EnsureCaretVisible();
    Refresh();
}

void StyledLineEdit::DrawContent(wxDC& dc, const wxRect& rect)
{
    EnsureCaretVisible();

    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    const wxColour textColor = style.color.IsOk() ? style.color : *wxBLACK;
    const wxColour selectionColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    const wxColour selectionTextColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);

    dc.SetFont(font);

    const wxString displayText = GetDisplayText();

    // Selection background
    if (m_selectionStart != m_selectionEnd) {
        const int selStartX = GetCaretPixelOffset(m_selectionStart) - m_scrollOffset;
        const int selEndX = GetCaretPixelOffset(m_selectionEnd) - m_scrollOffset;
        wxRect selRect(rect.x + selStartX, rect.y, selEndX - selStartX, rect.height);
        selRect.Intersect(rect);
        if (selRect.width > 0 && selRect.height > 0) {
            dc.SetBrush(wxBrush(selectionColor));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(selRect);
        }
    }

    // Text
    dc.SetTextForeground(textColor);
    dc.SetClippingRegion(rect);
    dc.DrawText(displayText, rect.x - m_scrollOffset, rect.y + (rect.height - dc.GetCharHeight()) / 2);
    dc.DestroyClippingRegion();

    // Caret
    if (m_focused && m_caretVisible && !m_readOnly) {
        const int caretX = rect.x + GetCaretPixelOffset(m_caretPos) - m_scrollOffset;
        if (caretX >= rect.x && caretX <= rect.x + rect.width) {
            dc.SetPen(wxPen(textColor, 1, wxPENSTYLE_SOLID));
            dc.DrawLine(caretX, rect.y + 2, caretX, rect.y + rect.height - 2);
        }
    }
}

wxSize StyledLineEdit::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    int charHeight = 0;
    GetTextExtent("Ag", nullptr, &charHeight, nullptr, nullptr, const_cast<wxFont*>(&font));

    int width = 120;
    int height = charHeight;

    width += style.borderLeftWidth + style.borderRightWidth
             + style.paddingLeft + style.paddingRight;
    height += style.borderTopWidth + style.borderBottomWidth
              + style.paddingTop + style.paddingBottom;

    if (style.IsSet(Property::MinWidth) && width < style.minWidth) {
        width = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && height < style.minHeight) {
        height = style.minHeight;
    }
    if (style.IsSet(Property::Width) && style.width > 0) {
        width = style.width;
    }
    if (style.IsSet(Property::Height) && style.height > 0) {
        height = style.height;
    }

    return wxSize(std::max(width, 32), std::max(height, 16));
}

void StyledLineEdit::OnChar(wxKeyEvent& evt)
{
    if (m_readOnly || !IsEnabled()) {
        evt.Skip();
        return;
    }

    const wxChar uniChar = evt.GetUnicodeKey();
    const int key = evt.GetKeyCode();

    // Navigation and special keys are handled in OnKeyDown.
    if (uniChar == 0 ||
        key == WXK_TAB ||
        key == WXK_RETURN ||
        key == WXK_NUMPAD_ENTER ||
        key == WXK_BACK ||
        key == WXK_DELETE ||
        key == WXK_NUMPAD_DELETE) {
        evt.Skip();
        return;
    }

    // Ctrl+letter shortcuts (e.g. Ctrl+A/C/V/X) are handled in OnKeyDown.
    if (evt.ControlDown() && !evt.AltDown() && uniChar >= WXK_SPACE && uniChar < 127) {
        evt.Skip();
        return;
    }

    if (uniChar < WXK_SPACE) {
        evt.Skip();
        return;
    }

    wxString text;
    text += uniChar;
    InsertText(text);
}

void StyledLineEdit::OnKeyDown(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();
    const bool ctrl = evt.ControlDown();
    const bool shift = evt.ShiftDown();

    switch (key) {
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            MoveCaretLeft(shift);
            return;

        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            MoveCaretRight(shift);
            return;

        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            MoveCaretHome(shift);
            return;

        case WXK_END:
        case WXK_NUMPAD_END:
            MoveCaretEnd(shift);
            return;

        case WXK_BACK:
            if (!m_readOnly && IsEnabled()) {
                if (m_selectionStart != m_selectionEnd) {
                    DeleteSelection();
                } else if (m_caretPos > 0) {
                    DeleteRange(m_caretPos - 1, m_caretPos);
                }
            }
            return;

        case WXK_DELETE:
        case WXK_NUMPAD_DELETE:
            if (!m_readOnly && IsEnabled()) {
                if (m_selectionStart != m_selectionEnd) {
                    DeleteSelection();
                } else if (m_caretPos < static_cast<long>(m_text.length())) {
                    DeleteRange(m_caretPos, m_caretPos + 1);
                }
            }
            return;

        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            EmitTextEnterEvent();
            evt.Skip();
            return;

        case 'A':
            if (ctrl && !evt.AltDown()) {
                SelectAll();
                return;
            }
            break;

        case 'C':
            if (ctrl && !evt.AltDown()) {
                if (m_selectionStart != m_selectionEnd && wxTheClipboard->Open()) {
                    wxTheClipboard->SetData(new wxTextDataObject(
                        m_text.Mid(m_selectionStart, m_selectionEnd - m_selectionStart)));
                    wxTheClipboard->Close();
                }
                return;
            }
            break;

        case 'X':
            if (ctrl && !evt.AltDown() && !m_readOnly && IsEnabled()) {
                if (m_selectionStart != m_selectionEnd) {
                    if (wxTheClipboard->Open()) {
                        wxTheClipboard->SetData(new wxTextDataObject(
                            m_text.Mid(m_selectionStart, m_selectionEnd - m_selectionStart)));
                        wxTheClipboard->Close();
                    }
                    DeleteSelection();
                }
                return;
            }
            break;

        case 'V':
            if (ctrl && !evt.AltDown() && !m_readOnly && IsEnabled()) {
                if (wxTheClipboard->Open()) {
                    if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                        wxTextDataObject data;
                        wxTheClipboard->GetData(data);
                        InsertText(data.GetText());
                    }
                    wxTheClipboard->Close();
                }
                return;
            }
            break;

        default:
            break;
    }

    evt.Skip();
}

void StyledLineEdit::OnLeftDown(wxMouseEvent& evt)
{
    SetFocus();
    const long pos = PositionFromPoint(evt.GetPosition());
    if (evt.ShiftDown()) {
        const long anchor = m_caretPos;
        SetSelection(anchor, pos);
    } else {
        m_mouseSelectionAnchor = pos;
        m_mouseSelecting = true;
        SetInsertionPoint(pos);
    }
    CaptureMouse();
}

void StyledLineEdit::OnLeftUp(wxMouseEvent& evt)
{
    if (HasCapture()) {
        ReleaseMouse();
    }
    m_mouseSelecting = false;
    evt.Skip();
}

void StyledLineEdit::OnMotion(wxMouseEvent& evt)
{
    if (m_mouseSelecting && evt.LeftIsDown()) {
        const long pos = PositionFromPoint(evt.GetPosition());
        SetSelection(m_mouseSelectionAnchor, pos);
    } else {
        evt.Skip();
    }
}

void StyledLineEdit::OnSetFocus(wxFocusEvent& evt)
{
    StyledControl::OnSetFocus(evt);
    m_caretVisible = true;
    m_caretTimer.Start(530);
}

void StyledLineEdit::OnKillFocus(wxFocusEvent& evt)
{
    StyledControl::OnKillFocus(evt);
    if (m_caretTimer.IsRunning()) {
        m_caretTimer.Stop();
    }
    m_caretVisible = false;
    m_mouseSelecting = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
    Refresh();
}

void StyledLineEdit::InsertText(const wxString& text)
{
    if (m_readOnly || text.empty()) {
        return;
    }

    DeleteSelection();
    m_text.insert(m_caretPos, text);
    m_caretPos += text.length();
    m_selectionStart = m_caretPos;
    m_selectionEnd = m_caretPos;
    EnsureCaretVisible();
    Refresh();
    EmitTextEvent();
}

void StyledLineEdit::DeleteSelection()
{
    if (m_selectionStart == m_selectionEnd) {
        return;
    }
    DeleteRange(m_selectionStart, m_selectionEnd);
}

void StyledLineEdit::DeleteRange(long from, long to)
{
    if (from >= to || from < 0 || to > static_cast<long>(m_text.length())) {
        return;
    }
    m_text.erase(from, to - from);
    m_caretPos = from;
    m_selectionStart = from;
    m_selectionEnd = from;
    EnsureCaretVisible();
    Refresh();
    EmitTextEvent();
}

void StyledLineEdit::MoveCaretLeft(bool extendSelection)
{
    if (m_caretPos > 0) {
        --m_caretPos;
    }
    if (!extendSelection) {
        m_selectionStart = m_caretPos;
        m_selectionEnd = m_caretPos;
    } else {
        m_selectionEnd = m_caretPos;
    }
    EnsureCaretVisible();
    Refresh();
}

void StyledLineEdit::MoveCaretRight(bool extendSelection)
{
    if (m_caretPos < static_cast<long>(m_text.length())) {
        ++m_caretPos;
    }
    if (!extendSelection) {
        m_selectionStart = m_caretPos;
        m_selectionEnd = m_caretPos;
    } else {
        m_selectionEnd = m_caretPos;
    }
    EnsureCaretVisible();
    Refresh();
}

void StyledLineEdit::MoveCaretHome(bool extendSelection)
{
    m_caretPos = 0;
    if (!extendSelection) {
        m_selectionStart = 0;
        m_selectionEnd = 0;
    } else {
        m_selectionEnd = 0;
    }
    EnsureCaretVisible();
    Refresh();
}

void StyledLineEdit::MoveCaretEnd(bool extendSelection)
{
    m_caretPos = static_cast<long>(m_text.length());
    if (!extendSelection) {
        m_selectionStart = m_caretPos;
        m_selectionEnd = m_caretPos;
    } else {
        m_selectionEnd = m_caretPos;
    }
    EnsureCaretVisible();
    Refresh();
}

long StyledLineEdit::PositionFromPoint(const wxPoint& pt) const
{
    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    wxClientDC dc(const_cast<StyledLineEdit*>(this));
    dc.SetFont(font);

    const wxRect contentRect = GetContentRect();
    const int localX = pt.x - contentRect.x + m_scrollOffset;
    const wxString displayText = GetDisplayText();

    long bestPos = 0;
    int bestDist = localX;
    for (size_t i = 0; i <= displayText.length(); ++i) {
        int x = 0;
        dc.GetTextExtent(displayText.Left(i), &x, nullptr);
        const int dist = std::abs(x - localX);
        if (dist < bestDist) {
            bestDist = dist;
            bestPos = static_cast<long>(i);
        }
    }
    return bestPos;
}

int StyledLineEdit::GetCaretPixelOffset(long pos) const
{
    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    wxClientDC dc(const_cast<StyledLineEdit*>(this));
    dc.SetFont(font);

    const wxString displayText = GetDisplayText();
    int x = 0;
    dc.GetTextExtent(displayText.Left(pos), &x, nullptr);
    return x;
}

wxString StyledLineEdit::GetDisplayText() const
{
    return m_password ? MaskPassword(m_text) : m_text;
}

void StyledLineEdit::EnsureCaretVisible()
{
    const wxRect contentRect = GetContentRect();
    if (contentRect.width <= 0) {
        return;
    }

    const int textWidth = GetCaretPixelOffset(static_cast<long>(m_text.length()));
    if (textWidth <= contentRect.width) {
        m_scrollOffset = 0;
        return;
    }

    const int caretX = GetCaretPixelOffset(m_caretPos);
    const int visibleCaretX = caretX - m_scrollOffset;

    if (visibleCaretX < 0) {
        m_scrollOffset = caretX;
    } else if (visibleCaretX > contentRect.width) {
        m_scrollOffset = caretX - contentRect.width;
    }

    if (m_scrollOffset < 0) {
        m_scrollOffset = 0;
    }
}

void StyledLineEdit::UpdateCaret()
{
    m_caretVisible = !m_caretVisible;
    Refresh();
}

void StyledLineEdit::OnCaretTimer(wxTimerEvent& /*evt*/)
{
    UpdateCaret();
}

void StyledLineEdit::EmitTextEvent()
{
    wxCommandEvent event(wxEVT_TEXT, GetId());
    event.SetEventObject(this);
    event.SetString(m_text);
    ProcessWindowEvent(event);
}

void StyledLineEdit::EmitTextEnterEvent()
{
    wxCommandEvent event(wxEVT_TEXT_ENTER, GetId());
    event.SetEventObject(this);
    event.SetString(m_text);
    ProcessWindowEvent(event);
}

} // namespace wxCustomization
