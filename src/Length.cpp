#include "wxCustomization/Length.h"

#include <cctype>
#include <cmath>
#include <map>
#include <string>

namespace wxCustomization {

namespace {

std::string Trim(const std::string& s)
{
    size_t first = 0;
    while (first < s.size() && std::isspace(static_cast<unsigned char>(s[first]))) {
        ++first;
    }
    size_t last = s.size();
    while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1]))) {
        --last;
    }
    return s.substr(first, last - first);
}

std::string ToLowerAscii(const std::string& s)
{
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

bool IsNumberChar(char c)
{
    return std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '+';
}

} // namespace

Length::Length(double value, Unit unit)
    : m_value(value)
    , m_unit(unit)
    , m_valid(true)
{
}

int Length::ToPixels(const wxWindow* context, int emBase, int percentBase) const
{
    if (!m_valid) {
        return 0;
    }

    switch (m_unit) {
        case Px:
            return static_cast<int>(std::round(m_value));

        case Dip:
        case Dp:
            if (context) {
                return context->FromDIP(static_cast<int>(std::round(m_value)));
            }
            return static_cast<int>(std::round(m_value));

        case Pt: {
            const int ppi = context ? context->GetDPI().GetHeight() : 96;
            return static_cast<int>(std::round(m_value * ppi / 72.0));
        }

        case Em:
            return static_cast<int>(std::round(m_value * emBase));

        case Percent:
            return static_cast<int>(std::round(m_value * percentBase / 100.0));
    }

    return 0;
}

Length Length::Parse(const wxString& str)
{
    Length result;
    TryParse(str, result);
    return result;
}

bool Length::TryParse(const wxString& str, Length& out)
{
    out = Length();

    std::string input = Trim(ToLowerAscii(str.ToStdString()));
    if (input.empty()) {
        return false;
    }

    // Percent: "50%"
    if (input.back() == '%') {
        std::string num = Trim(input.substr(0, input.size() - 1));
        if (num.empty()) {
            return false;
        }
        try {
            out.m_value = std::stod(num);
        } catch (...) {
            return false;
        }
        out.m_unit = Percent;
        out.m_valid = true;
        return true;
    }

    // Split numeric part and unit.
    size_t numEnd = 0;
    while (numEnd < input.size() && IsNumberChar(input[numEnd])) {
        ++numEnd;
    }

    std::string num = Trim(input.substr(0, numEnd));
    std::string unitStr = Trim(input.substr(numEnd));

    if (num.empty()) {
        return false;
    }

    try {
        out.m_value = std::stod(num);
    } catch (...) {
        return false;
    }

    if (unitStr == "px" || unitStr.empty()) {
        out.m_unit = Px;
    } else if (unitStr == "dip") {
        out.m_unit = Dip;
    } else if (unitStr == "dp") {
        out.m_unit = Dp;
    } else if (unitStr == "pt") {
        out.m_unit = Pt;
    } else if (unitStr == "em") {
        out.m_unit = Em;
    } else {
        return false;
    }

    out.m_valid = true;
    return true;
}

bool Length::operator==(const Length& other) const noexcept
{
    return m_valid == other.m_valid && m_value == other.m_value && m_unit == other.m_unit;
}

bool Length::operator!=(const Length& other) const noexcept
{
    return !(*this == other);
}

} // namespace wxCustomization
