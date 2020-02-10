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
    theme[STYLE_DOUBLE_QUOTE_ESCAPE_VALID].forecolor = QColor(255, 204, 20);
    theme[STYLE_DOUBLE_QUOTE_ESCAPE_INVALID].forecolor = QColor(255, 154, 0);
    theme[STYLE_COMMENT_LINE].forecolor = QColor(87, 166, 74)  ;
    theme[STYLE_COMMENT_BLOCK].forecolor = QColor(87, 166, 74)  ;
    theme[STYLE_COMMENT_PREPROCESSOR].forecolor = QColor(155, 155, 155);

    theme[JASS_NATIVE].forecolor = QColor(189, 99, 197) ;
    theme[JASS_FUNCTION].forecolor = QColor(200, 100, 100);
    theme[JASS_CONSTANT].forecolor = QColor(184, 215, 163);
    theme[JASS_TYPE].forecolor = QColor(78, 201, 176) ;

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
