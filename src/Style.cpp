#include "wxCustomization/Style.h"

namespace wxCustomization {

namespace {

size_t PropIndex(Property prop)
{
    return static_cast<size_t>(prop);
}

} // namespace

bool Style::IsSet(Property prop) const
{
    return m_set.test(PropIndex(prop));
}

void Style::Set(Property prop, bool value)
{
    m_set.set(PropIndex(prop), value);
}

void Style::Merge(const Style& other)
{
    const auto& bits = other.m_set;
    if (bits.none()) {
        return;
    }

    for (size_t i = 0; i < static_cast<size_t>(Property::Count); ++i) {
        if (!bits.test(i)) {
            continue;
        }

        const auto prop = static_cast<Property>(i);
        switch (prop) {
            case Property::Color: color = other.color; break;
            case Property::BackgroundColor: backgroundColor = other.backgroundColor; break;
            case Property::BackgroundImage: backgroundImage = other.backgroundImage; break;
            case Property::BackgroundRepeat: backgroundRepeat = other.backgroundRepeat; break;
            case Property::BackgroundPosition: backgroundPosition = other.backgroundPosition; break;
            case Property::Opacity: opacity = other.opacity; break;
            case Property::BackgroundGradient: backgroundGradient = other.backgroundGradient; break;
            case Property::BorderWidth: borderWidth = other.borderWidth; break;
            case Property::BorderTopWidth: borderTopWidth = other.borderTopWidth; break;
            case Property::BorderRightWidth: borderRightWidth = other.borderRightWidth; break;
            case Property::BorderBottomWidth: borderBottomWidth = other.borderBottomWidth; break;
            case Property::BorderLeftWidth: borderLeftWidth = other.borderLeftWidth; break;
            case Property::BorderStyle: borderStyle = other.borderStyle; break;
            case Property::BorderColor: borderColor = other.borderColor; break;
            case Property::BorderRadius: borderRadius = other.borderRadius; break;
            case Property::OutlineWidth: outlineWidth = other.outlineWidth; break;
            case Property::OutlineColor: outlineColor = other.outlineColor; break;
            case Property::OutlineOffset: outlineOffset = other.outlineOffset; break;
            case Property::PaddingTop: paddingTop = other.paddingTop; break;
            case Property::PaddingRight: paddingRight = other.paddingRight; break;
            case Property::PaddingBottom: paddingBottom = other.paddingBottom; break;
            case Property::PaddingLeft: paddingLeft = other.paddingLeft; break;
            case Property::MarginTop: marginTop = other.marginTop; break;
            case Property::MarginRight: marginRight = other.marginRight; break;
            case Property::MarginBottom: marginBottom = other.marginBottom; break;
            case Property::MarginLeft: marginLeft = other.marginLeft; break;
            case Property::Font: font = other.font; break;
            case Property::TextAlign: textAlign = other.textAlign; break;
            case Property::MinWidth: minWidth = other.minWidth; break;
            case Property::MaxWidth: maxWidth = other.maxWidth; break;
            case Property::MinHeight: minHeight = other.minHeight; break;
            case Property::MaxHeight: maxHeight = other.maxHeight; break;
            case Property::Width: width = other.width; break;
            case Property::Height: height = other.height; break;
            case Property::Icon: icon = other.icon; break;
            case Property::IconSize: iconSize = other.iconSize; break;
            case Property::Spacing: spacing = other.spacing; break;
            case Property::Count: break;
        }

        m_set.set(i);
    }
}

} // namespace wxCustomization
