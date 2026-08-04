#include "wxCustomization/widgets/StyledGroupBox.h"

#include "wxCustomization/Painter.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/sizer.h>

#include <algorithm>

namespace wxCustomization {

StyledGroupBox::StyledGroupBox(wxWindow* parent,
                               wxWindowID id,
                               const wxString& title,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_title(title)
{
    SetAccessibleRole(wxROLE_SYSTEM_GROUPING);
    SetAccessibleLabel(title);
}

void StyledGroupBox::SetTitle(const wxString& title)
{
    if (m_title == title) {
        return;
    }

    // Keep the accessible name in sync unless it was customized explicitly.
    if (GetAccessibleLabel() == m_title) {
        SetAccessibleLabel(title);
    }

    m_title = title;
    InvalidateBestSize();
    Refresh();
}

int StyledGroupBox::GetTitleHeight() const
{
    if (m_title.empty()) {
        return 0;
    }

    const wxFont font = GetTitleFont();
    int height = 0;
    GetTextExtent(m_title, nullptr, &height, nullptr, nullptr,
                  const_cast<wxFont*>(&font));
    return height;
}

void StyledGroupBox::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    DrawGroupBox(dc, GetClientRect());
}

void StyledGroupBox::DrawGroupBox(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();
    const Style titleStyle = GetSubControlStyle("title");

    const int titleHeight = GetTitleHeight();
    const int frameTop = titleHeight / 2;
    const wxColour behindColour = GetBehindColour();

    Painter painter;

    // Clear the strip above the frame: with wxBG_STYLE_PAINT nothing erases
    // it automatically, so stale pixels would remain there otherwise.
    if (frameTop > 0) {
        dc.SetBrush(behindColour);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rect.x, rect.y, rect.width, frameTop);
    }

    // Frame with background, border and outline, offset below the title.
    wxRect frameRect = rect;
    frameRect.y += frameTop;
    frameRect.height -= frameTop;
    painter.Paint(dc, frameRect, style, this);

    if (titleHeight <= 0) {
        return;
    }

    const wxRect titleRect = GetTitleRect(rect);

    // Cut the frame's top border out underneath the title by repainting the
    // title strip: the part above the frame with the colour behind the
    // widget, the part inside the frame with the group background.
    wxColour upperColour = behindColour;
    wxColour lowerColour = (style.IsSet(Property::BackgroundColor) && style.backgroundColor.IsOk())
                               ? style.backgroundColor
                               : behindColour;
    if (titleStyle.IsSet(Property::BackgroundColor) && titleStyle.backgroundColor.IsOk()) {
        upperColour = titleStyle.backgroundColor;
        lowerColour = titleStyle.backgroundColor;
    }

    int borderThickness = style.borderWidth;
    if (style.IsSet(Property::BorderTopWidth)) {
        borderThickness = style.borderTopWidth;
    }
    const int cutBottom = std::max(titleRect.y + titleRect.height,
                                   frameTop + borderThickness + 1);

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(upperColour);
    dc.DrawRectangle(titleRect.x, titleRect.y, titleRect.width, frameTop - titleRect.y);
    dc.SetBrush(lowerColour);
    dc.DrawRectangle(titleRect.x, frameTop, titleRect.width, cutBottom - frameTop);

    // Title text.
    dc.SetFont(GetTitleFont());
    const wxColour textColour = titleStyle.color.IsOk() ? titleStyle.color
        : (style.color.IsOk() ? style.color : wxColour(0, 0, 0));
    dc.SetTextForeground(textColour);

    int textWidth = 0;
    int textHeight = 0;
    dc.GetTextExtent(m_title, &textWidth, &textHeight);
    const int x = titleRect.x + titleStyle.paddingLeft;
    const int y = rect.y + (titleHeight - textHeight) / 2;
    dc.DrawText(m_title, x, y);
}

wxSize StyledGroupBox::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();

    wxSize size(0, 0);
    if (wxSizer* sizer = GetSizer()) {
        // Children are responsible for clearing the title strip; see the
        // class documentation. The style extras below account for the frame.
        size = sizer->CalcMin();
    } else if (!m_title.empty()) {
        const Style titleStyle = GetSubControlStyle("title");
        const wxFont font = GetTitleFont();
        int textWidth = 0;
        int textHeight = 0;
        GetTextExtent(m_title, &textWidth, &textHeight, nullptr, nullptr,
                      const_cast<wxFont*>(&font));
        size.x = textWidth + titleStyle.paddingLeft + titleStyle.paddingRight;
        size.y = textHeight;
    }

    size.x += style.borderLeftWidth + style.borderRightWidth
              + style.paddingLeft + style.paddingRight
              + style.marginLeft + style.marginRight;
    size.y += style.borderTopWidth + style.borderBottomWidth
              + style.paddingTop + style.paddingBottom
              + style.marginTop + style.marginBottom;

    if (style.IsSet(Property::MinWidth) && size.x < style.minWidth) {
        size.x = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && size.y < style.minHeight) {
        size.y = style.minHeight;
    }

    return size;
}

wxFont StyledGroupBox::GetTitleFont() const
{
    const Style titleStyle = GetSubControlStyle("title");
    if (titleStyle.font.IsOk()) {
        return titleStyle.font;
    }
    const Style& style = GetCurrentStyle();
    if (style.font.IsOk()) {
        return style.font;
    }
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

wxRect StyledGroupBox::GetTitleRect(const wxRect& rect) const
{
    const Style& style = GetCurrentStyle();
    const Style titleStyle = GetSubControlStyle("title");

    const wxFont font = GetTitleFont();
    int textWidth = 0;
    GetTextExtent(m_title, &textWidth, nullptr, nullptr, nullptr,
                  const_cast<wxFont*>(&font));

    const int x = rect.x + style.borderLeftWidth + style.paddingLeft;
    int width = textWidth + titleStyle.paddingLeft + titleStyle.paddingRight;
    width = std::min(width, rect.x + rect.width - x);

    return wxRect(x, rect.y, std::max(width, 0), GetTitleHeight());
}

wxColour StyledGroupBox::GetBehindColour() const
{
    // The strip above the frame belongs visually to the parent, so prefer the
    // parent's styled background when the parent is a StyledControl.
    if (const StyledControl* styledParent = dynamic_cast<const StyledControl*>(GetParent())) {
        const Style& parentStyle = styledParent->GetCurrentStyle();
        if (parentStyle.IsSet(Property::BackgroundColor) && parentStyle.backgroundColor.IsOk()) {
            return parentStyle.backgroundColor;
        }
    }
    if (const wxWindow* parent = GetParent()) {
        return parent->GetBackgroundColour();
    }
    return wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
}

} // namespace wxCustomization
