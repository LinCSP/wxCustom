#include "wxCustomization/Painter.h"

#include <wx/settings.h>
#include <wx/graphics.h>

#include <algorithm>
#include <cmath>
#include <memory>

namespace wxCustomization {

namespace {

wxPenStyle BorderStyleToPenStyle(BorderStyle style)
{
    switch (style) {
        case BorderStyle::Solid:
        case BorderStyle::Double:
        case BorderStyle::Groove:
        case BorderStyle::Ridge:
        case BorderStyle::Inset:
        case BorderStyle::Outset:
            return wxPENSTYLE_SOLID;
        case BorderStyle::Dashed:
            return wxPENSTYLE_SHORT_DASH;
        case BorderStyle::Dotted:
            return wxPENSTYLE_DOT;
        case BorderStyle::None:
        case BorderStyle::Hidden:
        default:
            return wxPENSTYLE_TRANSPARENT;
    }
}

int EffectiveBorderWidth(const Style& style)
{
    int width = style.borderWidth;
    if (style.IsSet(Property::BorderTopWidth)) width = style.borderTopWidth;
    else if (style.IsSet(Property::BorderLeftWidth)) width = style.borderLeftWidth;
    else if (style.IsSet(Property::BorderRightWidth)) width = style.borderRightWidth;
    else if (style.IsSet(Property::BorderBottomWidth)) width = style.borderBottomWidth;
    return width;
}

} // namespace

void Painter::Paint(wxDC& dc, const wxRect& rect, const Style& style,
                    const wxWindow* /*widget*/)
{
    DrawBackground(dc, rect, style);
    DrawBorder(dc, rect, style);
    DrawOutline(dc, rect, style);
}

void Painter::DrawBackground(wxDC& dc, const wxRect& rect, const Style& style)
{
    DrawBackgroundColour(dc, rect, style);
    DrawBackgroundGradient(dc, rect, style);
    DrawBackgroundImage(dc, rect, style);
}

void Painter::DrawBackgroundColour(wxDC& dc, const wxRect& rect, const Style& style)
{
    if (!style.IsSet(Property::BackgroundColor) || !style.backgroundColor.IsOk()) {
        return;
    }

    const wxColour colour = ApplyOpacity(style.backgroundColor, style.opacity);
    DrawRoundedRect(dc, rect, style.borderRadius,
                    wxBrush(colour), *wxTRANSPARENT_PEN);
}

void Painter::DrawBackgroundGradient(wxDC& dc, const wxRect& rect, const Style& style)
{
    if (!style.IsSet(Property::BackgroundGradient)) {
        return;
    }

    const Gradient& grad = style.backgroundGradient;
    if (grad.type == GradientType::Linear) {
        wxColour start = ApplyOpacity(grad.startColor, style.opacity);
        wxColour end = ApplyOpacity(grad.endColor, style.opacity);
        dc.GradientFillLinear(rect, start, end, grad.direction);
    }
}

void Painter::DrawBackgroundImage(wxDC& dc, const wxRect& rect, const Style& style)
{
    if (!style.IsSet(Property::BackgroundImage) || !style.backgroundImage.IsOk()) {
        return;
    }

    const wxBitmap& bmp = style.backgroundImage;
    const int w = bmp.GetWidth();
    const int h = bmp.GetHeight();
    if (w <= 0 || h <= 0) {
        return;
    }

    const BackgroundRepeat repeat = style.backgroundRepeat;
    const int startX = rect.x + style.backgroundPosition.x;
    const int startY = rect.y + style.backgroundPosition.y;

    auto drawOnce = [&](int x, int y) {
        dc.DrawBitmap(bmp, x, y, true);
    };

    if (repeat == BackgroundRepeat::NoRepeat) {
        drawOnce(startX, startY);
        return;
    }

    const int firstX = startX - ((startX - rect.x) / w + 1) * w;
    const int firstY = startY - ((startY - rect.y) / h + 1) * h;

    for (int y = firstY; y < rect.y + rect.height; y += h) {
        for (int x = firstX; x < rect.x + rect.width; x += w) {
            bool draw = true;
            if (repeat == BackgroundRepeat::RepeatX && (y != startY || h == 0)) {
                draw = false;
            }
            if (repeat == BackgroundRepeat::RepeatY && (x != startX || w == 0)) {
                draw = false;
            }
            if (draw) {
                drawOnce(x, y);
            }
        }
    }
}

void Painter::DrawBorder(wxDC& dc, const wxRect& rect, const Style& style)
{
    if (!style.IsSet(Property::BorderStyle) &&
        !style.IsSet(Property::BorderColor) &&
        !style.IsSet(Property::BorderWidth)) {
        return;
    }

    if (style.borderStyle == BorderStyle::None || style.borderStyle == BorderStyle::Hidden) {
        return;
    }

    const int width = EffectiveBorderWidth(style);
    if (width <= 0) {
        return;
    }

    const wxColour colour = ApplyOpacity(
        style.borderColor.IsOk() ? style.borderColor : wxColour(0, 0, 0),
        style.opacity);
    const wxPen pen(colour, width, BorderStyleToPenStyle(style.borderStyle));

    // Inset the rectangle so that the stroke stays fully inside the widget's
    // clipping region. Without this, a 1-pixel stroke centered on the boundary
    // can be clipped on the right/bottom edges, especially for widgets that
    // are stretched to the edge of their allocated area.
    wxRect borderRect = rect;
    const int inset = (width + 1) / 2;
    borderRect.Deflate(inset);
    if (borderRect.width <= 0 || borderRect.height <= 0) {
        return;
    }

    DrawRoundedRect(dc, borderRect, style.borderRadius,
                    *wxTRANSPARENT_BRUSH, pen);
}

void Painter::DrawOutline(wxDC& dc, const wxRect& rect, const Style& style)
{
    if (!style.IsSet(Property::OutlineWidth) || style.outlineWidth <= 0) {
        return;
    }

    const wxColour colour = ApplyOpacity(
        style.outlineColor.IsOk() ? style.outlineColor : wxColour(0, 120, 215),
        style.opacity);

    const int offset = style.outlineOffset + style.outlineWidth / 2;
    wxRect outlineRect = rect;
    outlineRect.Inflate(offset);

    const wxPen pen(colour, style.outlineWidth, wxPENSTYLE_SOLID);
    DrawRoundedRect(dc, outlineRect, style.borderRadius,
                    *wxTRANSPARENT_BRUSH, pen);
}

void Painter::DrawText(wxDC& dc, const wxRect& rect, const wxString& text,
                       const Style& style)
{
    if (text.empty()) {
        return;
    }

    wxRect content = rect;
    content.x += style.paddingLeft;
    content.y += style.paddingTop;
    content.width -= style.paddingLeft + style.paddingRight;
    content.height -= style.paddingTop + style.paddingBottom;
    if (content.width <= 0 || content.height <= 0) {
        return;
    }

    dc.SetFont(style.font.IsOk() ? style.font : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    dc.SetTextForeground(ApplyOpacity(
        style.color.IsOk() ? style.color : wxColour(0, 0, 0), style.opacity));

    int align = wxALIGN_CENTER_VERTICAL;
    switch (style.textAlign) {
        case TextAlign::Left:   align |= wxALIGN_LEFT; break;
        case TextAlign::Center: align |= wxALIGN_CENTER_HORIZONTAL; break;
        case TextAlign::Right:  align |= wxALIGN_RIGHT; break;
        case TextAlign::Justify: align |= wxALIGN_LEFT; break;
    }

    dc.DrawLabel(text, wxNullBitmap, content, align);
}

void Painter::DrawImage(wxDC& dc, const wxRect& rect, const wxBitmap& image,
                        const Style& style)
{
    if (!image.IsOk()) {
        return;
    }

    int w = image.GetWidth();
    int h = image.GetHeight();
    if (style.iconSize.IsFullySpecified() && style.iconSize.x > 0 && style.iconSize.y > 0) {
        w = style.iconSize.x;
        h = style.iconSize.y;
    }

    const int x = rect.x + (rect.width - w) / 2;
    const int y = rect.y + (rect.height - h) / 2;
    dc.DrawBitmap(image, x, y, true);
}

wxColour Painter::ApplyOpacity(const wxColour& colour, double opacity) const
{
    if (!colour.IsOk()) {
        return wxTransparentColour;
    }
    if (opacity >= 1.0 - 1e-9) {
        return colour;
    }
    const int alpha = std::clamp(static_cast<int>(std::round(colour.Alpha() * opacity)), 0, 255);
    return wxColour(colour.Red(), colour.Green(), colour.Blue(), alpha);
}

void Painter::DrawRoundedRect(wxDC& dc, const wxRect& rect, int radius,
                              const wxBrush& brush, const wxPen& pen)
{
    if (rect.width <= 0 || rect.height <= 0) {
        return;
    }

    int r = radius;
    if (r < 0) {
        // Percentage radius: interpret as a percentage of the smaller dimension.
        r = std::min(rect.width, rect.height) * (-r) / 100;
    }
    if (r > 0) {
        const int half = std::min(rect.width, rect.height) / 2;
        r = std::min(r, half);
    }

    // Use wxGraphicsContext when available so that rounded rectangles are
    // filled cleanly (including the corners) and opacity works correctly.
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::CreateFromUnknownDC(dc));
    if (gc) {
        gc->SetBrush(brush);
        gc->SetPen(pen);
        if (r > 0) {
            gc->DrawRoundedRectangle(rect.x, rect.y, rect.width, rect.height, r);
        } else {
            gc->DrawRectangle(rect.x, rect.y, rect.width, rect.height);
        }
        return;
    }

    dc.SetBrush(brush);
    dc.SetPen(pen);
    dc.DrawRoundedRectangle(rect, r);
}

} // namespace wxCustomization
