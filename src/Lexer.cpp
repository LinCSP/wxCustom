#include "wxCustomization/Lexer.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace wxCustomization {

wxString TokenTypeToString(Token::Type type)
{
    switch (type) {
        case Token::Type::Ident: return "Ident";
        case Token::Type::Class: return "Class";
        case Token::Type::Id: return "Id";
        case Token::Type::Number: return "Number";
        case Token::Type::Color: return "Color";
        case Token::Type::String: return "String";
        case Token::Type::Url: return "Url";
        case Token::Type::Colon: return "Colon";
        case Token::Type::Semicolon: return "Semicolon";
        case Token::Type::LBrace: return "LBrace";
        case Token::Type::RBrace: return "RBrace";
        case Token::Type::Comma: return "Comma";
        case Token::Type::Gt: return "Gt";
        case Token::Type::LSquare: return "LSquare";
        case Token::Type::RSquare: return "RSquare";
        case Token::Type::LParen: return "LParen";
        case Token::Type::RParen: return "RParen";
        case Token::Type::Plus: return "Plus";
        case Token::Type::Star: return "Star";
        case Token::Type::Eof: return "Eof";
    }
    return "Unknown";
}

Lexer::Lexer(const wxString& input)
    // Use UTF-8 explicitly: ToStdString() would go through the locale
    // encoding and silently return an empty string for non-ASCII input
    // (e.g. a comment with a typographic dash) under a non-UTF-8 locale.
    : m_input(input.utf8_string())
{
}

std::vector<Token> Lexer::Tokenize()
{
    std::vector<Token> tokens;
    Token token;
    do {
        token = NextToken();
        tokens.push_back(token);
    } while (token.type != Token::Type::Eof);
    return tokens;
}

Token Lexer::NextToken()
{
    SkipWhitespace();
    SkipComment();
    SkipWhitespace();

    if (IsAtEnd()) {
        return MakeToken(Token::Type::Eof, wxString());
    }

    const int startLine = m_line;
    const int startColumn = m_column;
    const char c = Peek();

    auto make = [&](Token::Type type, const wxString& value) {
        return Token(type, value, startLine, startColumn);
    };

    switch (c) {
        case ':': Advance(); return make(Token::Type::Colon, wxString(":"));
        case ';': Advance(); return make(Token::Type::Semicolon, wxString(";"));
        case '{': Advance(); return make(Token::Type::LBrace, wxString("{"));
        case '}': Advance(); return make(Token::Type::RBrace, wxString("}"));
        case ',': Advance(); return make(Token::Type::Comma, wxString(","));
        case '>': Advance(); return make(Token::Type::Gt, wxString(">"));
        case '[': Advance(); return make(Token::Type::LSquare, wxString("["));
        case ']': Advance(); return make(Token::Type::RSquare, wxString("]"));
        case '(': Advance(); return make(Token::Type::LParen, wxString("("));
        case ')': Advance(); return make(Token::Type::RParen, wxString(")"));
        case '+': Advance(); return make(Token::Type::Plus, wxString("+"));
        case '*': Advance(); return make(Token::Type::Star, wxString("*"));
        case '"': return ReadString('"');
        case '\'': return ReadString('\'');
        case '.': {
            if (IsIdentStart(Peek(1))) {
                Advance(); // consume '.'
                Token t = ReadIdentifier();
                t.type = Token::Type::Class;
                t.line = startLine;
                t.column = startColumn;
                return t;
            }
            // Standalone dot is not expected; consume as ident.
            return ReadIdentifier();
        }
        case '#': return ReadHashToken();
        default:
            break;
    }

    // url(...)
    if (m_input.compare(m_pos, 4, "url(") == 0 ||
        m_input.compare(m_pos, 4, "URL(") == 0) {
        return ReadUrl();
    }

    if (c == '-' || c == '+' || std::isdigit(static_cast<unsigned char>(c))) {
        // Distinguish signed numbers from identifiers.
        if ((c == '-' || c == '+') && !std::isdigit(static_cast<unsigned char>(Peek(1)))) {
            if (IsIdentStart(Peek(1))) {
                return ReadIdentifier();
            }
            // Lone sign: treat as identifier for robustness.
            return ReadIdentifier();
        }
        return ReadNumber();
    }

    if (IsIdentStart(c)) {
        return ReadIdentifier();
    }

    // Unknown character: skip it and return as a single-char ident.
    Advance();
    return make(Token::Type::Ident, wxString(wxChar(c)));
}

Token Lexer::MakeToken(Token::Type type, const wxString& value)
{
    return Token(type, value, m_line, m_column);
}

