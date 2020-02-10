#include "TokenizerJass.hpp"

#include <map>

#include "SpecialCharacters.hpp"

#include "LexerJass.hpp"

namespace Jass
{
    namespace
    {
        static HashMap<StringView32, TokenType> keywords
        {
            {U"globals", TokenType::Globals},
            {U"endglobals", TokenType::EndGlobals},
            {U"type", TokenType::Type},
            {U"extends", TokenType::Extends},
            {U"native", TokenType::Native},
            {U"takes", TokenType::Takes},
            {U"returns", TokenType::Returns},
            {U"function", TokenType::Function},
            {U"endfunction", TokenType::EndFunction},
            {U"method", TokenType::Method},
            {U"endmethod", TokenType::EndMethod},
            {U"operator", TokenType::Operator},
            {U"struct", TokenType::Struct},
            {U"endstruct", TokenType::EndStruct},
            {U"interface", TokenType::Interface},
            {U"endinterface", TokenType::EndInterface},
            {U"module", TokenType::Module},
            {U"endmodule", TokenType::EndModule},
            {U"implement", TokenType::Implement},
            {U"scope", TokenType::Scope},
            {U"endscope", TokenType::EndScope},
            {U"initializer", TokenType::Initializer},
            {U"library", TokenType::Library},
            {U"endlibrary", TokenType::EndLibrary},
            {U"requires", TokenType::Requires},
            {U"uses", TokenType::Uses},
            {U"needs", TokenType::Needs},
            {U"loop", TokenType::Loop},
            {U"exitwhen", TokenType::ExitWhen},
            {U"endloop", TokenType::EndLoop},
            {U"if", TokenType::If},
            {U"then", TokenType::Then},
            {U"elseif", TokenType::ElseIf},
            {U"else", TokenType::Else},
            {U"endif", TokenType::EndIf},
            {U"and", TokenType::And},
            {U"or", TokenType::Or},
            {U"not", TokenType::Not},
            {U"private", TokenType::Private},
            {U"public", TokenType::Public},
            {U"static", TokenType::Static},
            {U"constant", TokenType::Constant},
            {U"local", TokenType::Local},
            {U"set", TokenType::Set},
            {U"call", TokenType::Call},
            {U"return", TokenType::Return},

            {U"nothing", TokenType::Nothing},
            {U"array", TokenType::Array},

            {U"null", TokenType::Null},
            {U"true", TokenType::True},
            {U"false", TokenType::False}
        };
    }

    Token::Token(TokenType type, int start, int stop, String32 value, Vector<Token> tokens) :
        type(type),
        start(start),
        stop(stop),
        value(std::move(value)),
        tokens(std::move(tokens))
    {
    }

    Token::Token(TokenType type, int start, int stop, Vector<Token> tokens) :
        type(type),
        start(start),
        stop(stop),
        tokens(std::move(tokens))
    {
    }

    TokenType Token::Type() const
    {
        return type;
    }

    int Token::Start() const
    {
        return start;
    }

    int Token::Stop() const
    {
        return stop;
    }

    int Token::Length() const
    {
        return stop - start;
    }

    bool Token::Is(TokenType token_type) const
    {
        return type == token_type;
    }

    bool Token::IsComment() const
    {
        return type == TokenType::CommentBlock || type == TokenType::CommentLine || type == TokenType::PreprocessorComment;
    }

    bool Token::IsKeyword() const
    {
        return type >= TokenType::KeywordFirst && type <= TokenType::KeywordLast;
    }

    String32 const& Token::Value() const&
    {
        return value;
    }

    String32 Token::Value() &&
    {
        return std::move(value);
    }

    Vector<Token> const& Token::Tokens() const&
    {
        return tokens;
    }

    Vector<Token> Token::Tokens() &&
    {
        return std::move(tokens);
    }

