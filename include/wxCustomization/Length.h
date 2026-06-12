#pragma once

#include <wx/string.h>
#include <wx/window.h>

namespace wxCustomization {

class Length {
public:
    enum Unit { Px, Dip, Dp, Pt, Em, Percent };

    Length() = default;
    Length(double value, Unit unit);

    bool IsValid() const noexcept { return m_valid; }
    Unit GetUnit() const noexcept { return m_unit; }
    double GetValue() const noexcept { return m_value; }

    /// Convert the length to physical pixels.
    /// @param context     wxWindow used for dip/dp/pt conversion (must not be nullptr for those units).
    /// @param emBase      Base size for em units.
    /// @param percentBase Base value for percent units.
    int ToPixels(const wxWindow* context, int emBase = 0, int percentBase = 0) const;

    static Length Parse(const wxString& str);
    static bool TryParse(const wxString& str, Length& out);

    bool operator==(const Length& other) const noexcept;
    bool operator!=(const Length& other) const noexcept;

private:
    double m_value = 0.0;
    Unit m_unit = Px;
    bool m_valid = false;
};

} // namespace wxCustomization
