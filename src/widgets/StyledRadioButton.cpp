#include "wxCustomization/widgets/StyledRadioButton.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/dc.h>
#include <wx/radiobut.h>
#include <wx/settings.h>
#include <wx/window.h>

#include <algorithm>

namespace wxCustomization {

namespace {

wxColour MarkColor(const Style& style)
{
    return style.color.IsOk() ? style.color : wxColour(0, 0, 0);
}

} // namespace

StyledRadioButton::StyledRadioButton(wxWindow* parent,
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
    SetAccessibleRole(wxROLE_SYSTEM_RADIOBUTTON);
}

void StyledRadioButton::SetValue(bool value)
{
    if (value == m_checked) {
        return;
    }

    m_checked = value;

    if (value) {
        for (StyledRadioButton* sibling : GetGroupSiblings()) {
            if (sibling != this && sibling->GetValue()) {
                sibling->m_checked = false;
                sibling->ApplyStyle();
            }
        }
    }

    ApplyStyle();
}

void StyledRadioButton::SetLabel(const wxString& label)
{
    wxControl::SetLabel(label);
    m_label = label;
    ApplyStyle();
}

wxString StyledRadioButton::GetLabel() const
{
    return m_label;
}

void StyledRadioButton::DrawContent(wxDC& dc, const wxRect& rect)
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

void StyledRadioButton::Activate()
{
    if (m_checked) {
        return;
    }

    SetValue(true);

    wxCommandEvent event(wxEVT_RADIOBUTTON, GetId());
    event.SetEventObject(this);
    event.SetInt(1);
    ProcessWindowEvent(event);
}

void StyledRadioButton::OnChar(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();

    if (key == WXK_RETURN || key == WXK_SPACE || key == WXK_NUMPAD_ENTER) {
        Activate();
        return;
    }

    if (key == WXK_TAB) {
        Navigate(evt.ShiftDown() ? wxNavigationKeyEvent::IsBackward
                                 : wxNavigationKeyEvent::IsForward);
        return;
    }

    if (key == WXK_UP || key == WXK_LEFT) {
        MoveSelectionInGroup(-1);
        return;
    }
    if (key == WXK_DOWN || key == WXK_RIGHT) {
        MoveSelectionInGroup(1);
        return;
    }

    evt.Skip();
}

wxSize StyledRadioButton::DoGetBestSize() const
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

std::vector<StyledRadioButton*> StyledRadioButton::GetGroupSiblings() const
{
    std::vector<StyledRadioButton*> result;

    StyledRadioButton* self = const_cast<StyledRadioButton*>(this);
    wxWindow* parent = GetParent();
    if (!parent) {
        result.push_back(self);
        return result;
    }

    const wxWindowList& children = parent->GetChildren();
    auto it = std::find(children.begin(), children.end(), self);
    if (it == children.end()) {
        result.push_back(self);
        return result;
    }

    // Walk backward to find the start of the group. If this button itself
    // has wxRB_GROUP it is the first button of its group. Otherwise walk
    // back and include the nearest preceding button that has wxRB_GROUP.
    auto start = it;
    if (!(self->GetWindowStyle() & wxRB_GROUP)) {
        while (start != children.begin()) {
            auto prev = std::prev(start);
            StyledRadioButton* rb = wxDynamicCast(*prev, StyledRadioButton);
            if (!rb) {
                break;
            }
            if (rb->GetWindowStyle() & wxRB_GROUP) {
                start = prev; // include the group-start button
                break;
            }
            start = prev;
        }
    }

    // Walk forward to find the end of the group (before the next wxRB_GROUP).
    auto end = std::next(it);
    while (end != children.end()) {
        StyledRadioButton* rb = wxDynamicCast(*end, StyledRadioButton);
        if (!rb) {
            break;
        }
        if (rb->GetWindowStyle() & wxRB_GROUP) {
            break;
        }
        ++end;
    }

    for (auto i = start; i != end; ++i) {
        if (auto* rb = wxDynamicCast(*i, StyledRadioButton)) {
            result.push_back(rb);
        }
    }

    return result;
}

void StyledRadioButton::MoveSelectionInGroup(int delta)
{
    std::vector<StyledRadioButton*> group = GetGroupSiblings();
    if (group.size() <= 1) {
        return;
    }

    auto it = std::find(group.begin(), group.end(), this);
    if (it == group.end()) {
        return;
    }

    const int index = static_cast<int>(it - group.begin());
    const int count = static_cast<int>(group.size());

    int newIndex = index + delta;
    if (newIndex < 0) {
        newIndex = count - 1;
    } else if (newIndex >= count) {
        newIndex = 0;
    }

    StyledRadioButton* next = group[newIndex];
    next->SetFocus();

    if (!next->GetValue()) {
        next->SetValue(true);

        wxCommandEvent event(wxEVT_RADIOBUTTON, next->GetId());
        event.SetEventObject(next);
        event.SetInt(1);
        next->ProcessWindowEvent(event);
    }
}

wxSize StyledRadioButton::GetIndicatorSize(const Style& style) const
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

wxRect StyledRadioButton::GetIndicatorRect(const wxRect& contentRect,
                                           const wxSize& indicatorSize) const
{
    wxRect rect;
    rect.x = contentRect.x;
    rect.y = contentRect.y + (contentRect.height - indicatorSize.y) / 2;
    rect.width = indicatorSize.x;
    rect.height = indicatorSize.y;
    return rect;
}

wxRect StyledRadioButton::GetLabelRect(const wxRect& contentRect,
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

void StyledRadioButton::DrawIndicator(wxDC& dc, const wxRect& rect)
{
    Style indicatorStyle = GetSubControlStyle("indicator");

    Painter painter;
    painter.Paint(dc, rect, indicatorStyle, this);

    if (!m_checked) {
        return;
    }

    const wxColour color = MarkColor(indicatorStyle);

    const int margin = std::max(rect.width / 4, 2);
    wxRect dotRect = rect;
    dotRect.Deflate(margin);

    if (dotRect.width <= 0 || dotRect.height <= 0) {
        return;
    }

    dc.SetBrush(wxBrush(color));
    dc.SetPen(*wxTRANSPARENT_PEN);

    // Draw a filled circle clipped to the indicator area.
    const int cx = dotRect.x + dotRect.width / 2;
    const int cy = dotRect.y + dotRect.height / 2;
    const int radius = std::min(dotRect.width, dotRect.height) / 2;
    dc.DrawCircle(cx, cy, radius);
}

} // namespace wxCustomization
