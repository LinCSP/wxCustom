#include "wxCustomization/StyleSheet.h"

#include "wxCustomization/Lexer.h"

#include <wx/file.h>
#include <cctype>
#include <stdexcept>

namespace wxCustomization {

namespace {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens)
        : m_tokens(tokens)
    {
    }

    bool Parse(StyleSheet& sheet, wxString& error)
    {
        m_pos = 0;
        while (!Check(Token::Type::Eof)) {
            StyleRule rule;
            if (!ParseRule(rule, error)) {
                return false;
            }

            // :root declarations become CSS variables.
            for (const auto& seq : rule.selectors) {
                if (seq.parts.size() == 1 && seq.parts[0].IsRoot()) {
                    for (const auto& decl : rule.declarations) {
                        if (!decl.property.empty() && decl.property[0] == '-') {
                            sheet.SetVariable(decl.property, decl.value);
                        }
                    }
                }
            }

            sheet.AddRule(std::move(rule));
        }
        return true;
    }

private:
    bool ParseRule(StyleRule& rule, wxString& error)
    {
        if (!ParseSelectors(rule.selectors, error)) {
            return false;
        }
        if (!Consume(Token::Type::LBrace, "Expected '{' after selector", error)) {
            return false;
        }
        if (!ParseDeclarations(rule.declarations, error)) {
            return false;
        }
        if (!Consume(Token::Type::RBrace, "Expected '}' after declarations", error)) {
            return false;
        }
        return true;
    }

    bool ParseSelectors(std::vector<SelectorSequence>& selectors, wxString& error)
    {
        SelectorSequence first;
        if (!ParseSelectorSequence(first, error)) {
            return false;
        }
        selectors.push_back(std::move(first));

        while (Match(Token::Type::Comma)) {
            SelectorSequence seq;
            if (!ParseSelectorSequence(seq, error)) {
                return false;
            }
            selectors.push_back(std::move(seq));
        }
        return true;
    }

    bool ParseSelectorSequence(SelectorSequence& seq, wxString& error)
    {
        SimpleSelector first;
        if (!ParseSimpleSelector(first, error)) {
            return false;
        }
        seq.parts.push_back(std::move(first));

        while (!IsSelectorEnd()) {
            SelectorCombinator comb = SelectorCombinator::Descendant;
            if (Check(Token::Type::Gt)) {
                comb = SelectorCombinator::Child;
                Advance();
            } else if (Check(Token::Type::Plus)) {
                comb = SelectorCombinator::AdjacentSibling;
                Advance();
            } else if (Check(Token::Type::Ident) && Peek().value == "~") {
                comb = SelectorCombinator::GeneralSibling;
                Advance();
            }

            if (IsSelectorEnd()) {
                break;
            }

            SimpleSelector sel;
            if (!ParseSimpleSelector(sel, error)) {
                return false;
            }
            seq.combinators.push_back(comb);
            seq.parts.push_back(std::move(sel));
        }

        return true;
    }

    bool ParseSimpleSelector(SimpleSelector& sel, wxString& error)
    {
        bool hadSomething = false;

        // Optional element type or universal selector.
        if (Check(Token::Type::Ident)) {
            sel.type = Peek().value;
            Advance();
            hadSomething = true;
        } else if (Check(Token::Type::Star)) {
            sel.type = "*";
            Advance();
            hadSomething = true;
        }

        while (true) {
            if (Check(Token::Type::Class)) {
                sel.className = Peek().value;
                Advance();
                hadSomething = true;
            } else if (Check(Token::Type::Id)) {
                sel.id = Peek().value;
                Advance();
                hadSomething = true;
            } else if (Check(Token::Type::Colon)) {
                Advance();
                if (Check(Token::Type::Colon)) {
                    // ::sub-control
                    Advance();
                    if (!Consume(Token::Type::Ident, "Expected sub-control name after '::'", error)) {
                        return false;
                    }
                    sel.subControl = Previous().value;
                } else {
                    if (!Consume(Token::Type::Ident, "Expected pseudo-class after ':'", error)) {
                        return false;
                    }
                    sel.pseudo = Previous().value;
                }
                hadSomething = true;
            } else if (Check(Token::Type::LSquare)) {
                if (!ParseAttribute(sel, error)) {
                    return false;
                }
                hadSomething = true;
            } else {
                break;
            }
        }

        if (!hadSomething) {
            error = ErrorMessage("Expected selector");
            return false;
        }

        return true;
    }

