#ifndef LEXERJASS_HPP
#define LEXERJASS_HPP

#include "String32.hpp"
#include "Vector.hpp"
#include "HashSet.hpp"

#include "Lexer.hpp"

#include "Theme.hpp"

#include "TokenizerJass.hpp"

enum JassStyle
{
    JASS_NATIVE = STYLE_LAST_PREDEFINED + 1,
    JASS_FUNCTION,
    JASS_CONSTANT,
    JASS_TYPE
};

class LexerJass final : public Lexer
{
public:
    using Lexer::Lexer;

    void StyleToken(Jass::Token const& token, int start);

    virtual void Style(Position start, Position stop);

    virtual ~LexerJass() = default;
};

#endif // LEXERJASS_HPP
