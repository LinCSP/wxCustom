#include <gtest/gtest.h>
#include "wxCustomization/Lexer.h"

using wxCustomization::Lexer;
using wxCustomization::Token;

static std::vector<Token::Type> TokenizeTypes(const wxString& input)
{
    Lexer lexer(input);
    const auto tokens = lexer.Tokenize();
    std::vector<Token::Type> result;
    result.reserve(tokens.size());
    for (const auto& t : tokens) {
        result.push_back(t.type);
    }
    return result;
}

TEST(Lexer, EmptyInput)
{
    auto types = TokenizeTypes("");
    ASSERT_EQ(types.size(), 1u);
    EXPECT_EQ(types[0], Token::Type::Eof);
}

TEST(Lexer, SimpleRule)
{
    auto tokens = Lexer("StyledButton { color: red; }").Tokenize();
    ASSERT_EQ(tokens.size(), 8u);
    EXPECT_EQ(tokens[0].type, Token::Type::Ident);
    EXPECT_EQ(tokens[0].value, "StyledButton");
    EXPECT_EQ(tokens[1].type, Token::Type::LBrace);
    EXPECT_EQ(tokens[2].type, Token::Type::Ident);
    EXPECT_EQ(tokens[2].value, "color");
    EXPECT_EQ(tokens[3].type, Token::Type::Colon);
    EXPECT_EQ(tokens[4].type, Token::Type::Ident);
    EXPECT_EQ(tokens[4].value, "red");
    EXPECT_EQ(tokens[5].type, Token::Type::Semicolon);
    EXPECT_EQ(tokens[6].type, Token::Type::RBrace);
    EXPECT_EQ(tokens[7].type, Token::Type::Eof);
}

TEST(Lexer, SelectorsAndPunctuation)
{
    auto types = TokenizeTypes("a > b + c ~ d[e] { }");
    std::vector<Token::Type> expected = {
        Token::Type::Ident, Token::Type::Gt, Token::Type::Ident,
        Token::Type::Plus, Token::Type::Ident, Token::Type::Ident,
        Token::Type::Ident, Token::Type::LSquare, Token::Type::Ident,
        Token::Type::RSquare, Token::Type::LBrace, Token::Type::RBrace,
        Token::Type::Eof
    };
    EXPECT_EQ(types, expected);
}

TEST(Lexer, ClassAndId)
{
    auto tokens = Lexer(".class #id").Tokenize();
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, Token::Type::Class);
    EXPECT_EQ(tokens[0].value, "class");
    EXPECT_EQ(tokens[1].type, Token::Type::Id);
    EXPECT_EQ(tokens[1].value, "id");
    EXPECT_EQ(tokens[2].type, Token::Type::Eof);
}

TEST(Lexer, ColorVsId)
{
    auto tokens = Lexer("#abc #abcdef #item").Tokenize();
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, Token::Type::Color);
    EXPECT_EQ(tokens[0].value, "#abc");
    EXPECT_EQ(tokens[1].type, Token::Type::Color);
    EXPECT_EQ(tokens[1].value, "#abcdef");
    EXPECT_EQ(tokens[2].type, Token::Type::Id);
    EXPECT_EQ(tokens[2].value, "item");
}

TEST(Lexer, NumbersAndUnits)
{
    auto tokens = Lexer("10px -2.5em 50%").Tokenize();
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, Token::Type::Number);
    EXPECT_EQ(tokens[0].value, "10px");
    EXPECT_EQ(tokens[1].type, Token::Type::Number);
    EXPECT_EQ(tokens[1].value, "-2.5em");
    EXPECT_EQ(tokens[2].type, Token::Type::Number);
    EXPECT_EQ(tokens[2].value, "50%");
    EXPECT_EQ(tokens[3].type, Token::Type::Eof);
}

TEST(Lexer, Strings)
{
    auto tokens = Lexer("'hello' \"world\"").Tokenize();
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, Token::Type::String);
    EXPECT_EQ(tokens[0].value, "hello");
    EXPECT_EQ(tokens[1].type, Token::Type::String);
    EXPECT_EQ(tokens[1].value, "world");
}

TEST(Lexer, Url)
{
    auto tokens = Lexer("url(\"images/bg.png\")").Tokenize();
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, Token::Type::Url);
    EXPECT_EQ(tokens[0].value, "images/bg.png");
}

TEST(Lexer, CommentsAreSkipped)
{
    auto tokens = Lexer("/* comment */ a { } /* another */").Tokenize();
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, Token::Type::Ident);
    EXPECT_EQ(tokens[1].type, Token::Type::LBrace);
    EXPECT_EQ(tokens[2].type, Token::Type::RBrace);
    EXPECT_EQ(tokens[3].type, Token::Type::Eof);
}

TEST(Lexer, LineColumnTracking)
{
    auto tokens = Lexer("a\n{ b: 1; }").Tokenize();
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[1].line, 2);
    EXPECT_EQ(tokens[1].column, 1);
}
