#include "wxCustomization/widgets/StyledPanel.h"

#include <wx/sizer.h>

namespace wxCustomization {

StyledPanel::StyledPanel(wxWindow* parent,
                         wxWindowID id,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
{
}

wxSize StyledPanel::DoGetBestSize() const
{
    // A wxControl does not automatically use its sizer for best size,
    // but a panel-like container should.
    wxSize size(0, 0);
    if (wxSizer* sizer = GetSizer()) {
        size = sizer->CalcMin();
    } else {
        size = wxControl::DoGetBestSize();
    }

    const Style& style = GetCurrentStyle();

    const int horizontalExtras =
        style.borderLeftWidth + style.borderRightWidth +
        style.paddingLeft + style.paddingRight +
        style.marginLeft + style.marginRight;

    const int verticalExtras =
        style.borderTopWidth + style.borderBottomWidth +
        style.paddingTop + style.paddingBottom +
        style.marginTop + style.marginBottom;

    size.x += horizontalExtras;
    size.y += verticalExtras;

    if (style.IsSet(Property::MinWidth) && size.x < style.minWidth) {
        size.x = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && size.y < style.minHeight) {
        size.y = style.minHeight;
    }

    return size;
}

} // namespace wxCustomization
