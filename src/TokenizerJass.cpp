#include "TokenizerJass.hpp"

#include "SpecialCharacters.hpp"

#include <unordered_map>

#include <QDebug>

namespace Jass
{
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

    Token ReadCommentBlock(String32 const& text, int start, bool add_offset)
    {
        int stop = start;

        Vector<Token> tokens; // TODO@Daniel: Find a good way to implement doc params
        for (;;)
        {
            if (stop > text.size())
            {
                stop = text.size();
                break;
            }
            if (text[stop] == '*' && text[stop + 1] == '/')
            {
                stop += 2;
                break;
            }

            stop++;
        }

        return Token(TokenType::CommentBlock, start - add_offset * 2, stop, std::move(tokens));
    }

    Token ReadStringLiteral(String32 const& text, int start, bool add_offset)
    {
        static String32 slash_escapes = U"\\nt\"";
        static String32 pipe_escapes = U"cnr";

        int stop = start;

        Vector<Token> tokens;
        for (;;)
        {
            if (stop + 2 > text.size())
            {
                stop = text.size();
                break;
            }
            if (text[stop] == U'"' || text[stop] == U'\0')
            {
                stop++;
                break;
            }
            if (text[stop] == U'\\')
            {
                char32_t next = text[stop + 1];
                if (next == U'\0')
                {
                    stop += 2;
                    break;
                }

                TokenType type;
                if (slash_escapes.contains(next))
                {
                    type = TokenType::ValidEscapeSequence;
                }
                else
                {
                    type = TokenType::InvalidEscapeSequence;
                }

                tokens.emplace_back(type, stop, stop + 2);
                stop++;
            }
            if (text[stop] == U'|')
            {
                if (pipe_escapes.contains(text[stop + 1]))
                {
                    tokens.emplace_back(TokenType::ValidEscapeSequence, stop, stop + 2);
                    stop++;
                }
            }
            stop++;
        }

        return Token(TokenType::String, start - add_offset, stop, std::move(tokens));
    }

    Token ReadRawcodeLiteral(String32 const& text, int start, bool add_offset)
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

    Token ReadNumber(String32 const& text, int idx)
    {
        int start = idx;
        int stop = start + 1;
        while (stop < text.size() && IsDigit(text[stop])) stop++;

        String32 value = text.substr(start, stop - start);

        return Token(TokenType::Number, start, stop, std::move(value));
    }

    Token ReadIdentifier(String32 const& text, int idx, bool ignore_keywords)
    {
        int start = idx;
        int stop = start + 1;
        while (stop < text.size() && IsIdentifierChar(text[stop])) stop++;

        TokenType type = TokenType::Identifier;

        String32 value = text.substr(start, stop - start);

        if (!ignore_keywords)
        {
            std::unordered_map<String32, TokenType> keywords
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

            auto it = keywords.find(value);
            if (it != std::end(keywords)) type = it->second;
        }

        return Token(type, start, stop, std::move(value));
    }

    Token NextToken(String32 const& text, int start)
    {
        auto at = [&text](int idx) -> char32_t
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
                return Token(type, start, stop, text.substr(start, stop - start));
            default:
                return Token(type, start, stop);
        }
    }

    Vector<Token> Tokenize(String32 const& text, int start)
    {
        Vector<Token> tokens;
        while (start < text.size())
        {
            tokens.push_back(NextToken(text, start));
            start = tokens.back().Stop();
        }
        return tokens;
    }

    HashSet<String32> ScrapeFunctions(String32 const & text)
    {
        HashSet<String32> functions;

        Vector<Token> tokens = Tokenize(text);
        for (int idx = 0; idx < tokens.size() - 1; idx++)
        {
            Token & a = tokens[idx];
            Token & b = tokens[idx + 1];

            if (a.Is(TokenType::Function))
            {
                functions.insert(std::move(b).Value());
            }
        }

        return functions;
    }
}
