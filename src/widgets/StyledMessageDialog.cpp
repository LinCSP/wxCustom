#include "wxCustomization/widgets/StyledMessageDialog.h"

#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/widgets/StyledTitleBar.h"

#include <wx/artprov.h>
#include <wx/dc.h>
#include <wx/popupwin.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/toplevel.h>

#include <algorithm>

namespace wxCustomization {

namespace {

wxBitmap GetMessageIcon(long style)
{
    wxString artId = wxART_INFORMATION;
    if (style & wxICON_ERROR) {
        artId = wxART_ERROR;
    } else if (style & wxICON_WARNING) {
        artId = wxART_WARNING;
    } else if (style & wxICON_QUESTION) {
        artId = wxART_QUESTION;
    }

    return wxArtProvider::GetBitmap(artId, wxART_MESSAGE_BOX, wxSize(32, 32));
}

} // namespace

class StyledMessageDialog::ContentPanel : public StyledControl {
public:
    ContentPanel(wxWindow* parent, const wxString& message, long style)
        : StyledControl(parent, wxID_ANY)
        , m_message(message)
        , m_icon(GetMessageIcon(style))
    {
    }

    wxString GetStyledControlType() const override { return "StyledMessageDialog"; }

protected:
    // The message panel draws no surface of its own: the background panel
    // below it is the bordered card, so only the icon and text are painted.
    void DrawBackground(wxDC& /*dc*/, const wxRect& /*rect*/, const Style& /*style*/) override
    {
    }

    void DrawBorder(wxDC& /*dc*/, const wxRect& /*rect*/, const Style& /*style*/) override
    {
    }

    void DrawContent(wxDC& dc, const wxRect& rect) override
    {
        const Style& style = GetCurrentStyle();

        dc.SetFont(style.font.IsOk() ? style.font
                                     : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        dc.SetTextForeground(style.color.IsOk() ? style.color : *wxBLACK);

        int x = rect.x;
        const int spacing = m_icon.IsOk() ? style.spacing : 0;

        if (m_icon.IsOk()) {
            const int iconY = rect.y + (rect.height - m_icon.GetHeight()) / 2;
            dc.DrawBitmap(m_icon, x, iconY, true);
            x += m_icon.GetWidth() + spacing;
        }

        const int textWidth = rect.width - (x - rect.x);
        if (textWidth > 0) {
            const wxRect textRect(x, rect.y, textWidth, rect.height);
            dc.DrawLabel(m_message, wxNullBitmap, textRect,
                         wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
        }
    }

    wxSize DoGetBestSize() const override
    {
        const Style& style = GetCurrentStyle();
        const wxFont font = style.font.IsOk()
                                ? style.font
                                : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

        int textWidth = 0;
        int textHeight = 0;
        GetTextExtent(m_message, &textWidth, &textHeight,
                      nullptr, nullptr, const_cast<wxFont*>(&font));

        int width = textWidth;
        int height = textHeight;

        if (m_icon.IsOk()) {
            width += m_icon.GetWidth() + style.spacing;
            height = std::max(height, m_icon.GetHeight());
        }

        width += style.borderLeftWidth + style.borderRightWidth
                 + style.paddingLeft + style.paddingRight;
        height += style.borderTopWidth + style.borderBottomWidth
                  + style.paddingTop + style.paddingBottom;

        return wxSize(std::max(width, 360), std::max(height, 64));
    }

private:
    wxString m_message;
    wxBitmap m_icon;
};

namespace {

/// Paints the full StyledMessageDialog surface (background and border) over
/// the whole dialog body, so the dialog reads as a bordered card and never
/// blends into the main window. On Wayland the wxDialog's own background is
/// transparent (RGBA visual), so this panel is also what paints the body at
/// all.
class BackgroundPanel : public StyledControl {
public:
    explicit BackgroundPanel(wxWindow* parent)
        : StyledControl(parent, wxID_ANY)
    {
    }

    wxString GetStyledControlType() const override { return "StyledMessageDialog"; }

    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

protected:
    wxSize DoGetBestSize() const override
    {
        // A wxControl does not automatically use its sizer for best size,
        // but a panel-like container should.
        if (wxSizer* sizer = GetSizer()) {
            return sizer->CalcMin();
        }
        return wxControl::DoGetBestSize();
    }
};

} // namespace

/// Semi-transparent black overlay covering the parent top-level window while
/// the dialog is modal. Implemented with plain wx windows (popup + uniform
/// transparency), so it works the same on every platform.
class StyledMessageDialog::ScrimWindow : public wxPopupWindow {
public:
    explicit ScrimWindow(wxWindow* topLevel)
        : wxPopupWindow(topLevel, wxBORDER_NONE)
    {
        SetBackgroundColour(*wxBLACK);
        SetSize(topLevel->GetScreenRect());
    }

    /// Opacity of the black overlay (0-255); 100 keeps the parent readable.
    static constexpr int kDimAlpha = 100;
};

StyledMessageDialog::StyledMessageDialog(wxWindow* parent,
                                         const wxString& message,
                                         const wxString& caption,
                                         long style,
                                         const wxString& name)
    // No native decorations at all: the dialog's title bar is drawn by the
    // library (StyledTitleBar with just the close button).
    : wxDialog(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize,
               wxNO_BORDER, name)
{
    m_titleBar = new StyledTitleBar(this, wxID_ANY);
    m_titleBar->SetTitle(caption);
    m_titleBar->SetCaptionButtons(1 << StyledTitleBar::BtnClose);

    // The background panel paints the dialog background over the whole body:
    // on Wayland the wxDialog's own background is transparent, so without it
    // the gaps would show through to the desktop.
    m_background = new BackgroundPanel(this);

    m_content = new ContentPanel(m_background, message, style);
    m_okButton = new StyledButton(m_background, wxID_OK, "OK");
    m_okButton->Bind(wxEVT_BUTTON, &StyledMessageDialog::OnOk, this);

    Bind(wxEVT_CHAR_HOOK, &StyledMessageDialog::OnCharHook, this);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(m_okButton, 0, wxALL, 12);

    wxBoxSizer* bodySizer = new wxBoxSizer(wxVERTICAL);
    bodySizer->Add(m_content, 1, wxEXPAND);
    bodySizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 8);
    m_background->SetSizer(bodySizer);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_titleBar, 0, wxEXPAND);
    mainSizer->Add(m_background, 1, wxEXPAND);

