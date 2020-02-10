#ifndef PAINTER_HPP
#define PAINTER_HPP

#include <QPaintDevice>

#include <QPainter>
#include <QFont>
#include <QPen>

#include "String32.hpp"
#include "StringView32.hpp"

class Painter
{
private:
    QPainter painter;

    QTextOption text_option;

    QRect clip_rect;

public:
    Painter(QPaintDevice * widget);

    QRect Rect();
    QRect ClipRect();
    QPoint Origin();
    int X();
    int Y();

    void SetClipRect(QRect r);
    void SetFont(QFont const& font);
    void SetPen(QPen const& pen);

    void FillRect(QRect rect, QColor const& color);
    void DrawRect(QRect rect);
    void DrawLine(int x, int y, int w, int h);
    void DrawLine(QPoint const& start, QPoint const& stop);
    void DrawCharacter(QRect rect, char32_t ch);
    void DrawText(QRect rect, StringView32 text, int flags = Qt::AlignHCenter | Qt::AlignVCenter);
    void DrawText(int x, int y, int w, int h, StringView32 text);
};

#endif // PAINTER_HPP
