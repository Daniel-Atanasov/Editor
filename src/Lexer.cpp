#include "Lexer.hpp"

#include "Buffer.hpp"
#include "Theme.hpp"

Buffer * Lexer::Parent()
{
    return parent;
}

Lexer::Lexer(Buffer * parent) : parent(parent)
{
}

void Lexer::SetKeywordStyle(String32 keyword, int style)
{
    keywords[std::move(keyword)] = style;
}

void Lexer::RemoveKeyword(String32 const& keyword)
{
    if (keywords.contains(keyword))
    {
        keywords.remove(keyword);
    }
}

void Lexer::ClearKeywords()
{
    keywords.clear();
}

void Lexer::SetParent(Buffer * new_parent)
{
    parent = new_parent;
}
