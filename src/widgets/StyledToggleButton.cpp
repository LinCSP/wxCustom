#include "wxCustomization/widgets/StyledToggleButton.h"

#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/tglbtn.h>

#include <algorithm>

namespace wxCustomization {

StyledToggleButton::StyledToggleButton(wxWindow* parent,
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

void StyledToggleButton::SetValue(bool value)
{
    if (value == IsChecked()) {
        return;
    }

    m_checked = value;
    ApplyStyle();
}

void StyledToggleButton::DrawContent(wxDC& dc, const wxRect& rect)
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

    const int x = rect.x + (rect.width - textWidth) / 2;
    const int y = rect.y + (rect.height - textHeight) / 2;

    dc.DrawText(m_label, x, y);
}

void StyledToggleButton::Activate()
{
    m_checked = !m_checked;

    wxCommandEvent event(wxEVT_TOGGLEBUTTON, GetId());
    event.SetEventObject(this);
    event.SetInt(m_checked ? 1 : 0);
    ProcessWindowEvent(event);

    ApplyStyle();
}

wxSize StyledToggleButton::DoGetBestSize() const
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

    return wxSize(std::max(width, 32), std::max(height, 24));
}

} // namespace wxCustomization
