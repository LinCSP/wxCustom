#pragma once

#include "wxCustomization/Color.h"
#include "wxCustomization/Length.h"
#include "wxCustomization/StyleTypes.h"

#include <wx/bitmap.h>
#include <wx/string.h>
#include <variant>

namespace wxCustomization {

/// A type-safe container for a single CSS/QSS property value.
class StyleValue {
public:
    enum class Type { None, Color, Length, Number, String, Enum, Image, Gradient };

    StyleValue() = default;

    static StyleValue FromColor(const wxColour& color);
    static StyleValue FromLength(const Length& length);
    static StyleValue FromNumber(double number);
    static StyleValue FromString(const wxString& string);
    static StyleValue FromEnum(const wxString& enumValue);
    static StyleValue FromImage(const wxBitmap& image);
    static StyleValue FromGradient(const Gradient& gradient);

    bool IsValid() const noexcept { return m_type != Type::None; }
    Type GetType() const noexcept { return m_type; }

    wxColour AsColor() const;
    int AsLength(const wxWindow* context, int emBase = 0, int percentBase = 0) const;
    double AsNumber() const;
    wxString AsString() const;
    wxString AsEnum() const;
    wxBitmap AsImage() const;
    Gradient AsGradient() const;

private:
    Type m_type = Type::None;
    std::variant<std::monostate, wxColour, Length, double, wxString, wxBitmap, Gradient> m_value;
};

} // namespace wxCustomization
