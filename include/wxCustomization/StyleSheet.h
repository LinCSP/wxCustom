#pragma once

#include "wxCustomization/StyleRule.h"

#include <wx/string.h>
#include <map>
#include <vector>

namespace wxCustomization {

class StyleSheet {
public:
    StyleSheet() = default;

    bool LoadFromString(const wxString& text);
    bool Load(const wxString& filePath);

    void Clear();

    const std::vector<StyleRule>& GetRules() const { return m_rules; }
    void AddRule(StyleRule rule) { m_rules.push_back(std::move(rule)); }

    wxString GetVariable(const wxString& name) const;
    void SetVariable(const wxString& name, const wxString& value);

    wxString GetLastError() const { return m_lastError; }

private:
    std::vector<StyleRule> m_rules;
    std::map<wxString, wxString> m_variables;
    wxString m_lastError;
};

} // namespace wxCustomization
