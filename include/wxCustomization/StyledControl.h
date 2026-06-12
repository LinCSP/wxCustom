#pragma once

#include "wxCustomization/Style.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/control.h>
#include <wx/dcbuffer.h>
#include <wx/event.h>

#include <map>
#include <vector>

namespace wxCustomization {

/// Base class for all wxCustomization widgets.
/// Provides style resolution, state tracking, input events and rendering hooks.
class StyledControl : public wxControl {
public:
    StyledControl(wxWindow* parent, wxWindowID id,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0,
                  const wxString& name = wxControlNameStr);

    /// Set the stylesheet used to resolve this widget's appearance.
    void SetStyleSheet(StyleSheet* sheet);
    StyleSheet* GetStyleSheet() const;

    /// Resolve the style for the given transient state and request a repaint.
    void ApplyStyle(const wxString& state = wxEmptyString);
    /// Re-apply the current style. Override to add widget-specific logic.
    virtual void UpdateStyle();

    bool IsHovered() const noexcept { return m_hovered; }
    bool IsPressed() const noexcept { return m_pressed; }
    bool IsFocused() const noexcept { return m_focused; }
    bool IsChecked() const noexcept { return m_checked; }

    /// Dynamic properties used by attribute selectors `[name="value"]`.
    void SetStyleProperty(const wxString& name, const wxString& value);
    wxString GetStyleProperty(const wxString& name) const;

    /// CSS-style classes used by `.class` selectors.
    void AddStyleClass(const wxString& className);
    void RemoveStyleClass(const wxString& className);
    bool HasStyleClass(const wxString& className) const;
    std::vector<wxString> GetStyleClasses() const;

    /// Widget type used for type selectors (e.g. "StyledButton").
    virtual wxString GetStyledControlType() const;
    /// Widget ID used for `#id` selectors.
    virtual wxString GetStyledControlId() const;

    const Style& GetCurrentStyle() const { return m_currentStyle; }

protected:
    virtual void OnPaint(wxPaintEvent& evt);
    virtual void DrawBackground(wxDC& dc, const wxRect& rect, const Style& style);
    virtual void DrawBorder(wxDC& dc, const wxRect& rect, const Style& style);
    virtual void DrawFocusRect(wxDC& dc, const wxRect& rect, const Style& style);
    virtual wxRect GetContentRect() const;

    virtual void OnMouseEnter(wxMouseEvent& evt);
    virtual void OnMouseLeave(wxMouseEvent& evt);
    virtual void OnLeftDown(wxMouseEvent& evt);
    virtual void OnLeftUp(wxMouseEvent& evt);
    virtual void OnMotion(wxMouseEvent& evt);

    virtual void OnSetFocus(wxFocusEvent& evt);
    virtual void OnKillFocus(wxFocusEvent& evt);

    virtual void OnKeyDown(wxKeyEvent& evt);
    virtual void OnKeyUp(wxKeyEvent& evt);
    virtual void OnChar(wxKeyEvent& evt);

    virtual void OnNavigationKey(wxNavigationKeyEvent& evt);

    bool AcceptsFocus() const override;
    bool AcceptsFocusFromKeyboard() const override;

    /// Action triggered by Space/Enter or mouse click. Override in derived classes.
    virtual void Activate();

    wxSize DoGetBestSize() const override;

protected:
    StyleSheet* m_styleSheet = nullptr;
    Style m_currentStyle;

    bool m_hovered = false;
    bool m_pressed = false;
    bool m_focused = false;
    bool m_checked = false;

    std::vector<wxString> m_styleClasses;
    std::map<wxString, wxString> m_styleProperties;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