    SetSizer(mainSizer);
    Fit();
    CentreOnParent();

    m_okButton->SetFocus();
}

void StyledMessageDialog::SetStyleSheet(StyleSheet* sheet)
{
    m_titleBar->SetStyleSheet(sheet);
    m_background->SetStyleSheet(sheet);
    m_content->SetStyleSheet(sheet);
    m_okButton->SetStyleSheet(sheet);
    Layout();
    Fit();

    const Style& contentStyle = m_content->GetCurrentStyle();
    if (contentStyle.backgroundColor.IsOk()) {
        SetBackgroundColour(contentStyle.backgroundColor);
    }

    // Ensure the caption isn't truncated.
    wxSize size = GetSize();
    if (size.x < 420) {
        size.x = 420;
        SetSize(size);
    }

    CentreOnParent();
}

int StyledMessageDialog::Show(wxWindow* parent,
                              const wxString& message,
                              const wxString& caption,
                              long style,
                              StyleSheet* sheet)
{
    StyledMessageDialog dlg(parent, message, caption, style);
    if (sheet) {
        dlg.SetStyleSheet(sheet);
    }
    return dlg.ShowModal();
}

int StyledMessageDialog::ShowModal()
{
    ShowDimScrim();
    const int result = wxDialog::ShowModal();
    HideDimScrim();
    return result;
}

void StyledMessageDialog::ShowDimScrim()
{
    if (m_scrim != nullptr) {
        return;
    }
    wxWindow* parent = GetParent();
    if (parent == nullptr) {
        return;
    }
    wxWindow* topLevel = wxGetTopLevelParent(parent);
    if (topLevel == nullptr) {
        return;
    }

    m_scrim = new ScrimWindow(topLevel);
    // If uniform transparency is unsupported, an opaque black window would
    // just hide the parent — better to skip dimming entirely.
    if (!m_scrim->SetTransparent(ScrimWindow::kDimAlpha)) {
        m_scrim->Destroy();
        m_scrim = nullptr;
        return;
    }
    m_scrim->Show();
}

void StyledMessageDialog::HideDimScrim()
{
    if (m_scrim == nullptr) {
        return;
    }
    m_scrim->Destroy();
    m_scrim = nullptr;
}

void StyledMessageDialog::OnOk(wxCommandEvent& /*evt*/)
{
    EndModal(wxID_OK);
}

void StyledMessageDialog::OnCharHook(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();
    if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
        EndModal(wxID_OK);
        return;
    }
    if (key == WXK_ESCAPE) {
        EndModal(wxID_CANCEL);
        return;
    }
    evt.Skip();
}

} // namespace wxCustomization
