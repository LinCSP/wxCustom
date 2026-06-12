#include <wx/wx.h>
#include "wxCustomization/wxCustomization.h"

class DemoApp : public wxApp
{
public:
    bool OnInit() override
    {
        wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "wxCustomization Demo",
                                     wxDefaultPosition, wxSize(800, 600));

        wxPanel* panel = new wxPanel(frame, wxID_ANY);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* label = new wxStaticText(panel, wxID_ANY,
            wxString::Format("wxCustomization %s", wxCustomization::GetVersionString()));
        sizer->Add(label, 0, wxALL | wxCENTER, 20);

        panel->SetSizer(sizer);
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(DemoApp);
