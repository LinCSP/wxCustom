#pragma once

#include <wx/string.h>
#include <string>
#include <vector>

namespace wxCustomization {

struct Token {
    enum class Type {
        Ident,
        Class,
        Id,
        Number,
        Color,
        String,
        Url,
        Colon,
        Semicolon,
        LBrace,
        RBrace,
        Comma,
        Gt,
        LSquare,
        RSquare,
        LParen,
        RParen,
        Plus,
        Star,
        Eof
    };

    Type type = Type::Eof;
    wxString value;
    int line = 1;
    int column = 1;

    Token() = default;
    Token(Type type, const wxString& value, int line, int column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {
    }

    bool Is(Type t) const noexcept { return type == t; }
};

wxString TokenTypeToString(Token::Type type);

class Lexer {
public:
    explicit Lexer(const wxString& input);

    std::vector<Token> Tokenize();

private:
    void SkipWhitespace();
    void SkipComment();

    Token NextToken();
    Token MakeToken(Token::Type type, const wxString& value);

    Token ReadString(char quote);
    Token ReadNumber();
    Token ReadIdentifier();
    Token ReadHashToken();
    Token ReadUrl();

    char Peek(size_t offset = 0) const;
    char Advance();
    bool Match(char expected);

    bool IsAtEnd() const;
    bool IsIdentStart(char c) const;
    bool IsIdentChar(char c) const;
    bool IsHexDigit(char c) const;

    std::string m_input;
    size_t m_pos = 0;
    int m_line = 1;
    int m_column = 1;
};

} // namespace wxCustomization
