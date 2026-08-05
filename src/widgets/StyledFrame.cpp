#include "wxCustomization/widgets/StyledFrame.h"

#include "wxCustomization/widgets/StyledPanel.h"
#include "wxCustomization/widgets/StyledTitleBar.h"

#include <wx/panel.h>
#include <wx/sizer.h>

#ifdef __WXGTK__
#include <gtk/gtk.h>
#endif

namespace wxCustomization {

namespace {

/// Strip every window-manager decoration flag: the frame gets no native
/// caption, border or system menu; everything is drawn by the library.
/// wxNO_BORDER is added so wxGTK marks the window as fully undecorated
/// (otherwise it still shows a plain border on X11).
long StyleWithoutDecorations(long style)
{
    return (style & ~(wxCAPTION | wxTINY_CAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU
                      | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX
                      | wxMAXIMIZE | wxMINIMIZE))
           | wxNO_BORDER;
}

} // namespace

StyledFrame::StyledFrame(wxWindow* parent,
                         wxWindowID id,
                         const wxString& title,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString& name)
    : wxFrame(parent, id, title, pos, size, StyleWithoutDecorations(style), name)
{
    // Plain root panel: it is fully covered by the title bar and the client
    // panel, so it needs no styling of its own.
    wxPanel* rootPanel = new wxPanel(this, wxID_ANY);

    m_titleBar = new StyledTitleBar(rootPanel, wxID_ANY);
    m_clientPanel = new StyledPanel(rootPanel, wxID_ANY);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_titleBar, 0, wxEXPAND);
    sizer->Add(m_clientPanel, 1, wxEXPAND);
    rootPanel->SetSizer(sizer);

    m_titleBar->SetTitle(title);

#ifdef __WXGTK__
    // Without wxRESIZE_BORDER wxGTK marks the window non-resizable and
    // gtk_window_maximize() silently does nothing. The window must stay
    // resizable for maximize (and for the manual resize handles), even
    // though the resize border itself is drawn by the library.
    if (GtkWidget* widget = GetHandle()) {
        gtk_window_set_resizable(GTK_WINDOW(widget), TRUE);
    }
#endif
}

void StyledFrame::SetStyleSheet(StyleSheet* sheet)
{
    m_titleBar->SetStyleSheet(sheet);
    m_clientPanel->SetStyleSheet(sheet);
}

void StyledFrame::SetTitle(const wxString& title)
{
    wxFrame::SetTitle(title);
    m_titleBar->SetTitle(title);
}

} // namespace wxCustomization
