#pragma once

#include "wxCustomization/StyleSelector.h"

#include <wx/string.h>
#include <vector>

namespace wxCustomization {

struct Declaration {
    wxString property;
    wxString value;

    bool operator==(const Declaration& other) const
    {
        return property == other.property && value == other.value;
    }

    bool operator!=(const Declaration& other) const { return !(*this == other); }
};

struct StyleRule {
    std::vector<SelectorSequence> selectors;
    std::vector<Declaration> declarations;

    bool operator==(const StyleRule& other) const
    {
        return selectors == other.selectors && declarations == other.declarations;
    }

    bool operator!=(const StyleRule& other) const { return !(*this == other); }
};

} // namespace wxCustomization
