#include "wxCustomization/widgets/StyledFrame.h"

#include "wxCustomization/widgets/StyledPanel.h"
#include "wxCustomization/widgets/StyledTitleBar.h"

#include <wx/cursor.h>
#include <wx/eventfilter.h>
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

#ifdef __WXGTK__
GdkWindowEdge ToGdkEdge(StyledFrame::ResizeEdge edge)
{
    switch (edge) {
        case StyledFrame::ResizeN: return GDK_WINDOW_EDGE_NORTH;
        case StyledFrame::ResizeS: return GDK_WINDOW_EDGE_SOUTH;
        case StyledFrame::ResizeE: return GDK_WINDOW_EDGE_EAST;
        case StyledFrame::ResizeW: return GDK_WINDOW_EDGE_WEST;
        case StyledFrame::ResizeNE: return GDK_WINDOW_EDGE_NORTH_EAST;
        case StyledFrame::ResizeNW: return GDK_WINDOW_EDGE_NORTH_WEST;
        case StyledFrame::ResizeSE: return GDK_WINDOW_EDGE_SOUTH_EAST;
        case StyledFrame::ResizeSW: return GDK_WINDOW_EDGE_SOUTH_WEST;
        default: return GDK_WINDOW_EDGE_SOUTH_EAST;
    }
}
#endif

/// Stock cursor for a resize edge; wxCURSOR_ARROW means "not a resize zone".
wxStockCursor ToStockCursor(StyledFrame::ResizeEdge edge)
{
    switch (edge) {
        case StyledFrame::ResizeN:
        case StyledFrame::ResizeS:
            return wxCURSOR_SIZENS;
        case StyledFrame::ResizeE:
        case StyledFrame::ResizeW:
            return wxCURSOR_SIZEWE;
        case StyledFrame::ResizeNE:
        case StyledFrame::ResizeSW:
            return wxCURSOR_SIZENESW;
        case StyledFrame::ResizeNW:
        case StyledFrame::ResizeSE:
            return wxCURSOR_SIZENWSE;
        default:
            return wxCURSOR_ARROW;
    }
}

} // namespace

// ---------------------------------------------------------------------------
// StyledFrame::FrameResizer
// ---------------------------------------------------------------------------

/// Global event filter implementing window resize from the frame's edges
/// and corners. The filter is needed because mouse events do not propagate
/// from child windows to the frame.
class StyledFrame::FrameResizer : public wxEventFilter {
public:
    explicit FrameResizer(StyledFrame* frame)
        : m_frame(frame)
    {
        wxEvtHandler::AddFilter(this);
    }

    ~FrameResizer() override
    {
        wxEvtHandler::RemoveFilter(this);
    }

    int FilterEvent(wxEvent& event) override
    {
        const wxEventType type = event.GetEventType();
        if (type != wxEVT_MOTION && type != wxEVT_LEFT_DOWN && type != wxEVT_LEFT_UP
            && type != wxEVT_LEAVE_WINDOW) {
            return Event_Skip;
        }

        wxWindow* target = dynamic_cast<wxWindow*>(event.GetEventObject());
        if (target == nullptr || (target != m_frame && !m_frame->IsDescendant(target))) {
            return Event_Skip;
        }

        if (m_frame->IsMaximized()) {
            if (type == wxEVT_MOTION || type == wxEVT_LEAVE_WINDOW) {
                SetZoneCursor(target, ResizeNone);
            }
            return Event_Skip;
        }

        wxMouseEvent& me = static_cast<wxMouseEvent&>(event);

#ifndef __WXGTK__
        // Manual path: an active drag is handled before anything else.
        if (m_resizing != ResizeNone) {
            if (type == wxEVT_MOTION) {
                DragResize(me);
            } else if (type == wxEVT_LEFT_UP) {
                EndResize();
            }
            return Event_Processed;
        }
#endif

        if (type == wxEVT_LEFT_DOWN) {
            const ResizeEdge edge = CurrentEdge(target, me);
            if (edge != ResizeNone) {
                BeginResize(edge, target, me);
                // Consumed: the title bar must not start a move drag and the
                // children must not see the click.
                return Event_Processed;
            }
            return Event_Skip;
        }

        if (type == wxEVT_MOTION) {
            SetZoneCursor(target, CurrentEdge(target, me));
        } else if (type == wxEVT_LEAVE_WINDOW) {
            SetZoneCursor(target, ResizeNone);
        }
        return Event_Skip;
    }

private:
    ResizeEdge CurrentEdge(wxWindow* target, const wxMouseEvent& me) const
    {
        const wxPoint framePt = m_frame->ScreenToClient(target->ClientToScreen(me.GetPosition()));
        return HitTestResizeEdges(framePt, m_frame->GetClientSize(), m_frame->FromDIP(6));
    }