void Lexer::SkipWhitespace()
{
    while (!IsAtEnd()) {
        char c = Peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            Advance();
        } else {
            break;
        }
    }
}

void Lexer::SkipComment()
{
    if (Peek() == '/' && Peek(1) == '*') {
        Advance(); // '/'
        Advance(); // '*'
        while (!IsAtEnd() && !(Peek() == '*' && Peek(1) == '/')) {
            Advance();
        }
        if (!IsAtEnd()) {
            Advance(); // '*'
            Advance(); // '/'
        }
    }
}

Token Lexer::ReadString(char quote)
{
    const int startLine = m_line;
    const int startColumn = m_column;
    Advance(); // opening quote
    std::string value;
    while (!IsAtEnd() && Peek() != quote) {
        if (Peek() == '\\' && Peek(1) != '\0') {
            Advance(); // backslash
            value += Advance();
        } else {
            value += Advance();
        }
    }
    if (!IsAtEnd()) {
        Advance(); // closing quote
    }
    return Token(Token::Type::String, wxString::FromUTF8(value), startLine, startColumn);
}

Token Lexer::ReadNumber()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    std::string value;

    if (Peek() == '-' || Peek() == '+') {
        value += Advance();
    }

    while (!IsAtEnd() && (std::isdigit(static_cast<unsigned char>(Peek())) || Peek() == '.')) {
        value += Advance();
    }

    // Consume an immediately following unit (e.g. px, dip, em, %) so that
    // values like "8px" stay as a single token. This is important for
    // shorthand properties such as "padding: 8px 16px".
    if (!IsAtEnd() && (IsIdentStart(Peek()) || Peek() == '%')) {
        while (!IsAtEnd() && (IsIdentChar(Peek()) || Peek() == '%')) {
            value += Advance();
        }
    }

    return Token(Token::Type::Number, wxString::FromUTF8(value), startLine, startColumn);
}

Token Lexer::ReadIdentifier()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    std::string value;

    while (!IsAtEnd() && IsIdentChar(Peek())) {
        value += Advance();
    }

    return Token(Token::Type::Ident, wxString::FromUTF8(value), startLine, startColumn);
}

Token Lexer::ReadHashToken()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    Advance(); // '#'
    std::string value;
    while (!IsAtEnd() && IsIdentChar(Peek())) {
        value += Advance();
    }

    // Decide between color (#rgb, #rgba, #rrggbb, #rrggbbaa) and id selector.
    const bool allHex = !value.empty() && std::all_of(value.begin(), value.end(),
        [this](char c) { return IsHexDigit(c); });
    const bool isColor = allHex && (value.size() == 3 || value.size() == 4 || value.size() == 6 || value.size() == 8);

    if (isColor) {
        return Token(Token::Type::Color, wxString::FromUTF8("#" + value), startLine, startColumn);
    }
    return Token(Token::Type::Id, wxString::FromUTF8(value), startLine, startColumn);
}

Token Lexer::ReadUrl()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    // consume "url("
    Advance();
    Advance();
    Advance();
    Advance();

    SkipWhitespace();
    std::string value;

    char quote = Peek();
    if (quote == '"' || quote == '\'') {
        Advance(); // opening quote
        while (!IsAtEnd() && Peek() != quote) {
            value += Advance();
        }
        if (!IsAtEnd()) {
            Advance(); // closing quote
        }
    } else {
        while (!IsAtEnd() && Peek() != ')') {
            value += Advance();
        }
    }

    SkipWhitespace();
    if (!IsAtEnd() && Peek() == ')') {
        Advance();
    }

    return Token(Token::Type::Url, wxString::FromUTF8(value), startLine, startColumn);
}

char Lexer::Peek(size_t offset) const
{
    if (m_pos + offset >= m_input.size()) {
        return '\0';
    }
    return m_input[m_pos + offset];
}

char Lexer::Advance()
{
    if (IsAtEnd()) {
        return '\0';
    }
    const char c = m_input[m_pos++];
    if (c == '\n') {
        ++m_line;
        m_column = 1;
    } else {
        ++m_column;
    }
    return c;
}

bool Lexer::Match(char expected)
{
    if (Peek() == expected) {
        Advance();
        return true;
    }
    return false;
}

bool Lexer::IsAtEnd() const
{
    return m_pos >= m_input.size();
}

bool Lexer::IsIdentStart(char c) const
{
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '-';
}

bool Lexer::IsIdentChar(char c) const
{
    return IsIdentStart(c) || std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::IsHexDigit(char c) const
{
    return std::isxdigit(static_cast<unsigned char>(c));
}

} // namespace wxCustomization
