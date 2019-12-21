#ifndef LEXER_HPP
#define LEXER_HPP

#include <cstdint>

#include "HashMap.hpp"
#include "String32.hpp"

#include "Cursor.hpp"

class Buffer;

enum GenericStyle : std::uint8_t
{
    STYLE_DEFAULT,
    STYLE_KEYWORD,
    STYLE_NUMBER,
    STYLE_COMMENT_LINE,
    STYLE_COMMENT_BLOCK,
    STYLE_COMMENT_PREPROCESSOR,
    STYLE_DOUBLE_QUOTE_STRING,
    STYLE_DOUBLE_QUOTE_ESCAPE_VALID,
    STYLE_DOUBLE_QUOTE_ESCAPE_INVALID,
    STYLE_SINGLE_QUOTE_STRING,
    STYLE_SINGLE_QUOTE_ESCAPE_VALID,
    STYLE_SINGLE_QUOTE_ESCAPE_INVALID,
    STYLE_LAST_PREDEFINED = STYLE_SINGLE_QUOTE_ESCAPE_INVALID
};

class Lexer
{
private:
    Buffer * parent;

protected:
	HashMap<String32, int> keywords;

    Buffer * Parent();

public:
    Lexer(Buffer * parent = nullptr);

    void SetKeywordStyle(String32 keyword, int style);
    void RemoveKeyword(String32 const& keyword);

    void ClearKeywords();

    virtual void SetParent(Buffer * new_parent);

    virtual void Style(Position start, Position stop) = 0;

    virtual ~Lexer() = default;
};

#endif // LEXER_HPP
