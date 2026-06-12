#include <wx/wx.h>
#include <wx/sizer.h>

#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/StyledControl.h"
#include "wxCustomization/wxCustomization.h"

namespace {

const char* gDemoStyle =
    ":root {\n"
    "  --primary: #3498db;\n"
    "  --hover: #2980b9;\n"
    "  --bg: #ecf0f1;\n"
    "}\n"
    "DemoWidget {\n"
    "  background-color: var(--bg);\n"
    "  border-width: 2dip;\n"
    "  border-color: #bdc3c7;\n"
    "  border-style: solid;\n"
    "  border-radius: 6dip;\n"
    "  color: #2c3e50;\n"
    "  padding: 8dip;\n"
    "  min-width: 80dip;\n"
    "  min-height: 32dip;\n"
    "}\n"
    "DemoWidget:hover {\n"
    "  background-color: var(--hover);\n"
    "  border-color: var(--primary);\n"
    "  color: #ffffff;\n"
    "}\n"
    "DemoWidget:focused {\n"
    "  outline-width: 3dip;\n"
    "  outline-color: var(--primary);\n"
    "  outline-offset: 2dip;\n"
    "}\n";

} // namespace

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
    DemoFrame()
        : wxFrame(nullptr, wxID_ANY, "wxCustomization Demo",
                  wxDefaultPosition, wxSize(800, 600))
    {
        m_styleSheet.LoadFromString(gDemoStyle);

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
            widget->SetStyleSheet(&m_styleSheet);
            row->Add(widget, 0, wxALL, 10);
        }
        sizer->Add(row, 1, wxALIGN_CENTER);

        panel->SetSizer(sizer);
    }

private:
    wxCustomization::StyleSheet m_styleSheet;
};

class DemoApp : public wxApp {
public:
    bool OnInit() override
    {
        DemoFrame* frame = new DemoFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(DemoApp);
