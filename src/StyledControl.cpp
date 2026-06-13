#include "wxCustomization/StyledControl.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/StyledAccessible.h"
#include "wxCustomization/StyleResolver.h"
#include "wxCustomization/StyleResolverContext.h"

namespace wxCustomization {

namespace {

/// Adapts a StyledControl to the StyleResolverContext interface.
class StyledControlContext : public StyleResolverContext {
public:
    StyledControlContext(const StyledControl* control, const wxString& state)
        : m_control(control), m_state(state)
    {
    }

    wxString GetControlType() const override
    {
        return m_control->GetStyledControlType();
    }

    wxString GetControlId() const override
    {
        return m_control->GetStyledControlId();
    }

    std::vector<wxString> GetControlClasses() const override
    {
        return m_control->GetStyleClasses();
    }

    bool HasPseudoState(const wxString& s) const override
    {
        if (s == m_state) {
            return true;
        }
        if (s == "hover") {
            return m_control->IsHovered();
        }
        if (s == "pressed") {
            return m_control->IsPressed();
        }
        if (s == "focused") {
            return m_control->IsFocused();
        }
        if (s == "disabled") {
            return !m_control->IsEnabled();
        }
        if (s == "checked") {
            return m_control->IsChecked();
        }
        if (s == "unchecked") {
            return !m_control->IsChecked() && !m_control->IsIndeterminate();
        }
        if (s == "indeterminate") {
            return m_control->IsIndeterminate();
        }
        return false;
    }

    wxString GetDynamicProperty(const wxString& name) const override
    {
        return m_control->GetStyleProperty(name);
    }

    const wxWindow* GetWindow() const override
    {
        return m_control;
    }

private:
    const StyledControl* m_control;
    wxString m_state;
};

wxString CurrentState(const StyledControl* control)
{
    if (control->IsPressed()) {
        return "pressed";
    }
    if (control->IsHovered()) {
        return "hover";
    }
    return wxEmptyString;
}

} // namespace

wxBEGIN_EVENT_TABLE(StyledControl, wxControl)
    EVT_PAINT(StyledControl::OnPaint)
    EVT_ENTER_WINDOW(StyledControl::OnMouseEnter)
    EVT_LEAVE_WINDOW(StyledControl::OnMouseLeave)
    EVT_LEFT_DOWN(StyledControl::OnLeftDown)
    EVT_LEFT_UP(StyledControl::OnLeftUp)
    EVT_MOTION(StyledControl::OnMotion)
    EVT_SET_FOCUS(StyledControl::OnSetFocus)
    EVT_KILL_FOCUS(StyledControl::OnKillFocus)
    EVT_KEY_DOWN(StyledControl::OnKeyDown)
    EVT_KEY_UP(StyledControl::OnKeyUp)
    EVT_CHAR(StyledControl::OnChar)
    EVT_NAVIGATION_KEY(StyledControl::OnNavigationKey)
wxEND_EVENT_TABLE()

StyledControl::StyledControl(wxWindow* parent, wxWindowID id,
                             const wxPoint& pos, const wxSize& size,
                             long style, const wxString& name)
    : wxControl(parent, id, pos, size, style | wxBORDER_NONE, wxDefaultValidator, name)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
#if wxUSE_ACCESSIBILITY
    SetAccessible(new StyledAccessible(this));
#endif
}

void StyledControl::SetStyleSheet(StyleSheet* sheet)
{
    m_styleSheet = sheet;
    ApplyStyle();
}

StyleSheet* StyledControl::GetStyleSheet() const
{
    return m_styleSheet;
}

void StyledControl::ApplyStyle(const wxString& state)
{
    m_currentState = state;

    if (m_styleSheet != nullptr) {
        StyledControlContext context(this, state);
        StyleResolver resolver;
        m_currentStyle = resolver.Resolve(*m_styleSheet, context, wxEmptyString, state);
    } else {
        m_currentStyle = Style();
    }

    if (!IsBeingDeleted()) {
        InvalidateBestSize();
        Refresh();
    }
}

void StyledControl::UpdateStyle()
{
    ApplyStyle();
}

void StyledControl::SetStyleProperty(const wxString& name, const wxString& value)
{
    m_styleProperties[name] = value;
    ApplyStyle(CurrentState(this));
}

wxString StyledControl::GetStyleProperty(const wxString& name) const
{
    auto it = m_styleProperties.find(name);
    return it != m_styleProperties.end() ? it->second : wxString();
}

void StyledControl::AddStyleClass(const wxString& className)
{
    if (!HasStyleClass(className)) {
        m_styleClasses.push_back(className);
        ApplyStyle(CurrentState(this));
    }
}

void StyledControl::RemoveStyleClass(const wxString& className)
{
    auto it = std::find(m_styleClasses.begin(), m_styleClasses.end(), className);
    if (it != m_styleClasses.end()) {
        m_styleClasses.erase(it);
        ApplyStyle(CurrentState(this));
    }
}

bool StyledControl::HasStyleClass(const wxString& className) const
{
    return std::find(m_styleClasses.begin(), m_styleClasses.end(), className) != m_styleClasses.end();
}

std::vector<wxString> StyledControl::GetStyleClasses() const
{
    return m_styleClasses;
}

wxString StyledControl::GetStyledControlType() const
{
    return "StyledControl";
}

