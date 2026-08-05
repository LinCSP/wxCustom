#include "wxCustomization/widgets/StyledMessageDialog.h"

#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/widgets/StyledTitleBar.h"

#include <wx/artprov.h>
#include <wx/dc.h>
#include <wx/sizer.h>
#include <wx/settings.h>

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

    m_content = new ContentPanel(this, message, style);
    m_okButton = new StyledButton(this, wxID_OK, "OK");
    m_okButton->Bind(wxEVT_BUTTON, &StyledMessageDialog::OnOk, this);

    Bind(wxEVT_CHAR_HOOK, &StyledMessageDialog::OnCharHook, this);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_titleBar, 0, wxEXPAND);
    mainSizer->Add(m_content, 1, wxEXPAND | wxALL, 0);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(m_okButton, 0, wxALL, 12);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 8);

    SetSizer(mainSizer);
    Fit();
    CentreOnParent();

    m_okButton->SetFocus();
}

void StyledMessageDialog::SetStyleSheet(StyleSheet* sheet)
{
    m_titleBar->SetStyleSheet(sheet);
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
