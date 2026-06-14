#include "wxCustomization/StyleResolver.h"

#include "wxCustomization/Color.h"
#include "wxCustomization/Length.h"

#include <wx/bitmap.h>
#include <wx/filename.h>
#include <wx/image.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>
#include <sstream>

namespace wxCustomization {

namespace {

wxString ToLowerAscii(const wxString& s)
{
    wxString result = s;
    result.MakeLower();
    return result;
}

std::vector<wxString> SplitWhitespace(const wxString& s)
{
    std::vector<wxString> parts;
    wxString current;
    for (wxChar c : s) {
        if (std::isspace(static_cast<int>(c))) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

int LengthToPixels(const wxString& value, const wxWindow* context)
{
    return Length::Parse(value).ToPixels(context);
}

int PixelsToPoints(int pixels, const wxWindow* context)
{
    const int ppi = context ? context->GetDPI().GetHeight() : 96;
    if (ppi == 0) {
        return pixels;
    }
    return static_cast<int>(std::round(pixels * 72.0 / ppi));
}

wxBitmap LoadBitmapFromUrl(const wxString& value)
{
    wxString path = value;
    path.Trim(true).Trim(false);

    if (path.Lower().StartsWith("url(") && path.EndsWith(")")) {
        path = path.Mid(4, path.length() - 5);
    }

    if ((path.StartsWith("\"") && path.EndsWith("\"")) ||
        (path.StartsWith("'") && path.EndsWith("'"))) {
        path = path.Mid(1, path.length() - 2);
    }

    path.Trim(true).Trim(false);

    if (path.empty()) {
        return wxBitmap();
    }

    wxImage image;
    if (image.LoadFile(path)) {
        return wxBitmap(image);
    }

    return wxBitmap();
}

} // namespace

Style StyleResolver::Resolve(const StyleSheet& sheet,
                             const StyleResolverContext& context,
                             const wxString& subControl,
                             const wxString& state) const
{
    Style style;

    auto matches = CollectMatches(sheet, context, subControl, state);
    std::stable_sort(matches.begin(), matches.end(),
                     [](const Match& a, const Match& b) {
                         return a.specificity < b.specificity;
                     });

    for (const auto& match : matches) {
        const StyleRule& rule = *match.rule;
        for (const auto& decl : rule.declarations) {
            ApplyDeclaration(style, sheet, decl.property, decl.value, context.GetWindow());
        }
    }

    return style;
}

std::vector<StyleResolver::Match> StyleResolver::CollectMatches(
    const StyleSheet& sheet,
    const StyleResolverContext& context,
    const wxString& subControl,
    const wxString& state) const
{
    std::vector<Match> result;
    for (const auto& rule : sheet.GetRules()) {
        for (size_t i = 0; i < rule.selectors.size(); ++i) {
            if (Matches(rule.selectors[i], context, subControl, state)) {
                Match m;
                m.rule = &rule;
                m.selectorIndex = i;
                m.specificity = CalculateSpecificity(rule.selectors[i]);
                result.push_back(m);
            }
        }
    }
    return result;
}

bool StyleResolver::Matches(const SelectorSequence& seq,
                            const StyleResolverContext& context,
                            const wxString& subControl,
                            const wxString& state) const
{
    // For now combinators are ignored: every simple selector in the
    // sequence must match the current context. This is sufficient for
    // simple selector chains used in the project themes.
    for (const auto& part : seq.parts) {
        if (!MatchesSimpleSelector(part, context, subControl, state)) {
            return false;
        }
    }
    return true;
}

bool StyleResolver::MatchesSimpleSelector(const SimpleSelector& sel,
                                          const StyleResolverContext& context,
                                          const wxString& subControl,
                                          const wxString& state) const
{
    // Sub-control matching is strict: when resolving a sub-control, only
    // selectors that explicitly target that sub-control match, and vice versa.
    if (!subControl.empty()) {
        if (sel.subControl != subControl) {
            return false;
        }
    } else if (!sel.subControl.empty()) {
        return false;
    }

    if (!sel.type.empty() && sel.type != "*" && sel.type != context.GetControlType()) {
        return false;
    }

    if (!sel.id.empty() && sel.id != context.GetControlId()) {
        return false;
    }

    if (!sel.className.empty() && !Contains(context.GetControlClasses(), sel.className)) {
        return false;
    }

    if (!sel.pseudo.empty() && sel.pseudo != state && !context.HasPseudoState(sel.pseudo)) {
        return false;
    }

    if (!sel.attrName.empty() && sel.attrValue != context.GetDynamicProperty(sel.attrName)) {
        return false;
    }

    return true;
}

int StyleResolver::CalculateSpecificity(const SelectorSequence& seq) const
{
    int ids = 0;
    int classes = 0;
    int types = 0;

    for (const auto& part : seq.parts) {
        if (!part.id.empty()) {
            ++ids;
        }
        if (!part.className.empty() || !part.pseudo.empty() || !part.attrName.empty()) {
            ++classes;
        }
        if (!part.type.empty() && part.type != "*") {
            ++types;
        }
        if (!part.subControl.empty()) {
            ++types;
        }
    }

    return ids * 100 + classes * 10 + types;
}

wxString StyleResolver::ResolveVariables(const wxString& value, const StyleSheet& sheet) const
{
    std::string input = value.ToStdString();
    const std::regex varRegex(R"(\bvar\s*\(\s*(--[A-Za-z0-9-]+)\s*(?:,\s*([^)]*))?\s*\))");

    std::smatch match;
    while (std::regex_search(input, match, varRegex)) {
        const wxString name = wxString::FromUTF8(match[1].str());
        wxString replacement = sheet.GetVariable(name);
        if (replacement.empty() && match[2].matched) {
            replacement = wxString::FromUTF8(match[2].str());
        }
        input = match.prefix().str() + replacement.ToStdString() + match.suffix().str();
    }

    return wxString::FromUTF8(input);
}

void StyleResolver::ApplyDeclaration(Style& style,
                                     const StyleSheet& sheet,
                                     const wxString& property,
                                     const wxString& value,
                                     const wxWindow* context) const
{
    const wxString prop = ToLowerAscii(property);
    const wxString resolved = ResolveVariables(value, sheet);

    if (prop == "color") {
        style.color = Color::Parse(resolved);
        style.Set(Property::Color);
    } else if (prop == "background-color") {
        style.backgroundColor = Color::Parse(resolved);
        style.Set(Property::BackgroundColor);
    } else if (prop == "opacity") {
        try {
            style.opacity = std::stod(resolved.ToStdString());
        } catch (...) {
            style.opacity = 1.0;
        }
        style.Set(Property::Opacity);
    } else if (prop == "border-width") {
        const int w = LengthToPixels(resolved, context);
        style.borderWidth = w;
        style.borderTopWidth = w;
        style.borderRightWidth = w;
        style.borderBottomWidth = w;
        style.borderLeftWidth = w;
        style.Set(Property::BorderWidth);
        style.Set(Property::BorderTopWidth);
        style.Set(Property::BorderRightWidth);
        style.Set(Property::BorderBottomWidth);
        style.Set(Property::BorderLeftWidth);
    } else if (prop == "border-top-width") {
        style.borderTopWidth = LengthToPixels(resolved, context);
        style.Set(Property::BorderTopWidth);
    } else if (prop == "border-right-width") {
        style.borderRightWidth = LengthToPixels(resolved, context);
        style.Set(Property::BorderRightWidth);
    } else if (prop == "border-bottom-width") {
        style.borderBottomWidth = LengthToPixels(resolved, context);
        style.Set(Property::BorderBottomWidth);
    } else if (prop == "border-left-width") {
        style.borderLeftWidth = LengthToPixels(resolved, context);
        style.Set(Property::BorderLeftWidth);
    } else if (prop == "border-color") {
        style.borderColor = Color::Parse(resolved);
        style.Set(Property::BorderColor);
    } else if (prop == "border-style") {
        const wxString s = ToLowerAscii(resolved);
        if (s == "solid") style.borderStyle = BorderStyle::Solid;
        else if (s == "dashed") style.borderStyle = BorderStyle::Dashed;
        else if (s == "dotted") style.borderStyle = BorderStyle::Dotted;
        else if (s == "double") style.borderStyle = BorderStyle::Double;
        else if (s == "none") style.borderStyle = BorderStyle::None;
        else if (s == "hidden") style.borderStyle = BorderStyle::Hidden;
        else if (s == "groove") style.borderStyle = BorderStyle::Groove;
        else if (s == "ridge") style.borderStyle = BorderStyle::Ridge;
        else if (s == "inset") style.borderStyle = BorderStyle::Inset;
        else if (s == "outset") style.borderStyle = BorderStyle::Outset;
        style.Set(Property::BorderStyle);
    } else if (prop == "border-radius") {
        style.borderRadius = LengthToPixels(resolved, context);
        style.Set(Property::BorderRadius);
    } else if (prop == "outline-width") {
        style.outlineWidth = LengthToPixels(resolved, context);
        style.Set(Property::OutlineWidth);
    } else if (prop == "outline-color") {
        style.outlineColor = Color::Parse(resolved);
        style.Set(Property::OutlineColor);
    } else if (prop == "outline-offset") {
        style.outlineOffset = LengthToPixels(resolved, context);
        style.Set(Property::OutlineOffset);
    } else if (prop == "padding") {
        const auto parts = SplitWhitespace(resolved);
        if (!parts.empty()) {
            const std::vector<int> px = {
                LengthToPixels(parts[0], context),
                LengthToPixels(parts.size() > 1 ? parts[1] : parts[0], context),
                LengthToPixels(parts.size() > 2 ? parts[2] : parts[0], context),
                LengthToPixels(parts.size() > 3 ? parts[3] : (parts.size() > 1 ? parts[1] : parts[0]), context)
            };
            style.paddingTop = px[0];
            style.paddingRight = px[1];
            style.paddingBottom = px[2];
            style.paddingLeft = px[3];
            style.Set(Property::PaddingTop);
            style.Set(Property::PaddingRight);
            style.Set(Property::PaddingBottom);
            style.Set(Property::PaddingLeft);
        }
    } else if (prop == "padding-top") {
        style.paddingTop = LengthToPixels(resolved, context);
        style.Set(Property::PaddingTop);
    } else if (prop == "padding-right") {
        style.paddingRight = LengthToPixels(resolved, context);
        style.Set(Property::PaddingRight);
    } else if (prop == "padding-bottom") {
        style.paddingBottom = LengthToPixels(resolved, context);
        style.Set(Property::PaddingBottom);
    } else if (prop == "padding-left") {
        style.paddingLeft = LengthToPixels(resolved, context);
        style.Set(Property::PaddingLeft);
    } else if (prop == "margin") {
        const auto parts = SplitWhitespace(resolved);
        if (!parts.empty()) {
            const std::vector<int> px = {
                LengthToPixels(parts[0], context),
                LengthToPixels(parts.size() > 1 ? parts[1] : parts[0], context),
                LengthToPixels(parts.size() > 2 ? parts[2] : parts[0], context),
                LengthToPixels(parts.size() > 3 ? parts[3] : (parts.size() > 1 ? parts[1] : parts[0]), context)
            };
            style.marginTop = px[0];
            style.marginRight = px[1];
            style.marginBottom = px[2];
            style.marginLeft = px[3];
            style.Set(Property::MarginTop);
            style.Set(Property::MarginRight);
            style.Set(Property::MarginBottom);
            style.Set(Property::MarginLeft);
        }
    } else if (prop == "margin-top") {
        style.marginTop = LengthToPixels(resolved, context);
        style.Set(Property::MarginTop);
    } else if (prop == "margin-right") {
        style.marginRight = LengthToPixels(resolved, context);
        style.Set(Property::MarginRight);
    } else if (prop == "margin-bottom") {
        style.marginBottom = LengthToPixels(resolved, context);
        style.Set(Property::MarginBottom);
    } else if (prop == "margin-left") {
        style.marginLeft = LengthToPixels(resolved, context);
        style.Set(Property::MarginLeft);
    } else if (prop == "spacing") {
        style.spacing = LengthToPixels(resolved, context);
        style.Set(Property::Spacing);
    } else if (prop == "text-align") {
        const wxString s = ToLowerAscii(resolved);
        if (s == "left") style.textAlign = TextAlign::Left;
        else if (s == "right") style.textAlign = TextAlign::Right;
        else if (s == "center") style.textAlign = TextAlign::Center;
        else if (s == "justify") style.textAlign = TextAlign::Justify;
        style.Set(Property::TextAlign);
    } else if (prop == "text-decoration") {
        const wxString s = ToLowerAscii(resolved);
        if (s == "underline") style.textDecoration = TextDecoration::Underline;
        else if (s == "overline") style.textDecoration = TextDecoration::Overline;
        else if (s == "line-through") style.textDecoration = TextDecoration::LineThrough;
        else style.textDecoration = TextDecoration::None;
        style.Set(Property::TextDecoration);
    } else if (prop == "font-size") {
        const int pixels = LengthToPixels(resolved, context);
        const int points = PixelsToPoints(pixels, context);
        style.font = wxFont(wxFontInfo(points));
        style.Set(Property::Font);
    } else if (prop == "width") {
        style.width = LengthToPixels(resolved, context);
        style.Set(Property::Width);
    } else if (prop == "height") {
        style.height = LengthToPixels(resolved, context);
        style.Set(Property::Height);
    } else if (prop == "min-width") {
        style.minWidth = LengthToPixels(resolved, context);
        style.Set(Property::MinWidth);
    } else if (prop == "max-width") {
        style.maxWidth = LengthToPixels(resolved, context);
        style.Set(Property::MaxWidth);
    } else if (prop == "min-height") {
        style.minHeight = LengthToPixels(resolved, context);
        style.Set(Property::MinHeight);
    } else if (prop == "max-height") {
        style.maxHeight = LengthToPixels(resolved, context);
        style.Set(Property::MaxHeight);
    } else if (prop == "icon" || prop == "image") {
        style.icon = LoadBitmapFromUrl(resolved);
        style.Set(Property::Icon);
    } else if (prop == "icon-size") {
        const auto parts = SplitWhitespace(resolved);
        if (parts.size() >= 2) {
            style.iconSize = wxSize(LengthToPixels(parts[0], context),
                                    LengthToPixels(parts[1], context));
        } else if (!parts.empty()) {
            const int s = LengthToPixels(parts[0], context);
            style.iconSize = wxSize(s, s);
        }
        style.Set(Property::IconSize);
    }
}

bool StyleResolver::Contains(const std::vector<wxString>& list, const wxString& value)
{
    for (const auto& item : list) {
        if (item == value) {
            return true;
        }
    }
    return false;
}

} // namespace wxCustomization
