#include "LexerJass.hpp"

#include "Buffer.hpp"
#include "Theme.hpp"

#include "SpecialCharacters.hpp"

#include <QDebug>

using namespace Jass;

void LexerJass::StyleToken(Token const& token, int start)
{
	int style = STYLE_DEFAULT;

	switch (token.Type())
	{
	case Jass::TokenType::CommentLine:
		style = STYLE_COMMENT_LINE;
		break;
	case Jass::TokenType::PreprocessorComment:
		style = STYLE_COMMENT_PREPROCESSOR;
		break;
	case Jass::TokenType::CommentBlock:
		// TODO@Daniel:
		// Style docstring parameters

		style = STYLE_COMMENT_BLOCK;
		break;
	case Jass::TokenType::String:
		for (Jass::Token const& sequence : token.Tokens())
		{
			int style = STYLE_DOUBLE_QUOTE_ESCAPE_VALID;
			if (sequence.Is(Jass::TokenType::InvalidEscapeSequence))
			{
				style = STYLE_DOUBLE_QUOTE_ESCAPE_INVALID;
			}
			Parent()->SetStyle(sequence.Start() - start, STYLE_DOUBLE_QUOTE_STRING);
			Parent()->SetStyle(sequence.Length(), style);

			start = sequence.Stop();
		}

		style = STYLE_DOUBLE_QUOTE_STRING;
		break;
	case Jass::TokenType::Rawcode:
		style = STYLE_SINGLE_QUOTE_STRING;
		break;
	case Jass::TokenType::Number:
		style = STYLE_NUMBER;
		break;
	case Jass::TokenType::Identifier:
		if (keywords.contains(token.Value()))
		{
			style = keywords[token.Value()];
		}
		break;
	default:
		if (token.IsKeyword())
		{
			style = STYLE_KEYWORD;
		}
		else if (token.Is(Jass::TokenType::Nothing, Jass::TokenType::Array))
		{
			style = JASS_TYPE;
		}
		break;
	}

	Parent()->SetStyle(token.Stop() - start, style);
}

void LexerJass::Style(Position start, Position stop)
{
	String32 text = Parent()->Text(start, stop);

	int starting_style = STYLE_DEFAULT;
	if (start != Position{ 0,0 })
	{
		starting_style = Parent()->StyleAt(Parent()->PrevPosition(start));
	}

	Jass::Token last_token;

	int idx = 0;
	switch (starting_style)
	{
	case STYLE_COMMENT_BLOCK:
		last_token = Jass::ReadCommentBlock(text, idx, false);
		StyleToken(last_token, 0);
		idx = last_token.Stop();
		break;
	case STYLE_DOUBLE_QUOTE_STRING:
		last_token = Jass::ReadStringLiteral(text, idx, false);
		StyleToken(last_token, 0);
		idx = last_token.Stop();
		break;
	case STYLE_SINGLE_QUOTE_STRING:
		last_token = Jass::ReadStringLiteral(text, idx, false);
		StyleToken(last_token, 0);
		idx = last_token.Stop();
		break;
	default:
		break;
	}

	for (Jass::Token const& token : Jass::Tokenize(text, idx))
	{
		StyleToken(token, idx);
		idx = token.Stop();
	}

	Parent()->SetStyle(text.size() - idx, STYLE_DEFAULT);
}
