#include "wxCustomization/widgets/StyledCheckBox.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/checkbox.h>
#include <wx/dc.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

namespace {

wxColour MarkColor(const Style& style)
{
    return style.color.IsOk() ? style.color : wxColour(0, 0, 0);
}

} // namespace

StyledCheckBox::StyledCheckBox(wxWindow* parent,
                               wxWindowID id,
                               const wxString& label,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_label(label)
{
    SetLabel(label);
    SetAccessibleRole(wxROLE_SYSTEM_CHECKBUTTON);
}

void StyledCheckBox::SetValue(bool value)
{
    Set3StateValue(value ? CheckState::Checked : CheckState::Unchecked);
}

void StyledCheckBox::Set3StateValue(CheckState state)
{
    if (state == m_state) {
        return;
    }

    m_state = state;
    m_checked = (m_state == CheckState::Checked);

    ApplyStyle();
}

void StyledCheckBox::SetLabel(const wxString& label)
{
    wxControl::SetLabel(label);
    m_label = label;
    ApplyStyle();
}

wxString StyledCheckBox::GetLabel() const
{
    return m_label;
}

void StyledCheckBox::DrawContent(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();

    const wxSize indicatorSize = GetIndicatorSize(style);
    const wxRect indicatorRect = GetIndicatorRect(rect, indicatorSize);

    DrawIndicator(dc, indicatorRect);

    if (m_label.empty()) {
        return;
    }

    const wxRect labelRect = GetLabelRect(rect, indicatorRect);
    if (labelRect.width <= 0 || labelRect.height <= 0) {
        return;
    }

    dc.SetFont(style.font.IsOk() ? style.font
                                 : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    dc.SetTextForeground(style.color.IsOk() ? style.color : *wxBLACK);

    int textWidth = 0;
    int textHeight = 0;
    dc.GetTextExtent(m_label, &textWidth, &textHeight);

    int x = labelRect.x;
    switch (style.textAlign) {
        case TextAlign::Right:
            x = labelRect.x + labelRect.width - textWidth;
            break;
        case TextAlign::Center:
        case TextAlign::Justify:
            x = labelRect.x + (labelRect.width - textWidth) / 2;
            break;
        case TextAlign::Left:
        default:
            x = labelRect.x;
            break;
    }

    const int y = labelRect.y + (labelRect.height - textHeight) / 2;
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

void StyledCheckBox::Activate()
{
    CheckState newState;
    switch (m_state) {
        case CheckState::Checked:
            newState = CheckState::Unchecked;
            break;
        case CheckState::Indeterminate:
            newState = CheckState::Unchecked;
            break;
        case CheckState::Unchecked:
        default:
            newState = CheckState::Checked;
            break;
    }

    m_state = newState;
    m_checked = (m_state == CheckState::Checked);

    wxCommandEvent event(wxEVT_CHECKBOX, GetId());
    event.SetEventObject(this);
    event.SetInt(static_cast<int>(m_state));
    ProcessWindowEvent(event);

    ApplyStyle();
}

wxSize StyledCheckBox::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    const wxSize indicatorSize = GetIndicatorSize(style);

    int textWidth = 0;
    int textHeight = 0;
    if (!m_label.empty()) {
        GetTextExtent(m_label, &textWidth, &textHeight,
                      nullptr, nullptr, const_cast<wxFont*>(&font));
    }

    const int spacing = m_label.empty() ? 0 : style.spacing;

    int width = indicatorSize.x + spacing + textWidth;
    int height = std::max(indicatorSize.y, textHeight);

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

    return wxSize(std::max(width, indicatorSize.x), std::max(height, indicatorSize.y));
}

wxSize StyledCheckBox::GetIndicatorSize(const Style& style) const
{
    if (style.iconSize.IsFullySpecified() && style.iconSize.x > 0 && style.iconSize.y > 0) {
        return style.iconSize;
    }
    if (style.IsSet(Property::Width) && style.width > 0 &&
        style.IsSet(Property::Height) && style.height > 0) {
        return wxSize(style.width, style.height);
    }
    return FromDIP(wxSize(16, 16));
}

wxRect StyledCheckBox::GetIndicatorRect(const wxRect& contentRect,
                                        const wxSize& indicatorSize) const
{
    wxRect rect;
    rect.x = contentRect.x;
    rect.y = contentRect.y + (contentRect.height - indicatorSize.y) / 2;
    rect.width = indicatorSize.x;
    rect.height = indicatorSize.y;
    return rect;
}

wxRect StyledCheckBox::GetLabelRect(const wxRect& contentRect,
                                    const wxRect& indicatorRect) const
{
    const Style& style = GetCurrentStyle();
    const int spacing = m_label.empty() ? 0 : style.spacing;

    wxRect rect;
    rect.x = indicatorRect.x + indicatorRect.width + spacing;
    rect.y = contentRect.y;
    rect.width = contentRect.width - indicatorRect.width - spacing;
    rect.height = contentRect.height;

    if (rect.width < 0) {
        rect.width = 0;
    }
    if (rect.height < 0) {
        rect.height = 0;
    }
    return rect;
}

void StyledCheckBox::DrawIndicator(wxDC& dc, const wxRect& rect)
{
    Style indicatorStyle = GetSubControlStyle("indicator");

    Painter painter;
    painter.Paint(dc, rect, indicatorStyle, this);

    const wxColour color = MarkColor(indicatorStyle);

    if (m_state == CheckState::Unchecked) {
        return;
    }

    const int penWidth = std::max(FromDIP(2), 1);
    dc.SetPen(wxPen(color, penWidth, wxPENSTYLE_SOLID));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    if (m_state == CheckState::Indeterminate) {
        const int y = rect.y + rect.height / 2;
        const int inset = std::max(rect.width / 4, penWidth);
        dc.DrawLine(rect.x + inset, y, rect.x + rect.width - inset, y);
        return;
    }

    // Checked: draw a simple check mark.
    const int x = rect.x;
    const int y = rect.y;
    const int w = rect.width;
    const int h = rect.height;

    const wxPoint start(x + w * 2 / 10, y + h * 5 / 10);
    const wxPoint middle(x + w * 4 / 10, y + h * 7 / 10);
    const wxPoint end(x + w * 8 / 10, y + h * 3 / 10);

    dc.DrawLine(start, middle);
    dc.DrawLine(middle, end);
}

} // namespace wxCustomization