wxString StyledControl::GetStyledControlId() const
{
    return GetName();
}

void StyledControl::SetAccessibleLabel(const wxString& label)
{
    m_accessibleLabel = label;
}

wxString StyledControl::GetAccessibleLabel() const
{
    return m_accessibleLabel;
}

void StyledControl::SetAccessibleRole(wxAccRole role)
{
    m_accessibleRole = role;
}

wxAccRole StyledControl::GetAccessibleRole() const
{
    return m_accessibleRole;
}

void StyledControl::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    const wxRect rect = GetClientRect();

    Painter painter;
    painter.Paint(dc, rect, m_currentStyle, this);
    DrawContent(dc, GetContentRect());
}

void StyledControl::DrawContent(wxDC& /*dc*/, const wxRect& /*rect*/)
{
    // Default implementation draws no content. Derived widgets override this
    // to render text, icons and sub-controls.
}

void StyledControl::DrawBackground(wxDC& dc, const wxRect& rect, const Style& style)
{
    Painter painter;
    painter.DrawBackground(dc, rect, style);
}

void StyledControl::DrawBorder(wxDC& dc, const wxRect& rect, const Style& style)
{
    Painter painter;
    painter.DrawBorder(dc, rect, style);
}

void StyledControl::DrawFocusRect(wxDC& dc, const wxRect& rect, const Style& style)
{
    Painter painter;
    painter.DrawOutline(dc, rect, style);
}

wxRect StyledControl::GetContentRect() const
{
    wxRect rect = GetClientRect();
    rect.x += m_currentStyle.borderLeftWidth + m_currentStyle.paddingLeft;
    rect.y += m_currentStyle.borderTopWidth + m_currentStyle.paddingTop;
    rect.width -= m_currentStyle.borderLeftWidth + m_currentStyle.borderRightWidth
                  + m_currentStyle.paddingLeft + m_currentStyle.paddingRight;
    rect.height -= m_currentStyle.borderTopWidth + m_currentStyle.borderBottomWidth
                   + m_currentStyle.paddingTop + m_currentStyle.paddingBottom;

    if (rect.width < 0) {
        rect.width = 0;
    }
    if (rect.height < 0) {
        rect.height = 0;
    }
    return rect;
}

void StyledControl::OnMouseEnter(wxMouseEvent& /*evt*/)
{
    m_hovered = true;
    ApplyStyle(m_pressed ? wxString("pressed") : wxString("hover"));
}

void StyledControl::OnMouseLeave(wxMouseEvent& /*evt*/)
{
    m_hovered = false;
    m_pressed = false;
    ApplyStyle(wxEmptyString);
}

void StyledControl::OnLeftDown(wxMouseEvent& /*evt*/)
{
    m_pressed = true;
    SetFocus();
    ApplyStyle(wxString("pressed"));
}

void StyledControl::OnLeftUp(wxMouseEvent& /*evt*/)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    if (wasPressed && m_hovered) {
        Activate();
    }
    ApplyStyle(m_hovered ? wxString("hover") : wxString(wxEmptyString));
}

void StyledControl::OnMotion(wxMouseEvent& evt)
{
    evt.Skip();
}

void StyledControl::OnSetFocus(wxFocusEvent& evt)
{
    m_focused = true;
    ApplyStyle(CurrentState(this));
    evt.Skip();
}

void StyledControl::OnKillFocus(wxFocusEvent& evt)
{
    m_focused = false;
    ApplyStyle(CurrentState(this));
    evt.Skip();
}

void StyledControl::OnKeyDown(wxKeyEvent& evt)
{
    evt.Skip();
}

void StyledControl::OnKeyUp(wxKeyEvent& evt)
{
    evt.Skip();
}

void StyledControl::OnChar(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();
    if (key == WXK_RETURN || key == WXK_SPACE || key == WXK_NUMPAD_ENTER) {
        Activate();
    } else if (key == WXK_TAB) {
        Navigate(evt.ShiftDown() ? wxNavigationKeyEvent::IsBackward
                                 : wxNavigationKeyEvent::IsForward);
    } else {
        evt.Skip();
    }
}

void StyledControl::OnNavigationKey(wxNavigationKeyEvent& evt)
{
    evt.Skip();
}

bool StyledControl::AcceptsFocus() const
{
    return wxControl::AcceptsFocus() && IsEnabled() && IsShown();
}

bool StyledControl::AcceptsFocusFromKeyboard() const
{
    return AcceptsFocus();
}

void StyledControl::DoEnable(bool enable)
{
    wxControl::DoEnable(enable);
    ApplyStyle(m_pressed ? wxString("pressed") : (m_hovered ? wxString("hover") : wxString()));
}

void StyledControl::Activate()
{
    // Default implementation does nothing. Derived widgets override this
    // to emit command events or toggle state.
}

Style StyledControl::GetSubControlStyle(const wxString& subControl) const
{
    if (m_styleSheet == nullptr) {
        return Style();
    }

    StyledControlContext context(this, m_currentState);
    StyleResolver resolver;
    return resolver.Resolve(*m_styleSheet, context, subControl, m_currentState);
}

wxSize StyledControl::DoGetBestSize() const
{
    return wxSize(0, 0);
}

} // namespace wxCustomization
