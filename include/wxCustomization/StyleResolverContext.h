#pragma once

#include <wx/string.h>
#include <wx/window.h>
#include <vector>

namespace wxCustomization {

/// Abstract interface used by StyleResolver to query widget properties.
class StyleResolverContext {
public:
    virtual ~StyleResolverContext() = default;

    /// The widget type name used for type selectors, e.g. "StyledButton".
    virtual wxString GetControlType() const = 0;

    /// The widget ID used for #id selectors (without the leading '#').
    virtual wxString GetControlId() const = 0;

    /// The list of CSS class names used for .class selectors (without the leading '.').
    virtual std::vector<wxString> GetControlClasses() const = 0;

    /// Returns true if the widget is in the given pseudo-state (e.g. "hover").
    virtual bool HasPseudoState(const wxString& state) const = 0;

    /// Returns the value of a dynamic property used in [name="value"] selectors.
    virtual wxString GetDynamicProperty(const wxString& name) const = 0;

    /// The underlying wxWindow for dip/pt/em conversion.
    virtual const wxWindow* GetWindow() const = 0;
};

} // namespace wxCustomization
