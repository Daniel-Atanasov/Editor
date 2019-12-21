#ifndef SPECIALCHARACTERS_HPP
#define SPECIALCHARACTERS_HPP

enum class Spaces
{
    CharacterTabulation     = 9,
    LineFeed                = 10,
    LineTabulation          = 11,
    FormFeed                = 12,
    CarriageReturn          = 13,
    Space                   = 32,
    NextLine                = 133,
    NoBreakSpace            = 160,
    OghamSpaceMark          = 5760,
    EnQuad                  = 8192,
    EmQuad                  = 8193,
    EnSpace                 = 8194,
    EmSpace                 = 8195,
    ThreePerEmSpace         = 8196,
    FourPerEmSpace          = 8197,
    SixPerEmSpace           = 8198,
    FigureSpace             = 8199,
    PunctuationSpace        = 8200,
    ThinSpace               = 8201,
    HairSpace               = 8202,
    LineSeparator           = 8232,
    ParagraphSeparator      = 8233,
    NarrowNoBreakSpace      = 8239,
    MediumMathematicalSpace = 8287,
    IdeographicSpace        = 12288
};

enum class LineBreaks
{
    LineFeed           = 10,
    LineTabulation     = 11,
    FormFeed           = 12,
    CarriageReturn     = 13,
    NextLine           = 133,
    LineSeparator      = 8232,
    ParagraphSeparator = 8233
};

bool IsLineBreak(char32_t ch);
bool IsSpace(char32_t ch);
bool IsDigit(char32_t ch);
bool IsAsciiLetter(char32_t ch);
bool IsIdentifierChar(char32_t ch);

#endif // SPECIALCHARACTERS_HPP
