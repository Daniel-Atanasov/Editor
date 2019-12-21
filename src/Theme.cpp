#include "Theme.hpp"

#include "Lexer.hpp"
#include "LexerJass.hpp"

Style & Theme::operator[](int idx)
{
    return styles[idx];
}

Style const& Theme::operator[](int idx) const
{
    return styles[idx];
}

ThemeManager::ThemeManager()
{
    Theme theme;
    theme[STYLE_DEFAULT].forecolor = QColor(255, 255, 255);
    theme[STYLE_KEYWORD].forecolor = QColor(56, 156, 214) ;
    theme[STYLE_NUMBER].forecolor = QColor(189, 99, 197);
    theme[STYLE_DOUBLE_QUOTE_STRING].forecolor = QColor(214, 157, 133);
    theme[STYLE_SINGLE_QUOTE_STRING].forecolor = QColor(214, 157, 133);
    theme[STYLE_DOUBLE_QUOTE_ESCAPE_VALID].forecolor = QColor(255, 204, 0);
    theme[STYLE_COMMENT_LINE].forecolor = QColor(87, 166, 74)  ;
    theme[STYLE_COMMENT_BLOCK].forecolor = QColor(87, 166, 74)  ;
    theme[STYLE_COMMENT_PREPROCESSOR].forecolor = QColor(155, 155, 155);

    theme[JASS_NATIVE].forecolor = QColor(189, 99, 197) ;
    theme[JASS_FUNCTION].forecolor = QColor(200, 100, 100);
    theme[JASS_CONSTANT].forecolor = QColor(184, 215, 163);
    theme[JASS_TYPE].forecolor = QColor(78, 201, 176) ;

//    setColor(QColor(181, 206, 168), JASS_NUMBER); // numbers
//	setColor(QColor(56, 156, 214)  , JASS_KEYWORD); // keywords
//	setColor(QColor(214, 157, 133) , JASS_STRING); // string
//	setColor(QColor(255, 204, 0)   , JASS_ESCAPE_SEQUENCE); // character escape sequences
//	setColor(QColor(214, 157, 133) , JASS_RAWCODE); // rawcode
//	setColor(QColor(87, 166, 74)   , JASS_COMMENT_LINE); // comment
//	setColor(QColor(155, 155, 155) , JASS_PREPROCESSOR_COMMENT); // preprocessor comment
//	setColor(QColor(87, 166, 74)   , JASS_COMMENT_BLOCK); // comment
//	setColor(QColor(189, 99, 197)  , JASS_NATIVE); // native
//	setColor(QColor(200, 100, 100) , JASS_FUNCTION); // function
//	setColor(QColor(184, 215, 163) , JASS_CONSTANT); // constant
//	setColor(QColor(78, 201, 176)  , JASS_TYPE); // type

    themes.push_back(theme);

    current_theme = 0;
}

ThemeManager & ThemeManager::Instance()
{
    static ThemeManager manager;
    return manager;
}

Theme & ThemeManager::CurrentTheme()
{
    return themes[current_theme];
}

void ThemeManager::LoadTheme(String32 const& filename)
{

}
