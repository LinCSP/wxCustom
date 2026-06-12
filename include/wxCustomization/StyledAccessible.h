#pragma once

#include <wx/defs.h>

#if wxUSE_ACCESSIBILITY

#include <wx/access.h>
#include <wx/string.h>
#include <wx/variant.h>

namespace wxCustomization {

class StyledControl;

/// Accessibility adapter for StyledControl, used by screen readers.
/// Only available when wxWidgets is built with wxUSE_ACCESSIBILITY=1.
class StyledAccessible : public wxAccessible {
public:
    explicit StyledAccessible(StyledControl* control);

    wxAccStatus GetName(int childId, wxString* name) override;
    wxAccStatus GetRole(int childId, wxVariant* role) override;
    wxAccStatus GetState(int childId, long* state) override;

private:
    StyledControl* m_control;
};

} // namespace wxCustomization

#endif // wxUSE_ACCESSIBILITY
