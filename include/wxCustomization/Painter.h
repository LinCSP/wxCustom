#pragma once

#include "wxCustomization/Style.h"

#include <wx/dc.h>
#include <wx/window.h>

namespace wxCustomization {

/// Renders a resolved Style into a wxDC.
class Painter {
public:
    /// Draw the full decorative background of @p style into @p rect.
    void Paint(wxDC& dc, const wxRect& rect, const Style& style,
               const wxWindow* widget = nullptr);

    void DrawBackground(wxDC& dc, const wxRect& rect, const Style& style);
    void DrawBorder(wxDC& dc, const wxRect& rect, const Style& style);
    void DrawOutline(wxDC& dc, const wxRect& rect, const Style& style);

    void DrawText(wxDC& dc, const wxRect& rect, const wxString& text,
                  const Style& style);
    void DrawImage(wxDC& dc, const wxRect& rect, const wxBitmap& image,
                   const Style& style);

private:
    wxColour ApplyOpacity(const wxColour& colour, double opacity) const;
    void DrawBackgroundColour(wxDC& dc, const wxRect& rect, const Style& style);
    void DrawBackgroundGradient(wxDC& dc, const wxRect& rect, const Style& style);
    void DrawBackgroundImage(wxDC& dc, const wxRect& rect, const Style& style);
    void DrawRoundedRect(wxDC& dc, const wxRect& rect, int radius,
                         const wxBrush& brush, const wxPen& pen);
};

} // namespace wxCustomization
