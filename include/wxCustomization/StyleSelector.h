#pragma once

#include <wx/string.h>
#include <vector>

namespace wxCustomization {

/// A single simple selector: type#id.class:pseudo::sub-control[attr="value"].
struct SimpleSelector {
    wxString type;        // empty means "any type" (unless everything else is empty too)
    wxString id;
    wxString className;
    wxString pseudo;
    wxString subControl;
    wxString attrName;
    wxString attrValue;

    bool IsEmpty() const
    {
        return type.empty() && id.empty() && className.empty() &&
               pseudo.empty() && subControl.empty() && attrName.empty();
    }

    bool IsRoot() const
    {
        return pseudo == "root" && type.empty() && id.empty() && className.empty() &&
               subControl.empty() && attrName.empty();
    }

    bool operator==(const SimpleSelector& other) const
    {
        return type == other.type && id == other.id && className == other.className &&
               pseudo == other.pseudo && subControl == other.subControl &&
               attrName == other.attrName && attrValue == other.attrValue;
    }

    bool operator!=(const SimpleSelector& other) const { return !(*this == other); }
};

/// Combinator between simple selectors in a sequence.
enum class SelectorCombinator {
    Descendant,       // whitespace
    Child,            // >
    AdjacentSibling,  // +
    GeneralSibling    // ~
};

/// A selector sequence is a chain like "Panel > Button:hover".
struct SelectorSequence {
    std::vector<SimpleSelector> parts;
    std::vector<SelectorCombinator> combinators; // size == parts.size() - 1

    bool operator==(const SelectorSequence& other) const
    {
        return parts == other.parts && combinators == other.combinators;
    }

    bool operator!=(const SelectorSequence& other) const { return !(*this == other); }
};

} // namespace wxCustomization
