#ifndef STYLESHEET_HPP
#define STYLESHEET_HPP

#include <array>

#include <QColor>

#include "String32.hpp"
#include "Vector.hpp"

struct Style
{
    QColor forecolor;
};

class Theme
{
private:
    std::array<Style, std::numeric_limits<std::uint8_t>::max()> styles;

public:
    Theme() = default;

    Style & operator[](int idx);
    Style const& operator[](int idx) const;

    void Update();
};

class ThemeManager
{
private:
    Vector<Theme> themes;
    int current_theme;

    ThemeManager();

    ThemeManager(ThemeManager const& other) = delete;
    ThemeManager(ThemeManager && other) = delete;

    ThemeManager & operator=(ThemeManager const& other) = delete;
    ThemeManager & operator=(ThemeManager && other) = delete;

public:
    static ThemeManager & Instance();

    Theme & CurrentTheme();

    void LoadTheme(String32 const& filename);
};

#endif // STYLESHEET_HPP
