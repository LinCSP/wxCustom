#pragma once

#include "wxCustomization/Length.h"
#include "wxCustomization/StyleTypes.h"

#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <bitset>

namespace wxCustomization {

enum class Property {
    Color,
    BackgroundColor,
    BackgroundImage,
    BackgroundRepeat,
    BackgroundPosition,
    Opacity,
    BackgroundGradient,
    BorderWidth,
    BorderTopWidth,
    BorderRightWidth,
    BorderBottomWidth,
    BorderLeftWidth,
    BorderStyle,
    BorderColor,
    BorderRadius,
    OutlineWidth,
    OutlineColor,
    OutlineOffset,
    PaddingTop,
    PaddingRight,
    PaddingBottom,
    PaddingLeft,
    MarginTop,
    MarginRight,
    MarginBottom,
    MarginLeft,
    Font,
    TextAlign,
    TextDecoration,
    MinWidth,
    MaxWidth,
    MinHeight,
    MaxHeight,
    Width,
    Height,
    Icon,
    IconSize,
    Spacing,
    Count
};

/// Full set of resolved style properties for a widget in a single state.
struct Style {
    // Colours and background
    wxColour color;
    wxColour backgroundColor;
    wxBitmap backgroundImage;
    BackgroundRepeat backgroundRepeat = BackgroundRepeat::NoRepeat;
    wxPoint backgroundPosition;
    double opacity = 1.0;
    Gradient backgroundGradient;

    // Border
    int borderWidth = 0;
    int borderTopWidth = 0;
    int borderRightWidth = 0;
    int borderBottomWidth = 0;
    int borderLeftWidth = 0;
    BorderStyle borderStyle = BorderStyle::None;
    wxColour borderColor;
    int borderRadius = 0;

    // Focus outline
    int outlineWidth = 0;
    wxColour outlineColor;
    int outlineOffset = 0;

    // Padding and margin
    int paddingTop = 0;
    int paddingRight = 0;
    int paddingBottom = 0;
    int paddingLeft = 0;
    int marginTop = 0;
    int marginRight = 0;
    int marginBottom = 0;
    int marginLeft = 0;

    // Font
    wxFont font;
    TextAlign textAlign = TextAlign::Center;
    TextDecoration textDecoration = TextDecoration::None;

    // Size constraints
    int minWidth = -1;
    int maxWidth = -1;
    int minHeight = -1;
    int maxHeight = -1;
    int width = -1;
    int height = -1;

    // Content
    wxBitmap icon;
    wxSize iconSize;
    int spacing = 4;

    bool IsSet(Property prop) const;
    void Set(Property prop, bool value = true);

    /// Overwrite only properties that are set in @p other.
    void Merge(const Style& other);

private:
    std::bitset<static_cast<size_t>(Property::Count)> m_set;
};

} // namespace wxCustomization
