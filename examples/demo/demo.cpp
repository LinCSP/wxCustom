#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>
#include <wx/tglbtn.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/StyledControl.h"
#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/widgets/StyledMessageDialog.h"
#include "wxCustomization/widgets/StyledPanel.h"
#include "wxCustomization/widgets/StyledLabel.h"
#include "wxCustomization/widgets/StyledToggleButton.h"
#include "wxCustomization/widgets/StyledCheckBox.h"
#include "wxCustomization/widgets/StyledRadioButton.h"
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
        wxCustomization::StyledPanel* panel =
            new wxCustomization::StyledPanel(this, wxID_ANY);
        panel->SetStyleSheet(m_styleSheet);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxCustomization::StyledLabel* label = new wxCustomization::StyledLabel(
            panel, wxID_ANY,
            wxString::Format("wxCustomization %s", wxCustomization::GetVersionString()));
        label->SetStyleSheet(m_styleSheet);
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

        wxCustomization::StyledToggleButton* toggle =
            new wxCustomization::StyledToggleButton(panel, wxID_ANY, "Toggle me");
        toggle->SetStyleSheet(m_styleSheet);
        toggle->Bind(wxEVT_TOGGLEBUTTON, &DemoFrame::OnToggle, this);
        sizer->Add(toggle, 0, wxALL | wxCENTER, 10);

        wxCustomization::StyledCheckBox* checkBox =
            new wxCustomization::StyledCheckBox(panel, wxID_ANY, "Check me");
        checkBox->SetStyleSheet(m_styleSheet);
        checkBox->Bind(wxEVT_CHECKBOX, &DemoFrame::OnCheckBox, this);
        sizer->Add(checkBox, 0, wxALL | wxCENTER, 10);

        wxBoxSizer* radioSizer = new wxBoxSizer(wxHORIZONTAL);
        wxCustomization::StyledRadioButton* radio1 =
            new wxCustomization::StyledRadioButton(panel, wxID_ANY, "Radio 1",
                                                   wxDefaultPosition, wxDefaultSize,
                                                   wxRB_GROUP);
        radio1->SetStyleSheet(m_styleSheet);
        radio1->Bind(wxEVT_RADIOBUTTON, &DemoFrame::OnRadio, this);
        radioSizer->Add(radio1, 0, wxALL, 10);

        wxCustomization::StyledRadioButton* radio2 =
            new wxCustomization::StyledRadioButton(panel, wxID_ANY, "Radio 2");
        radio2->SetStyleSheet(m_styleSheet);
        radio2->Bind(wxEVT_RADIOBUTTON, &DemoFrame::OnRadio, this);
        radioSizer->Add(radio2, 0, wxALL, 10);

        sizer->Add(radioSizer, 0, wxALIGN_CENTER);

        panel->SetSizer(sizer);
    }

private:
    void OnButtonClick(wxCommandEvent& /*evt*/)
    {
        wxCustomization::StyledMessageDialog::Show(
            this, "StyledButton works!", "wxCustomization Demo",
            wxOK | wxICON_INFORMATION, m_styleSheet);
    }

    void OnToggle(wxCommandEvent& evt)
    {
        const bool value = evt.GetInt() != 0;
        wxCustomization::StyledMessageDialog::Show(
            this,
            wxString::Format("Toggle is now %s", value ? "ON" : "OFF"),
            "Demo Information",
            wxOK | wxICON_INFORMATION,
            m_styleSheet);
    }

    void OnCheckBox(wxCommandEvent& evt)
    {
        const int state = evt.GetInt();
        const wxString stateText =
            state == static_cast<int>(wxCustomization::CheckState::Checked)
                ? "CHECKED"
                : (state == static_cast<int>(wxCustomization::CheckState::Indeterminate)
                       ? "INDETERMINATE"
                       : "UNCHECKED");
        wxCustomization::StyledMessageDialog::Show(
            this,
            wxString::Format("CheckBox is now %s", stateText),
            "Demo Information",
            wxOK | wxICON_INFORMATION,
            m_styleSheet);
    }

    void OnRadio(wxCommandEvent& evt)
    {
        wxWindow* source = dynamic_cast<wxWindow*>(evt.GetEventObject());
        const wxString label = source ? source->GetLabel() : wxString("Unknown");
        wxCustomization::StyledMessageDialog::Show(
            this,
            wxString::Format("Selected: %s", label),
            "Demo Information",
            wxOK | wxICON_INFORMATION,
            m_styleSheet);
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
