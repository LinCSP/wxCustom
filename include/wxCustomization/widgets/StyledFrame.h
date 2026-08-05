#pragma once

#include <wx/frame.h>

namespace wxCustomization {

class StyledPanel;
class StyledTitleBar;
class StyleSheet;

/// A top-level frame with client-side decorations: no native title bar or
/// window border. The window title bar is drawn by an embedded
/// StyledTitleBar (fully styled via QSS like every other widget), the
/// application content goes into the client panel below it.
///
/// The frame also implements window resizing from its edges and corners
/// (~6dip zones along the perimeter, with resize cursors): on GTK the drag
/// is handed to the compositor, elsewhere the geometry is adjusted manually.
/// SetMinSize() is honoured. Resizing is disabled while maximized.
///
/// Usage:
///
/// ```cpp
/// class MyFrame : public wxCustomization::StyledFrame { ... };
///
/// auto* frame = new MyFrame(...);
/// frame->SetStyleSheet(sheet);
/// // build the UI inside frame->GetClientPanel()
/// ```
class StyledFrame : public wxFrame {
public:
    /// Edge/corner identifiers used by the manual resize support.
    enum ResizeEdge {
        ResizeNone = 0,
        ResizeN,
        ResizeS,
        ResizeE,
        ResizeW,
        ResizeNE,
        ResizeNW,
        ResizeSE,
        ResizeSW
    };

    StyledFrame(wxWindow* parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxFrameNameStr);

    ~StyledFrame() override;

    /// The styled title bar at the top of the window.
    StyledTitleBar* GetTitleBar() const { return m_titleBar; }

    /// The panel for the application content (below the title bar).
    StyledPanel* GetClientPanel() const { return m_clientPanel; }

    /// Apply the stylesheet to the title bar and the client panel.
    void SetStyleSheet(StyleSheet* sheet);

    /// Updates both the window manager title and the styled title bar text.
    void SetTitle(const wxString& title) override;

    /// Hit-test for the resize zones along the window perimeter.
    /// @p pt is in frame client coordinates, @p size the frame client size,
    /// @p margin the zone thickness in pixels. Exposed for tests.
    static ResizeEdge HitTestResizeEdges(const wxPoint& pt, const wxSize& size,
                                         int margin);

private:
    class FrameResizer;

    StyledTitleBar* m_titleBar;
    StyledPanel* m_clientPanel;
    FrameResizer* m_resizer;
};

} // namespace wxCustomization
