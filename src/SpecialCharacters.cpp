#include "SpecialCharacters.hpp"

bool IsLineBreak(char32_t ch)
{
    switch ((Spaces)ch)
    {
    case Spaces::LineFeed:
    case Spaces::LineTabulation:
    case Spaces::FormFeed:
    case Spaces::CarriageReturn:
    case Spaces::NextLine:
    case Spaces::LineSeparator:
    case Spaces::ParagraphSeparator:
        return true;
    default:
        return false;
    }
}

bool IsSpace(char32_t ch)
{
    switch ((Spaces)ch)
    {
    case Spaces::CharacterTabulation:
    case Spaces::LineFeed:
    case Spaces::LineTabulation:
    case Spaces::FormFeed:
    case Spaces::CarriageReturn:
    case Spaces::Space:
    case Spaces::NextLine:
    case Spaces::NoBreakSpace:
    case Spaces::OghamSpaceMark:
    case Spaces::EnQuad:
    case Spaces::EmQuad:
    case Spaces::EnSpace:
    case Spaces::EmSpace:
    case Spaces::ThreePerEmSpace:
    case Spaces::FourPerEmSpace:
    case Spaces::SixPerEmSpace:
    case Spaces::FigureSpace:
    case Spaces::PunctuationSpace:
    case Spaces::ThinSpace:
    case Spaces::HairSpace:
    case Spaces::LineSeparator:
    case Spaces::ParagraphSeparator:
    case Spaces::NarrowNoBreakSpace:
    case Spaces::MediumMathematicalSpace:
    case Spaces::IdeographicSpace:
        return true;
    default:
        return false;
    }
}

bool IsDigit(char32_t ch)
{
    return ch >= '0' && ch <= '9';
}

bool IsAsciiLetter(char32_t ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool IsIdentifierChar(char32_t ch)
{
    return IsDigit(ch) || IsAsciiLetter(ch) || ch == '_';
}

int TabWidth(int pos)
{
    return 4 - (pos & 0b11);
}
