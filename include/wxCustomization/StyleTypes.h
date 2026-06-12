#pragma once

#include <wx/colour.h>
#include <wx/gdicmn.h>

namespace wxCustomization {

enum class BackgroundRepeat {
    Repeat,
    RepeatX,
    RepeatY,
    NoRepeat,
    Space,
    Round
};

enum class BorderStyle {
    None,
    Hidden,
    Dotted,
    Dashed,
    Solid,
    Double,
    Groove,
    Ridge,
    Inset,
    Outset
};

enum class TextAlign {
    Left,
    Right,
    Center,
    Justify
};

enum class TextDecoration {
    None,
    Underline,
    Overline,
    LineThrough
};

enum class GradientType {
    Linear,
    Radial
};

struct Gradient {
    GradientType type = GradientType::Linear;
    wxDirection direction = wxEAST;
    wxColour startColor = wxColour(0, 0, 0);
    wxColour endColor = wxColour(255, 255, 255);

    bool operator==(const Gradient& other) const noexcept
    {
        return type == other.type &&
               direction == other.direction &&
               startColor == other.startColor &&
               endColor == other.endColor;
    }

    bool operator!=(const Gradient& other) const noexcept
    {
        return !(*this == other);
    }
};

} // namespace wxCustomization
