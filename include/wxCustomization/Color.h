#pragma once

#include <wx/colour.h>
#include <wx/string.h>

namespace wxCustomization {

/// Parses CSS/QSS-like colour values into wxColour.
class Color {
public:
    /// Parse a colour string. Returns wxNullColour on failure.
    static wxColour Parse(const wxString& str);

    /// Parse a colour string; returns true on success and writes the result to @p out.
    static bool TryParse(const wxString& str, wxColour& out);
};

} // namespace wxCustomization
