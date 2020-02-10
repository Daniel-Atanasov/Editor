#include "Lexer.hpp"

#include "Theme.hpp"

Buffer * Lexer::Parent()
{
    return parent;
}

Lexer::Lexer(Buffer * parent) : parent(parent)
{
}

void Lexer::SetKeywordStyle(HashMap<String32, int> const& new_keywords)
{
    for (auto const& token : new_keywords)
    {
        SetKeywordStyle(token.first, token.second);
    }
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
