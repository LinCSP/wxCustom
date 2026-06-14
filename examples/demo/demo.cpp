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
#include "wxCustomization/widgets/StyledLineEdit.h"
#include "wxCustomization/widgets/StyledComboBox.h"
#include "wxCustomization/wxCustomization.h"

namespace {

/// A small helper label used as section header in the demo.
wxCustomization::StyledLabel* CreateHeader(wxWindow* parent,
                                           wxCustomization::StyleSheet* sheet,
                                           const wxString& text)
{
    wxCustomization::StyledLabel* label =
        new wxCustomization::StyledLabel(parent, wxID_ANY, text);
    label->SetStyleSheet(sheet);
    label->AddStyleClass("section-header");
    return label;
}

} // namespace

class DemoFrame : public wxFrame {
public:
    DemoFrame(wxCustomization::StyleSheet* styleSheet)
        : wxFrame(nullptr, wxID_ANY, "wxCustomization Demo",
                  wxDefaultPosition, wxSize(900, 700))
        , m_styleSheet(styleSheet)
    {
        wxCustomization::StyledPanel* panel =
            new wxCustomization::StyledPanel(this, wxID_ANY);
        panel->SetStyleSheet(m_styleSheet);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxCustomization::StyledLabel* title = new wxCustomization::StyledLabel(
            panel, wxID_ANY,
            wxString::Format("wxCustomization %s", wxCustomization::GetVersionString()));
        title->SetStyleSheet(m_styleSheet);
        title->AddStyleClass("title");
        sizer->Add(title, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 20);

        sizer->Add(CreateHeader(panel, m_styleSheet, "StyledButton"), 0,
                   wxLEFT | wxRIGHT | wxTOP, 16);

        wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
        m_button = new wxCustomization::StyledButton(panel, wxID_ANY, "Click me");
        m_button->SetStyleSheet(m_styleSheet);
        m_button->Bind(wxEVT_BUTTON, &DemoFrame::OnButtonClick, this);
        buttonRow->Add(m_button, 0, wxALL, 8);

        wxCustomization::StyledButton* disabledButton =
            new wxCustomization::StyledButton(panel, wxID_ANY, "Disabled");
        disabledButton->SetStyleSheet(m_styleSheet);
        disabledButton->Disable();
        buttonRow->Add(disabledButton, 0, wxALL, 8);

        sizer->Add(buttonRow, 0, wxLEFT | wxRIGHT, 8);

        sizer->Add(CreateHeader(panel, m_styleSheet, "StyledToggleButton"), 0,
                   wxLEFT | wxRIGHT | wxTOP, 16);

        wxBoxSizer* toggleRow = new wxBoxSizer(wxHORIZONTAL);
        m_toggle = new wxCustomization::StyledToggleButton(panel, wxID_ANY, "Toggle me");
        m_toggle->SetStyleSheet(m_styleSheet);
        m_toggle->Bind(wxEVT_TOGGLEBUTTON, &DemoFrame::OnToggle, this);
        toggleRow->Add(m_toggle, 0, wxALL, 8);

        wxCustomization::StyledToggleButton* disabledToggle =
            new wxCustomization::StyledToggleButton(panel, wxID_ANY, "Disabled ON");
        disabledToggle->SetValue(true);
        disabledToggle->SetStyleSheet(m_styleSheet);
        disabledToggle->Disable();
        toggleRow->Add(disabledToggle, 0, wxALL, 8);

        sizer->Add(toggleRow, 0, wxLEFT | wxRIGHT, 8);

        sizer->Add(CreateHeader(panel, m_styleSheet, "StyledCheckBox"), 0,
                   wxLEFT | wxRIGHT | wxTOP, 16);

        wxBoxSizer* checkRow = new wxBoxSizer(wxHORIZONTAL);
        m_checkBox = new wxCustomization::StyledCheckBox(panel, wxID_ANY, "Check me");
        m_checkBox->SetStyleSheet(m_styleSheet);
        m_checkBox->Bind(wxEVT_CHECKBOX, &DemoFrame::OnCheckBox, this);
        checkRow->Add(m_checkBox, 0, wxALL, 8);

        wxCustomization::StyledCheckBox* disabledCheck =
            new wxCustomization::StyledCheckBox(panel, wxID_ANY, "Disabled checked");
        disabledCheck->SetValue(true);
        disabledCheck->SetStyleSheet(m_styleSheet);
        disabledCheck->Disable();
        checkRow->Add(disabledCheck, 0, wxALL, 8);

        m_indeterminateCheck =
            new wxCustomization::StyledCheckBox(panel, wxID_ANY, "Indeterminate");
        m_indeterminateCheck->Set3StateValue(wxCustomization::CheckState::Indeterminate);
        m_indeterminateCheck->SetStyleSheet(m_styleSheet);
        checkRow->Add(m_indeterminateCheck, 0, wxALL, 8);

        sizer->Add(checkRow, 0, wxLEFT | wxRIGHT, 8);

        sizer->Add(CreateHeader(panel, m_styleSheet, "StyledRadioButton"), 0,
                   wxLEFT | wxRIGHT | wxTOP, 16);

        wxBoxSizer* radioRow = new wxBoxSizer(wxHORIZONTAL);
        m_radio1 = new wxCustomization::StyledRadioButton(panel, wxID_ANY, "Radio 1",
                                                          wxDefaultPosition, wxDefaultSize,
                                                          wxRB_GROUP);
        m_radio1->SetStyleSheet(m_styleSheet);
        m_radio1->Bind(wxEVT_RADIOBUTTON, &DemoFrame::OnRadio, this);
        radioRow->Add(m_radio1, 0, wxALL, 8);

        m_radio2 = new wxCustomization::StyledRadioButton(panel, wxID_ANY, "Radio 2");
        m_radio2->SetStyleSheet(m_styleSheet);
        m_radio2->Bind(wxEVT_RADIOBUTTON, &DemoFrame::OnRadio, this);
        radioRow->Add(m_radio2, 0, wxALL, 8);

        wxCustomization::StyledRadioButton* disabledRadio =
            new wxCustomization::StyledRadioButton(panel, wxID_ANY, "Disabled selected");
        disabledRadio->SetValue(true);
        disabledRadio->SetStyleSheet(m_styleSheet);
        disabledRadio->Disable();
        radioRow->Add(disabledRadio, 0, wxALL, 8);

        sizer->Add(radioRow, 0, wxLEFT | wxRIGHT, 8);

        sizer->Add(CreateHeader(panel, m_styleSheet, "StyledLineEdit"), 0,
                   wxLEFT | wxRIGHT | wxTOP, 16);

        wxBoxSizer* editRow = new wxBoxSizer(wxHORIZONTAL);
        m_lineEdit = new wxCustomization::StyledLineEdit(panel, wxID_ANY, "Type here...");
        m_lineEdit->SetStyleSheet(m_styleSheet);
        m_lineEdit->Bind(wxEVT_TEXT, &DemoFrame::OnLineEditText, this);
        m_lineEdit->Bind(wxEVT_TEXT_ENTER, &DemoFrame::OnLineEditEnter, this);
        editRow->Add(m_lineEdit, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);

        m_lineEditValue = new wxCustomization::StyledLabel(panel, wxID_ANY, "Value: Type here...");
        m_lineEditValue->SetStyleSheet(m_styleSheet);
        editRow->Add(m_lineEditValue, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);

        sizer->Add(editRow, 0, wxLEFT | wxRIGHT | wxEXPAND, 8);

        sizer->Add(CreateHeader(panel, m_styleSheet, "StyledComboBox"), 0,
                   wxLEFT | wxRIGHT | wxTOP, 16);

        wxBoxSizer* comboRow = new wxBoxSizer(wxHORIZONTAL);
        wxArrayString choices;
        choices.Add("Option 1");
        choices.Add("Option 2");
        choices.Add("Option 3");
        m_comboBox = new wxCustomization::StyledComboBox(panel, wxID_ANY, choices);
        m_comboBox->SetStyleSheet(m_styleSheet);
        m_comboBox->SetSelection(0);
        m_comboBox->Bind(wxEVT_COMBOBOX, &DemoFrame::OnComboBox, this);
        comboRow->Add(m_comboBox, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);

        m_comboBoxValue = new wxCustomization::StyledLabel(panel, wxID_ANY, "Selected: Option 1");
        m_comboBoxValue->SetStyleSheet(m_styleSheet);
        comboRow->Add(m_comboBoxValue, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);

        sizer->Add(comboRow, 0, wxLEFT | wxRIGHT | wxEXPAND, 8);

        m_disableButton =
            new wxCustomization::StyledButton(panel, wxID_ANY, "Disable all widgets");
        m_disableButton->SetStyleSheet(m_styleSheet);
        m_disableButton->Bind(wxEVT_BUTTON, &DemoFrame::OnToggleDisabled, this);
        sizer->Add(m_disableButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 24);

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

    void OnLineEditText(wxCommandEvent& evt)
    {
        const wxString value = evt.GetString();
        m_lineEditValue->SetLabel(wxString::Format("Value: %s", value));
        Layout();
    }

    void OnLineEditEnter(wxCommandEvent& evt)
    {
        wxCustomization::StyledMessageDialog::Show(
            this,
            wxString::Format("Entered: %s", evt.GetString()),
            "Demo Information",
            wxOK | wxICON_INFORMATION,
            m_styleSheet);
    }

    void OnComboBox(wxCommandEvent& evt)
    {
        const wxString value = evt.GetString();
        m_comboBoxValue->SetLabel(wxString::Format("Selected: %s", value));
        Layout();
    }

    void OnToggleDisabled(wxCommandEvent& /*evt*/)
    {
        m_allDisabled = !m_allDisabled;

        m_button->Enable(!m_allDisabled);
        m_toggle->Enable(!m_allDisabled);
        m_checkBox->Enable(!m_allDisabled);
        m_indeterminateCheck->Enable(!m_allDisabled);
        m_radio1->Enable(!m_allDisabled);
        m_radio2->Enable(!m_allDisabled);
        m_lineEdit->Enable(!m_allDisabled);
        m_comboBox->Enable(!m_allDisabled);

        m_disableButton->SetLabel(m_allDisabled ? "Enable all widgets" : "Disable all widgets");
        Layout();
    }

    wxCustomization::StyleSheet* m_styleSheet;

    wxCustomization::StyledButton* m_button = nullptr;
    wxCustomization::StyledToggleButton* m_toggle = nullptr;
    wxCustomization::StyledCheckBox* m_checkBox = nullptr;
    wxCustomization::StyledCheckBox* m_indeterminateCheck = nullptr;
    wxCustomization::StyledLineEdit* m_lineEdit = nullptr;
    wxCustomization::StyledLabel* m_lineEditValue = nullptr;
    wxCustomization::StyledComboBox* m_comboBox = nullptr;
    wxCustomization::StyledLabel* m_comboBoxValue = nullptr;
    wxCustomization::StyledRadioButton* m_radio1 = nullptr;
    wxCustomization::StyledRadioButton* m_radio2 = nullptr;
    wxCustomization::StyledButton* m_disableButton = nullptr;

    bool m_allDisabled = false;
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
