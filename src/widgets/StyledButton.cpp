#include "wxCustomization/widgets/StyledButton.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/dc.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

StyledButton::StyledButton(wxWindow* parent, wxWindowID id,
                           const wxString& label,
                           const wxPoint& pos, const wxSize& size)
    : StyledControl(parent, id, pos, size)
{
    SetLabel(label);
}

void StyledButton::SetLabel(const wxString& label)
{
    wxControl::SetLabel(label);

    if (m_pressed) {
        ApplyStyle(wxString("pressed"));
    } else if (m_hovered) {
        ApplyStyle(wxString("hover"));
    } else {
        ApplyStyle(wxEmptyString);
    }
}

wxString StyledButton::GetLabel() const
{
    return wxControl::GetLabel();
}

void StyledButton::SetIcon(const wxBitmap& icon)
{
    m_icon = icon;
    ApplyStyle(m_hovered ? wxString("hover") : wxString(wxEmptyString));
}

wxBitmap StyledButton::GetIcon() const
{
    return m_icon;
}

void StyledButton::SetIconSize(const wxSize& size)
{
    m_iconSize = size;
    ApplyStyle(m_hovered ? wxString("hover") : wxString(wxEmptyString));
}

wxSize StyledButton::GetIconSize() const
{
    return m_iconSize;
}

void StyledButton::DrawContent(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();

    const wxBitmap icon = GetEffectiveIcon(style);
    const wxSize iconSize = GetEffectiveIconSize(style, icon);
    const wxString label = GetLabel();

    dc.SetFont(style.font.IsOk() ? style.font
                                 : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    dc.SetTextForeground(style.color.IsOk() ? style.color : *wxBLACK);

    int textWidth = 0;
    int textHeight = 0;
    if (!label.empty()) {
        dc.GetTextExtent(label, &textWidth, &textHeight);
    }

    const int spacing = icon.IsOk() && !label.empty() ? style.spacing : 0;
    const int totalWidth = iconSize.x + spacing + textWidth;
    const int totalHeight = std::max(iconSize.y, textHeight);

    int startX = rect.x;
    switch (style.textAlign) {
        case TextAlign::Left:
            startX = rect.x;
            break;
        case TextAlign::Center:
        case TextAlign::Justify:
            startX = rect.x + (rect.width - totalWidth) / 2;
            break;
        case TextAlign::Right:
            startX = rect.x + rect.width - totalWidth;
            break;
    }

    const int centerY = rect.y + rect.height / 2;

    const int iconX = startX;
    const int iconY = centerY - iconSize.y / 2;
    const int textX = startX + iconSize.x + spacing;
    const int textY = centerY - textHeight / 2;

    if (icon.IsOk()) {
        wxBitmap drawnIcon = icon;
        if (iconSize.x > 0 && iconSize.y > 0 &&
            (iconSize.x != icon.GetWidth() || iconSize.y != icon.GetHeight())) {
            drawnIcon = wxBitmap(icon.ConvertToImage().Rescale(iconSize.x, iconSize.y));
        }
        dc.DrawBitmap(drawnIcon, iconX, iconY, true);
    }

    if (!label.empty()) {
        dc.DrawText(label, textX, textY);
    }
}

void StyledButton::Activate()
{
    wxCommandEvent event(wxEVT_BUTTON, GetId());
    event.SetEventObject(this);
    ProcessWindowEvent(event);
}

wxSize StyledButton::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();

    const wxBitmap icon = GetEffectiveIcon(style);
    const wxSize iconSize = GetEffectiveIconSize(style, icon);
    const wxString label = GetLabel();

    int textWidth = 0;
    int textHeight = 0;
    if (!label.empty()) {
        const wxFont font = style.font.IsOk() ? style.font
                                              : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
        GetTextExtent(label, &textWidth, &textHeight, nullptr, nullptr, const_cast<wxFont*>(&font));
    }

    const int spacing = icon.IsOk() && !label.empty() ? style.spacing : 0;
    int width = iconSize.x + spacing + textWidth;
    int height = std::max(iconSize.y, textHeight);

    width += style.borderLeftWidth + style.borderRightWidth
             + style.paddingLeft + style.paddingRight;
    height += style.borderTopWidth + style.borderBottomWidth
              + style.paddingTop + style.paddingBottom;

    return wxSize(std::max(width, 32), std::max(height, 24));
}

wxBitmap StyledButton::GetEffectiveIcon(const Style& style) const
{
    if (style.icon.IsOk()) {
        return style.icon;
    }
    return m_icon;
}

wxSize StyledButton::GetEffectiveIconSize(const Style& style, const wxBitmap& icon) const
{
    if (style.iconSize.IsFullySpecified()) {
        return style.iconSize;
    }
    if (m_iconSize.IsFullySpecified()) {
        return m_iconSize;
    }
    if (icon.IsOk()) {
        return wxSize(icon.GetWidth(), icon.GetHeight());
    }
    return wxSize(0, 0);
}

} // namespace wxCustomization