    Token ReadCommentBlock(StringView32 text, int start, bool add_offset)
    {
        int stop = start;

        Vector<Token> tokens; // TODO@Daniel: Find a good way to implement doc params
        for (; stop != text.size(); stop++)
        {
            if (text.middle_view(stop, 2) == U"*/")
            {
                stop += 2;
                break;
            }
        }

        return Token(TokenType::CommentBlock, start - add_offset * 2, stop, std::move(tokens));
    }

    Token ReadStringLiteral(StringView32 text, int start, bool add_offset)
    {
        StringView32 slash_escapes = U"\\nt\"";
        StringView32 pipe_escapes = U"cnr";

        int stop = start;

        Vector<Token> tokens;
        for (; stop < text.size(); stop++)
        {
            StringView32 token = text.middle_view(stop, 2);

            switch (token[0])
            {
            case U'"':
                stop++;
                goto ret;
            case U'\\':
            case U'|':
                if (token.size() == 2)
                {
                    bool b;
                    if (token[0] == U'\\') b = slash_escapes.contains(token[1]);
                    else                  b = pipe_escapes.contains(token[1]);

                    TokenType type;
                    if (b) type = TokenType::ValidEscapeSequence;
                    else   type = TokenType::InvalidEscapeSequence;

                    tokens.emplace_back(type, stop, stop + 2);
                    stop ++;
                }
                break;
            }
        }
        ret:

        return Token(TokenType::String, start - add_offset, stop, std::move(tokens));
    }

    Token ReadRawcodeLiteral(StringView32 text, int start, bool add_offset)
    {
        int stop = start;
        while (stop < text.size())
        {
            if (text[stop] == U'\'' || text[stop] == U'\0')
            {
                stop++;
                break;
            }
            stop++;
        }

        return Token(TokenType::Rawcode, start - add_offset, stop);
    }

    Token ReadNumber(StringView32 text, int idx)
    {
        int start = idx;
        int stop = start + 1;
        while (stop < text.size() && IsDigit(text[stop])) stop++;

        String32 value = text.middle(start, stop - start);

        return Token(TokenType::Number, start, stop, std::move(value));
    }

    Token ReadIdentifier(StringView32 text, int idx, bool ignore_keywords)
    {
        int start = idx;
        int stop = start + 1;
        while (stop < text.size() && IsIdentifierChar(text[stop])) stop++;

        TokenType type = TokenType::Identifier;

        StringView32 value = text.middle_view(start, stop - start);

        if (!ignore_keywords)
        {
            auto it = keywords.find(value);
            if (it != std::end(keywords)) type = it->second;
        }

        if (type == TokenType::Identifier)
        {
            return Token(type, start, stop, value);
        }

        return Token(type, start, stop);
    }

