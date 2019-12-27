#ifndef TOKENIZERJASS_HPP
#define TOKENIZERJASS_HPP

#include "String32.hpp"
#include "Vector.hpp"
#include "HashMap.hpp"

namespace Jass {
    // NOTE@Daniel:
    //  A lot of these aren't valid jass/vjass but will be useful in the future
    //  Basic error reporting for starters
    enum class TokenType : char32_t {
        Number,
        Identifier,
        Keyword,
        CommentLine,
        PreprocessorComment,
        CommentBlock,

        OpenParen,
        CloseParen,
        OpenBracket,
        CloseBracket,
        OpenBrace,
        CloseBrace,

        Comma,
        Dot,

        // Arithmetic operators
        Add,
        Sub,
        Mul,
        Div,

        // Assignment operators
        Assign,
        AssignAdd,
        AssignSub,
        AssignMul,
        AssignDiv,

        // Comparison operators
        Less,
        LessEq,
        More,
        MoreEq,
        Equal,
        NotEqual,

        String,
        Rawcode,

        // Only valid in strings
        ValidEscapeSequence,
        InvalidEscapeSequence,

        // Only valid in block comments
        DocParam,

        // Keywords
        Globals,
        EndGlobals,
        Type,
        Extends,
        Native,
        Takes,
        Returns,
        Function,
        EndFunction,
        Method,
        EndMethod,
        Operator,
        Struct,
        EndStruct,
        Interface,
        EndInterface,
        Module,
        EndModule,
        Implement,
        Scope,
        EndScope,
        Initializer,
        Library,
        EndLibrary,
        Requires,
        Uses,
        Needs,
        Loop,
        ExitWhen,
        EndLoop,
        If,
        Then,
        ElseIf,
        Else,
        EndIf,
        And,
        Or,
        Not,
        Private,
        Public,
        Static,
        Constant,
        Local,
        Set,
        Call,
        Return,

        // Special types
        Nothing,
        Array,

        // Keyword constants
        Null,
        True,
        False,

        Unknown,
        Eof,

        KeywordFirst = Globals,
        KeywordLast = Return
    };

    class Token
    {
    private:
        TokenType type;
        int start;
        int stop;

        String32 value;

        // For escape sequences in strings, parameters in docstring and similar
        Vector<Token> tokens;

    public:
        Token() = default;
        Token(TokenType type, int start, int stop, String32 value = {}, Vector<Token> tokens = {});
        Token(TokenType type, int start, int stop, Vector<Token> tokens);

        TokenType Type() const;

        int Start() const;
        int Stop() const;
        int Length() const;

        bool Is(TokenType token_type) const;
        template <typename T, typename ... Ts>
        bool Is(T first, Ts ... other) const
        {
            return Is(first) || Is(other...);
        }
        bool IsComment() const;
        bool IsKeyword() const;

        String32 const& Value() const&;
        String32 Value() &&;

        Vector<Token> const& Tokens() const&;
        Vector<Token> Tokens() &&;
    };

    Token ReadCommentBlock(StringView32 text, int start, bool add_offset = true);
    Token ReadStringLiteral(StringView32 text, int start, bool add_offset = true);
    Token ReadRawcodeLiteral(StringView32 text, int start, bool add_offset = true);

    Token ReadNumber(StringView32 text, int idx);
    Token ReadIdentifier(StringView32 text, int idx, bool ignore_keywords = false);
    Token NextToken(StringView32 text, int start = 0);

    Vector<Token> Tokenize(StringView32 text, int start = 0);

    int NextMeaningfullToken(Vector<Token> const& tokens, int idx = 0);

    HashMap<String32, int> Scrape(StringView32 text);
}

#endif // TOKENIZERJASS_HPP