    bool ParseAttribute(SimpleSelector& sel, wxString& error)
    {
        if (!Consume(Token::Type::LSquare, "Expected '['", error)) {
            return false;
        }
        if (!Consume(Token::Type::Ident, "Expected attribute name", error)) {
            return false;
        }
        sel.attrName = Previous().value;

        if (!Consume(Token::Type::Ident, "Expected '=' in attribute selector", error)) {
            return false;
        }
        if (Previous().value != "=") {
            error = ErrorMessage("Expected '=' in attribute selector");
            return false;
        }

        if (!Consume(Token::Type::String, "Expected attribute value", error)) {
            return false;
        }
        sel.attrValue = Previous().value;

        if (!Consume(Token::Type::RSquare, "Expected ']'", error)) {
            return false;
        }
        return true;
    }

    bool ParseDeclarations(std::vector<Declaration>& declarations, wxString& error)
    {
        while (!Check(Token::Type::RBrace) && !Check(Token::Type::Eof)) {
            Declaration decl;
            if (!ParseDeclaration(decl, error)) {
                return false;
            }
            declarations.push_back(std::move(decl));
        }
        return true;
    }

    bool ParseDeclaration(Declaration& decl, wxString& error)
    {
        if (!Consume(Token::Type::Ident, "Expected property name", error)) {
            return false;
        }
        decl.property = Previous().value;

        if (!Consume(Token::Type::Colon, "Expected ':' after property name", error)) {
            return false;
        }

        std::vector<wxString> parts;
        while (!Check(Token::Type::Semicolon) &&
               !Check(Token::Type::RBrace) &&
               !Check(Token::Type::Eof)) {
            parts.push_back(Peek().value);
            Advance();
        }
        decl.value = JoinValueParts(parts);

        if (!Match(Token::Type::Semicolon)) {
            // Declaration may end at '}', which is consumed by the caller.
            if (!Check(Token::Type::RBrace) && !Check(Token::Type::Eof)) {
                error = ErrorMessage("Expected ';' or '}' after declaration value");
                return false;
            }
        }
        return true;
    }

    wxString JoinValueParts(const std::vector<wxString>& parts) const
    {
        wxString result;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) {
                result += " ";
            }
            result += parts[i];
        }
        return result;
    }

    bool IsSelectorEnd() const
    {
        return Check(Token::Type::LBrace) ||
               Check(Token::Type::Comma) ||
               Check(Token::Type::Semicolon) ||
               Check(Token::Type::Eof);
    }

    const Token& Peek(size_t offset = 0) const
    {
        const size_t idx = m_pos + offset;
        if (idx >= m_tokens.size()) {
            return m_tokens.back();
        }
        return m_tokens[idx];
    }

    bool Check(Token::Type type) const
    {
        return Peek().type == type;
    }

    bool Match(Token::Type type)
    {
        if (Check(type)) {
            ++m_pos;
            return true;
        }
        return false;
    }

    bool Consume(Token::Type type, const char* message, wxString& error)
    {
        if (Check(type)) {
            ++m_pos;
            return true;
        }
        error = ErrorMessage(message);
        return false;
    }

    const Token& Previous() const
    {
        if (m_pos == 0) {
            return m_tokens.front();
        }
        return m_tokens[m_pos - 1];
    }

    void Advance()
    {
        if (!Check(Token::Type::Eof)) {
            ++m_pos;
        }
    }

    wxString ErrorMessage(const char* message) const
    {
        const Token& t = Peek();
        return wxString::Format("%s at line %d, column %d", message, t.line, t.column);
    }

    const std::vector<Token>& m_tokens;
    size_t m_pos = 0;
};

} // namespace

bool StyleSheet::LoadFromString(const wxString& text)
{
    Clear();
    Lexer lexer(text);
    const auto tokens = lexer.Tokenize();
    wxString error;
    Parser parser(tokens);
    if (!parser.Parse(*this, error)) {
        m_lastError = wxString::Format("Failed to parse stylesheet: %s", error);
        return false;
    }
    return true;
}

bool StyleSheet::Load(const wxString& filePath)
{
    wxFile file(filePath);
    if (!file.IsOpened()) {
        m_lastError = wxString::Format("Cannot open stylesheet file: %s", filePath);
        return false;
    }
    wxString text;
    if (!file.ReadAll(&text)) {
        m_lastError = wxString::Format("Cannot read stylesheet file: %s", filePath);
        return false;
    }
    return LoadFromString(text);
}

void StyleSheet::Clear()
{
    m_rules.clear();
    m_variables.clear();
}

wxString StyleSheet::GetVariable(const wxString& name) const
{
    auto it = m_variables.find(name);
    return it != m_variables.end() ? it->second : wxString();
}

void StyleSheet::SetVariable(const wxString& name, const wxString& value)
{
    m_variables[name] = value;
}

} // namespace wxCustomization
