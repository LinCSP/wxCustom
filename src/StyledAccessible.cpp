#include "wxCustomization/StyledAccessible.h"

#if wxUSE_ACCESSIBILITY

#include "wxCustomization/StyledControl.h"

namespace wxCustomization {

StyledAccessible::StyledAccessible(StyledControl* control)
    : m_control(control)
{
}

wxAccStatus StyledAccessible::GetName(int childId, wxString* name)
{
    if (childId != wxACC_SELF) {
        return wxACC_NOT_SUPPORTED;
    }

    *name = m_control->GetAccessibleLabel();
    if (name->IsEmpty()) {
        *name = m_control->GetLabel();
    }
    return wxACC_OK;
}

wxAccStatus StyledAccessible::GetRole(int childId, wxVariant* role)
{
    if (childId != wxACC_SELF) {
        return wxACC_NOT_SUPPORTED;
    }

    *role = static_cast<int>(m_control->GetAccessibleRole());
    return wxACC_OK;
}

wxAccStatus StyledAccessible::GetState(int childId, long* state)
{
    if (childId != wxACC_SELF) {
        return wxACC_NOT_SUPPORTED;
    }

    *state = 0;
    if (!m_control->IsEnabled()) {
        *state |= wxACC_STATE_SYSTEM_UNAVAILABLE;
    }
    if (!m_control->IsShown()) {
        *state |= wxACC_STATE_SYSTEM_INVISIBLE;
    }
    if (m_control->HasFocus()) {
        *state |= wxACC_STATE_SYSTEM_FOCUSED;
    }
    if (m_control->AcceptsFocusFromKeyboard()) {
        *state |= wxACC_STATE_SYSTEM_FOCUSABLE;
    }
    return wxACC_OK;
}

} // namespace wxCustomization

#endif // wxUSE_ACCESSIBILITY
