#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/StyledControl.h"
#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/wxCustomization.h"

class DemoWidget : public wxCustomization::StyledControl {
public:
    DemoWidget(wxWindow* parent, wxWindowID id, const wxString& label)
        : wxCustomization::StyledControl(parent, id, wxDefaultPosition, wxSize(120, 40))
        , m_label(label)
    {
        SetLabel(label);
    }

    wxString GetStyledControlType() const override { return "DemoWidget"; }

protected:
    void DrawContent(wxDC& dc, const wxRect& rect) override
    {
        const wxCustomization::Style& style = GetCurrentStyle();
        dc.SetFont(style.font.IsOk() ? style.font
                                     : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        dc.SetTextForeground(style.color.IsOk() ? style.color : *wxBLACK);
        dc.DrawLabel(m_label, wxNullBitmap, rect, wxALIGN_CENTER);
    }

    wxSize DoGetBestSize() const override
    {
        return wxSize(120, 40);
    }

private:
    wxString m_label;
};

class DemoFrame : public wxFrame {
public:
    DemoFrame(wxCustomization::StyleSheet* styleSheet)
        : wxFrame(nullptr, wxID_ANY, "wxCustomization Demo",
                  wxDefaultPosition, wxSize(800, 600))
        , m_styleSheet(styleSheet)
    {
        wxPanel* panel = new wxPanel(this, wxID_ANY);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* label = new wxStaticText(
            panel, wxID_ANY,
            wxString::Format("wxCustomization %s", wxCustomization::GetVersionString()));
        sizer->Add(label, 0, wxALL | wxCENTER, 20);

        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        for (int i = 1; i <= 3; ++i) {
            DemoWidget* widget = new DemoWidget(panel, wxID_ANY,
                                                wxString::Format("Widget %d", i));
            widget->SetStyleSheet(m_styleSheet);
            row->Add(widget, 0, wxALL, 10);
        }
        sizer->Add(row, 0, wxALIGN_CENTER);

        wxCustomization::StyledButton* button =
            new wxCustomization::StyledButton(panel, wxID_ANY, "Click me");
        button->SetStyleSheet(m_styleSheet);
        button->Bind(wxEVT_BUTTON, &DemoFrame::OnButtonClick, this);
        sizer->Add(button, 0, wxALL | wxCENTER, 10);

        panel->SetSizer(sizer);
    }

private:
    void OnButtonClick(wxCommandEvent& /*evt*/)
    {
        wxMessageBox("StyledButton works!", "wxCustomization Demo", wxOK | wxICON_INFORMATION, this);
    }

    wxCustomization::StyleSheet* m_styleSheet;
};

class DemoApp : public wxApp {
public:
    bool OnInit() override
    {
        if (!LoadTheme()) {
            wxLogWarning("Failed to load theme file");
        }

        DemoFrame* frame = new DemoFrame(&m_styleSheet);
        frame->Show(true);
        return true;
    }

private:
    bool LoadTheme()
    {
        wxFileName stylePath(wxStandardPaths::Get().GetExecutablePath());
        stylePath.SetFullName("theme.qss");
        return m_styleSheet.Load(stylePath.GetFullPath());
    }

    wxCustomization::StyleSheet m_styleSheet;
};

wxIMPLEMENT_APP(DemoApp);