    Token NextToken(StringView32 text, int start)
    {
        auto at = [&text](int idx)
        {
            if (idx < 0 || idx >= text.size()) return U'\0';
            return text[idx];
        };

        // Newlines are also ignored since this is mostly intended for highlighting
        while (IsSpace(at(start))) start++;

        if (start == text.size())
        {
            return Token(TokenType::Eof, start, start);
        }

        char32_t first = at(start);
        char32_t second = at(start + 1);
        char32_t third = at(start + 2);

        TokenType type = TokenType::Unknown;

        int stop = start + 1;

        switch (first)
        {
        case U'(': type = TokenType::OpenParen;    break;
        case U')': type = TokenType::CloseParen;   break;
        case U'[': type = TokenType::OpenBracket;  break;
        case U']': type = TokenType::CloseBracket; break;
        case U'{': type = TokenType::OpenBrace;    break;
        case U'}': type = TokenType::CloseBrace;   break;
        case U'.': type = TokenType::Dot;          break;
        case U',': type = TokenType::Comma;        break;
        case U'=':
            if (second == U'=')
            {
                type = TokenType::Equal;
                stop++;
            }
            else
            {
                type = TokenType::Assign;
            }
            break;
        case U'+':
            if (second == U'=')
            {
                type = TokenType::AssignAdd;
                stop++;
            }
            else
            {
                type = TokenType::Add;
            }
            break;
        case U'-':
            if (second == U'=')
            {
                type = TokenType::AssignSub;
                stop++;
            }
            else
            {
                type = TokenType::Sub;
            }
            break;
        case U'*':
            if (second == U'=')
            {
                type = TokenType::AssignMul;
                stop++;
            }
            else
            {
                type = TokenType::Mul;
            }
            break;
        case U'/':
            if (second == U'*')
            {
                return ReadCommentBlock(text, stop + 1);
            }
            else if (second == U'/')
            {
                if (third == U'!')
                {
                    type = TokenType::PreprocessorComment;
                    stop += 2;
                }
                else
                {
                    type = TokenType::CommentLine;
                    stop += 1;
                }

                for (;;)
                {
                    if (at(stop) == U'\n') break;
                    if (at(stop) == U'\0') break;
                    stop++;
                }
            }
            else if (second == U'=')
            {
                type = TokenType::AssignDiv;
                stop++;
            }
            else
            {
                type = TokenType::Div;
            }
            break;
        case U'<':
            if (second == U'=')
            {
                type = TokenType::LessEq;
                stop++;
            }
            else
            {
                type = TokenType::Less;
            }
            break;
        case U'>':
            if (second == U'=')
            {
                type = TokenType::MoreEq;
                stop++;
            }
            else
            {
                type = TokenType::More;
            }
            break;
        case U'!':
            if (second == U'=')
            {
                type = TokenType::NotEqual;
                stop++;
            }
            break;
        case U'"':
            return ReadStringLiteral(text, stop);
        case U'\'':
            return ReadRawcodeLiteral(text, stop);
        default:
            if (IsDigit(first))
            {
                return ReadNumber(text, start);
            }
            else if (IsIdentifierChar(first))
            {
                return ReadIdentifier(text, start);
            }
        }

        switch (type)
        {
        case TokenType::String:
        case TokenType::Rawcode:
            return Token(type, start, stop, text.middle(start, stop - start));
        default:
            return Token(type, start, stop);
        }
    }

    Vector<Token> Tokenize(StringView32 text, int start)
    {
        Vector<Token> tokens;
        tokens.reserve((text.size() - start) / 40);
        while (start < text.size())
        {
            tokens.push_back(NextToken(text, start));
            start = tokens.back().Stop();
        }
        return tokens;
    }

    int NextMeaningfullToken(Vector<Token> const& tokens, int idx)
    {
        for (;;)
        {
            if (idx == tokens.size())     return -1;
            if (!tokens[idx].IsComment()) return idx;

            idx++;
        }
    }

    HashMap<String32, int> Scrape(StringView32 text)
    {
        HashMap<String32, int> keywords;

        Vector<Token> tokens = Tokenize(text);

        keywords.reserve(tokens.size() / 20);

        int prev_idx = 0;
        do
        {
            int first_idx = NextMeaningfullToken(tokens, prev_idx);
            if (first_idx == -1) break;

            int second_idx = NextMeaningfullToken(tokens, first_idx + 1);
            if (second_idx == -1) break;

            Token & first  = tokens[first_idx];
            Token & second = tokens[second_idx];

            prev_idx = second_idx;

            if (second.Is(TokenType::Identifier))
            {
                int style;

                switch (first.Type())
                {
                case TokenType::Function:
                    style = JASS_FUNCTION;
                    break;
                case TokenType::Native:
                    style = JASS_NATIVE;
                    break;
                case TokenType::Type:
                case TokenType::Struct:
                    style = JASS_TYPE;
                    break;
                default:
                    continue;
                }

                keywords[std::move(second).Value()] = style;
                prev_idx++;
            }
        } while (prev_idx != text.size());

        return keywords;
    }
}
