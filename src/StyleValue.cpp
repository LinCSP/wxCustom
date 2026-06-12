#include "wxCustomization/StyleValue.h"

namespace wxCustomization {

StyleValue StyleValue::FromColor(const wxColour& color)
{
    StyleValue v;
    v.m_type = Type::Color;
    v.m_value = color;
    return v;
}

StyleValue StyleValue::FromLength(const Length& length)
{
    StyleValue v;
    v.m_type = Type::Length;
    v.m_value = length;
    return v;
}

StyleValue StyleValue::FromNumber(double number)
{
    StyleValue v;
    v.m_type = Type::Number;
    v.m_value = number;
    return v;
}

StyleValue StyleValue::FromString(const wxString& string)
{
    StyleValue v;
    v.m_type = Type::String;
    v.m_value = string;
    return v;
}

StyleValue StyleValue::FromEnum(const wxString& enumValue)
{
    StyleValue v;
    v.m_type = Type::Enum;
    v.m_value = enumValue;
    return v;
}

StyleValue StyleValue::FromImage(const wxBitmap& image)
{
    StyleValue v;
    v.m_type = Type::Image;
    v.m_value = image;
    return v;
}

StyleValue StyleValue::FromGradient(const Gradient& gradient)
{
    StyleValue v;
    v.m_type = Type::Gradient;
    v.m_value = gradient;
    return v;
}

wxColour StyleValue::AsColor() const
{
    if (m_type == Type::Color && std::holds_alternative<wxColour>(m_value)) {
        return std::get<wxColour>(m_value);
    }
    return wxNullColour;
}

int StyleValue::AsLength(const wxWindow* context, int emBase, int percentBase) const
{
    if (m_type == Type::Length && std::holds_alternative<Length>(m_value)) {
        return std::get<Length>(m_value).ToPixels(context, emBase, percentBase);
    }
    return 0;
}

double StyleValue::AsNumber() const
{
    if (m_type == Type::Number && std::holds_alternative<double>(m_value)) {
        return std::get<double>(m_value);
    }
    return 0.0;
}

wxString StyleValue::AsString() const
{
    if (m_type == Type::String && std::holds_alternative<wxString>(m_value)) {
        return std::get<wxString>(m_value);
    }
    return wxString();
}

wxString StyleValue::AsEnum() const
{
    if (m_type == Type::Enum && std::holds_alternative<wxString>(m_value)) {
        return std::get<wxString>(m_value);
    }
    return wxString();
}

wxBitmap StyleValue::AsImage() const
{
    if (m_type == Type::Image && std::holds_alternative<wxBitmap>(m_value)) {
        return std::get<wxBitmap>(m_value);
    }
    return wxBitmap();
}

Gradient StyleValue::AsGradient() const
{
    if (m_type == Type::Gradient && std::holds_alternative<Gradient>(m_value)) {
        return std::get<Gradient>(m_value);
    }
    return Gradient();
}

} // namespace wxCustomization
