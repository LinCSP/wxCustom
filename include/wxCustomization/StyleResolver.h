#pragma once

#include "wxCustomization/Style.h"
#include "wxCustomization/StyleResolverContext.h"
#include "wxCustomization/StyleSheet.h"

#include <wx/string.h>

namespace wxCustomization {

/// Resolves a concrete Style for a widget from a StyleSheet.
class StyleResolver {
public:
    StyleResolver() = default;

    Style Resolve(const StyleSheet& sheet,
                  const StyleResolverContext& context,
                  const wxString& subControl = wxEmptyString,
                  const wxString& state = wxEmptyString) const;

private:
    struct Match {
        const StyleRule* rule = nullptr;
        size_t selectorIndex = 0;
        int specificity = 0;
    };

    std::vector<Match> CollectMatches(const StyleSheet& sheet,
                                      const StyleResolverContext& context,
                                      const wxString& subControl,
                                      const wxString& state) const;

    bool Matches(const SelectorSequence& seq,
                 const StyleResolverContext& context,
                 const wxString& subControl,
                 const wxString& state) const;

    bool MatchesSimpleSelector(const SimpleSelector& sel,
                               const StyleResolverContext& context,
                               const wxString& subControl,
                               const wxString& state) const;

    int CalculateSpecificity(const SelectorSequence& seq) const;

    wxString ResolveVariables(const wxString& value, const StyleSheet& sheet) const;

    void ApplyDeclaration(Style& style,
                          const StyleSheet& sheet,
                          const wxString& property,
                          const wxString& value,
                          const wxWindow* context) const;

    static bool Contains(const std::vector<wxString>& list, const wxString& value);
};

} // namespace wxCustomization
