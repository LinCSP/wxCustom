#include "wxCustomization/widgets/StyledLabel.h"

#include <wx/dc.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

StyledLabel::StyledLabel(wxWindow* parent,
                         wxWindowID id,
                         const wxString& label,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_label(label)
{
}

void StyledLabel::SetLabel(const wxString& label)
{
    m_label = label;
    InvalidateBestSize();
    Refresh();
}

wxString StyledLabel::GetLabel() const
{
    return m_label;
}

void StyledLabel::DrawContent(wxDC& dc, const wxRect& rect)
{
    if (m_label.empty()) {
        return;
    }

    const Style& style = GetCurrentStyle();

    dc.SetFont(style.font.IsOk() ? style.font
                                 : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    dc.SetTextForeground(style.color.IsOk() ? style.color : *wxBLACK);

    int textWidth = 0;
    int textHeight = 0;
    dc.GetTextExtent(m_label, &textWidth, &textHeight);

    int x = rect.x;
    switch (style.textAlign) {
        case TextAlign::Left:
            x = rect.x;
            break;
        case TextAlign::Right:
            x = rect.x + rect.width - textWidth;
            break;
        case TextAlign::Center:
        case TextAlign::Justify:
        default:
            x = rect.x + (rect.width - textWidth) / 2;
            break;
    }

    const int y = rect.y + (rect.height - textHeight) / 2;

    dc.DrawText(m_label, x, y);

    switch (style.textDecoration) {
        case TextDecoration::Underline:
            dc.DrawLine(x, y + textHeight, x + textWidth, y + textHeight);
            break;
        case TextDecoration::Overline:
            dc.DrawLine(x, y, x + textWidth, y);
            break;
        case TextDecoration::LineThrough:
            dc.DrawLine(x, y + textHeight / 2, x + textWidth, y + textHeight / 2);
            break;
        case TextDecoration::None:
        default:
            break;
    }
}

wxSize StyledLabel::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    int textWidth = 0;
    int textHeight = 0;
    if (!m_label.empty()) {
        GetTextExtent(m_label, &textWidth, &textHeight,
                      nullptr, nullptr, const_cast<wxFont*>(&font));
    }

    int width = textWidth;
    int height = textHeight;

    width += style.borderLeftWidth + style.borderRightWidth
             + style.paddingLeft + style.paddingRight;
    height += style.borderTopWidth + style.borderBottomWidth
              + style.paddingTop + style.paddingBottom;

    if (style.IsSet(Property::MinWidth) && width < style.minWidth) {
        width = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && height < style.minHeight) {
        height = style.minHeight;
    }
    if (style.IsSet(Property::MaxWidth) && style.maxWidth >= 0 && width > style.maxWidth) {
        width = style.maxWidth;
    }
    if (style.IsSet(Property::MaxHeight) && style.maxHeight >= 0 && height > style.maxHeight) {
        height = style.maxHeight;
    }

    return wxSize(std::max(width, 0), std::max(height, 0));
}

} // namespace wxCustomization
