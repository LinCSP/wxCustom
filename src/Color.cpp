#include "wxCustomization/Color.h"

#include <wx/gdicmn.h>

#include <cctype>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

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

std::vector<std::string> SplitByComma(const std::string& s)
{
    std::vector<std::string> parts;
    std::string current;
    for (char c : s) {
        if (c == ',') {
            parts.push_back(Trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(Trim(current));
    return parts;
}

unsigned char ClampByte(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<unsigned char>(value);
}

bool ParseDouble(const std::string& s, double& out)
{
    try {
        size_t pos = 0;
        out = std::stod(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

wxColour ParseHex(const std::string& hex)
{
    if (hex.empty()) {
        return wxNullColour;
    }

    std::string digits = hex;
    if (digits.front() == '#') {
        digits = digits.substr(1);
    }

    unsigned long value = 0;
    try {
        value = std::stoul(digits, nullptr, 16);
    } catch (...) {
        return wxNullColour;
    }

    if (digits.size() == 3) {
        // #rgb -> #rrggbb
        const unsigned char r = static_cast<unsigned char>(((value >> 8) & 0xF) * 17);
        const unsigned char g = static_cast<unsigned char>(((value >> 4) & 0xF) * 17);
        const unsigned char b = static_cast<unsigned char>((value & 0xF) * 17);
        return wxColour(r, g, b);
    }
    if (digits.size() == 6) {
        return wxColour(static_cast<unsigned char>((value >> 16) & 0xFF),
                        static_cast<unsigned char>((value >> 8) & 0xFF),
                        static_cast<unsigned char>(value & 0xFF));
    }
    if (digits.size() == 8) {
        return wxColour(static_cast<unsigned char>((value >> 16) & 0xFF),
                        static_cast<unsigned char>((value >> 8) & 0xFF),
                        static_cast<unsigned char>(value & 0xFF),
                        static_cast<unsigned char>((value >> 24) & 0xFF));
    }

    return wxNullColour;
}

bool ParseComponent(const std::string& s, int& out)
{
    if (s.empty()) {
        return false;
    }
    if (s.back() == '%') {
        double percent = 0.0;
        if (!ParseDouble(s.substr(0, s.size() - 1), percent)) {
            return false;
        }
        out = static_cast<int>(std::round(percent * 255.0 / 100.0));
        return true;
    }

    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

wxColour ParseRgb(const std::vector<std::string>& parts, bool hasAlpha)
{
    if (hasAlpha && parts.size() != 4) {
        return wxNullColour;
    }
    if (!hasAlpha && parts.size() != 3) {
        return wxNullColour;
    }

    int r = 0, g = 0, b = 0;
    if (!ParseComponent(parts[0], r) || !ParseComponent(parts[1], g) || !ParseComponent(parts[2], b)) {
        return wxNullColour;
    }

    unsigned char alpha = wxALPHA_OPAQUE;
    if (hasAlpha) {
        double a = 0.0;
        if (!ParseDouble(parts[3], a)) {
            return wxNullColour;
        }
        if (a <= 1.0) {
            alpha = ClampByte(static_cast<int>(std::round(a * 255.0)));
        } else {
            alpha = ClampByte(static_cast<int>(std::round(a)));
        }
    }

    return wxColour(ClampByte(r), ClampByte(g), ClampByte(b), alpha);
}

wxColour HsvToRgb(double h, double s, double v)
{
    h = std::fmod(h, 360.0);
    if (h < 0) {
        h += 360.0;
    }
    s = std::max(0.0, std::min(1.0, s));
    v = std::max(0.0, std::min(1.0, v));

    const double c = v * s;
    const double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    const double m = v - c;

    double r = 0.0, g = 0.0, b = 0.0;
    if (h < 60.0) {
        r = c; g = x; b = 0;
    } else if (h < 120.0) {
        r = x; g = c; b = 0;
    } else if (h < 180.0) {
        r = 0; g = c; b = x;
    } else if (h < 240.0) {
        r = 0; g = x; b = c;
    } else if (h < 300.0) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }

    return wxColour(ClampByte(static_cast<int>(std::round((r + m) * 255.0))),
                    ClampByte(static_cast<int>(std::round((g + m) * 255.0))),
                    ClampByte(static_cast<int>(std::round((b + m) * 255.0))));
}

wxColour ParseHsv(const std::vector<std::string>& parts, bool hasAlpha)
{
    if (hasAlpha && parts.size() != 4) {
        return wxNullColour;
    }
    if (!hasAlpha && parts.size() != 3) {
        return wxNullColour;
    }

    double h = 0.0, s = 0.0, v = 0.0;
    if (!ParseDouble(parts[0], h)) {
        return wxNullColour;
    }

    auto parsePercent = [](const std::string& str, double& out) {
        if (str.empty()) return false;
        if (str.back() == '%') {
            double percent = 0.0;
            if (!ParseDouble(str.substr(0, str.size() - 1), percent)) {
                return false;
            }
            out = percent / 100.0;
            return true;
        }
        return ParseDouble(str, out);
    };

    if (!parsePercent(parts[1], s) || !parsePercent(parts[2], v)) {
        return wxNullColour;
    }

    unsigned char alpha = wxALPHA_OPAQUE;
    if (hasAlpha) {
        double a = 0.0;
        if (!ParseDouble(parts[3], a)) {
            return wxNullColour;
        }
        if (a <= 1.0) {
            alpha = ClampByte(static_cast<int>(std::round(a * 255.0)));
        } else {
            alpha = ClampByte(static_cast<int>(std::round(a)));
        }
    }

    wxColour result = HsvToRgb(h, s, v);
    return wxColour(result.Red(), result.Green(), result.Blue(), alpha);
}

const std::map<std::string, wxColour>& NamedColors()
{
    static const std::map<std::string, wxColour> colors = {
        {"transparent", wxColour(0, 0, 0, 0)},
        {"black", wxColour(0, 0, 0)},
        {"white", wxColour(255, 255, 255)},
        {"red", wxColour(255, 0, 0)},
        {"green", wxColour(0, 128, 0)},
        {"blue", wxColour(0, 0, 255)},
        {"yellow", wxColour(255, 255, 0)},
        {"cyan", wxColour(0, 255, 255)},
        {"magenta", wxColour(255, 0, 255)},
        {"silver", wxColour(192, 192, 192)},
        {"gray", wxColour(128, 128, 128)},
        {"grey", wxColour(128, 128, 128)},
        {"maroon", wxColour(128, 0, 0)},
        {"olive", wxColour(128, 128, 0)},
        {"lime", wxColour(0, 255, 0)},
        {"aqua", wxColour(0, 255, 255)},
        {"teal", wxColour(0, 128, 128)},
        {"navy", wxColour(0, 0, 128)},
        {"fuchsia", wxColour(255, 0, 255)},
        {"purple", wxColour(128, 0, 128)},
        {"orange", wxColour(255, 165, 0)},
    };
    return colors;
}

} // namespace

wxColour Color::Parse(const wxString& str)
{
    wxColour result;
    if (TryParse(str, result)) {
        return result;
    }
    return wxNullColour;
}

bool Color::TryParse(const wxString& str, wxColour& out)
{
    out = wxNullColour;

    std::string input = Trim(ToLowerAscii(str.ToStdString()));
    if (input.empty()) {
        return false;
    }

    // Hex formats.
    if (input.front() == '#') {
        out = ParseHex(input);
        return out.IsOk();
    }

    // rgb() / rgba()
    if (input.compare(0, 4, "rgb(") == 0 && input.back() == ')') {
        std::string body = Trim(input.substr(4, input.size() - 5));
        out = ParseRgb(SplitByComma(body), false);
        return out.IsOk();
    }
    if (input.compare(0, 5, "rgba(") == 0 && input.back() == ')') {
        std::string body = Trim(input.substr(5, input.size() - 6));
        out = ParseRgb(SplitByComma(body), true);
        return out.IsOk();
    }

    // hsv() / hsva()
    if (input.compare(0, 4, "hsv(") == 0 && input.back() == ')') {
        std::string body = Trim(input.substr(4, input.size() - 5));
        out = ParseHsv(SplitByComma(body), false);
        return out.IsOk();
    }
    if (input.compare(0, 5, "hsva(") == 0 && input.back() == ')') {
        std::string body = Trim(input.substr(5, input.size() - 6));
        out = ParseHsv(SplitByComma(body), true);
        return out.IsOk();
    }

    // Named colours.
    const auto& named = NamedColors();
    auto it = named.find(input);
    if (it != named.end()) {
        out = it->second;
        return true;
    }

    return false;
}

} // namespace wxCustomization