    void BeginResize(ResizeEdge edge, wxWindow* target, wxMouseEvent& me)
    {
#ifdef __WXGTK__
        // Hand the drag to the toolkit/compositor: works on both X11 and
        // Wayland and honours the window's size hints (SetMinSize).
        const wxPoint root = target->ClientToScreen(me.GetPosition());
        gtk_window_begin_resize_drag(GTK_WINDOW(m_frame->GetHandle()), ToGdkEdge(edge), 1,
                                     root.x, root.y, gtk_get_current_event_time());
#else
        m_resizing = edge;
        m_resizeStartRect = m_frame->GetRect();
        m_captureTarget = target;
        m_resizeStartMouse = target->ClientToScreen(me.GetPosition());
        target->CaptureMouse();
#endif
    }

#ifndef __WXGTK__
    void DragResize(const wxMouseEvent& me)
    {
        const wxPoint delta = m_captureTarget->ClientToScreen(me.GetPosition())
                              - m_resizeStartMouse;
        wxRect rect = m_resizeStartRect;

        if (m_resizing == ResizeE || m_resizing == ResizeNE || m_resizing == ResizeSE) {
            rect.width += delta.x;
        }
        if (m_resizing == ResizeW || m_resizing == ResizeNW || m_resizing == ResizeSW) {
            rect.x += delta.x;
            rect.width -= delta.x;
        }
        if (m_resizing == ResizeS || m_resizing == ResizeSE || m_resizing == ResizeSW) {
            rect.height += delta.y;
        }
        if (m_resizing == ResizeN || m_resizing == ResizeNE || m_resizing == ResizeNW) {
            rect.y += delta.y;
            rect.height -= delta.y;
        }

        wxSize minSize = m_frame->GetMinSize();
        if (minSize.x <= 0) {
            minSize.x = m_frame->FromDIP(120);
        }
        if (minSize.y <= 0) {
            minSize.y = m_frame->FromDIP(60);
        }
        if (rect.width < minSize.x) {
            if (m_resizing == ResizeW || m_resizing == ResizeNW || m_resizing == ResizeSW) {
                rect.x -= minSize.x - rect.width;
            }
            rect.width = minSize.x;
        }
        if (rect.height < minSize.y) {
            if (m_resizing == ResizeN || m_resizing == ResizeNE || m_resizing == ResizeNW) {
                rect.y -= minSize.y - rect.height;
            }
            rect.height = minSize.y;
        }

        m_frame->SetSize(rect);
    }

    void EndResize()
    {
        m_resizing = ResizeNone;
        if (m_captureTarget != nullptr && m_captureTarget->HasCapture()) {
            m_captureTarget->ReleaseMouse();
        }
        m_captureTarget = nullptr;
    }
#endif

    void SetZoneCursor(wxWindow* target, ResizeEdge edge)
    {
        if (edge == ResizeNone) {
            if (m_cursorWindow != nullptr) {
                m_cursorWindow->SetCursor(wxNullCursor);
                m_cursorWindow = nullptr;
            }
            m_cursorEdge = ResizeNone;
            return;
        }

        if (m_cursorWindow == target && m_cursorEdge == edge) {
            return;
        }
        if (m_cursorWindow != nullptr && m_cursorWindow != target) {
            m_cursorWindow->SetCursor(wxNullCursor);
        }
        target->SetCursor(wxCursor(ToStockCursor(edge)));
        m_cursorWindow = target;
        m_cursorEdge = edge;
    }

    StyledFrame* m_frame;
    wxWindow* m_cursorWindow = nullptr;
    ResizeEdge m_cursorEdge = ResizeNone;

#ifndef __WXGTK__
    ResizeEdge m_resizing = ResizeNone;
    wxRect m_resizeStartRect;
    wxPoint m_resizeStartMouse;
    wxWindow* m_captureTarget = nullptr;
#endif
};

// ---------------------------------------------------------------------------
// StyledFrame
// ---------------------------------------------------------------------------

StyledFrame::ResizeEdge StyledFrame::HitTestResizeEdges(const wxPoint& pt,
                                                        const wxSize& size,
                                                        int margin)
{
    const bool left = pt.x < margin;
    const bool right = pt.x >= size.x - margin;
    const bool top = pt.y < margin;
    const bool bottom = pt.y >= size.y - margin;

    if (left && top) {
        return ResizeNW;
    }
    if (right && top) {
        return ResizeNE;
    }
    if (left && bottom) {
        return ResizeSW;
    }
    if (right && bottom) {
        return ResizeSE;
    }
    if (left) {
        return ResizeW;
    }
    if (right) {
        return ResizeE;
    }
    if (top) {
        return ResizeN;
    }
    if (bottom) {
        return ResizeS;
    }
    return ResizeNone;
}

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

    m_resizer = new FrameResizer(this);

#ifdef __WXGTK__
    // Without wxRESIZE_BORDER wxGTK marks the window non-resizable and
    // gtk_window_maximize() silently does nothing. The window must stay
    // resizable for maximize (and for the resize handles), even though the
    // resize border itself is drawn by the library.
    if (GtkWidget* widget = GetHandle()) {
        gtk_window_set_resizable(GTK_WINDOW(widget), TRUE);
    }
#endif
}

StyledFrame::~StyledFrame()
{
    delete m_resizer;
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
