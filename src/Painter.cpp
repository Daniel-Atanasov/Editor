#include "Painter.hpp"

#include <QFlag>
#include <QGuiApplication>
#include <QTextLayout>
#include <QTextOption>

#include <QDebug>

#include "Vector.hpp"

Painter::Painter(QWidget * widget) : painter(widget), clip_rect(painter.window())
{
}

QRect Painter::Rect()
{
    QRect r = clip_rect;
    r.moveTopLeft(QPoint(0, 0));
    return r;
}

QRect Painter::ClipRect()
{
    return clip_rect;
}

QPoint Painter::Origin()
{
    return clip_rect.topLeft();
}

int Painter::X()
{
    return Origin().x();
}

int Painter::Y()
{
    return Origin().y();
}

void Painter::SetClipRect(QRect r)
{
    clip_rect = r;
    painter.setClipRect(r);
}

void Painter::SetFont(QFont const& font)
{
    painter.setFont(font);
}

void Painter::SetPen(QPen const& pen)
{
    painter.setPen(pen);
}

void Painter::FillRect(QRect rect, QColor const& color)
{
    rect.moveTopLeft(Origin() + rect.topLeft());
    painter.fillRect(rect, color);
}

void Painter::DrawRect(QRect rect)
{
    rect.moveTopLeft(Origin() + rect.topLeft());
    painter.drawRect(rect);
}

void Painter::DrawLine(int x, int y, int w, int h)
{
    DrawLine(QPoint(x, y), QPoint(x + w, y + h));
}

void Painter::DrawLine(QPoint const& start, QPoint const& stop)
{
    painter.drawLine(start + Origin(), stop + Origin());
}

void Painter::DrawCharacter(QRect rect, char32_t ch)
{
    int flags = Qt::AlignHCenter | Qt::AlignVCenter;

    rect.moveTopLeft(Origin() + rect.topLeft());
    painter.drawText(rect, flags, QString::fromUcs4(&ch, 1));
}

void Painter::DrawText(QRect rect, StringView32 text, int flags)
{
    rect.moveTopLeft(Origin() + rect.topLeft());
    painter.drawText(rect, flags, QString::fromUcs4(text.data(), text.size()));

    //int width = rect.width();

    //double radius = (double)width / (double)text.size();
    //double half_radius = radius / 2.0;
    //double quarter_radius = half_radius / 2.0;

    //QPainterPath path;

    //double x = half_radius;
    //for (int i = 0; i < text.size(); i++)
    //{
    //    double x1 = x - quarter_radius;
    //    double y1 = -half_radius;

    //    double x2 = x + quarter_radius;
    //    double y2 = half_radius;

    //    path.quadTo(x1, y1, x, 0);
    //    path.quadTo(x2, y2, x + half_radius, 0);

    //    x += radius;
    //}

    //QPixmap pixmap(width, radius);
    //pixmap.fill(Qt::transparent);
    //{
    //    QPen wavePen = painter.pen();
    //    wavePen.setCapStyle(Qt::RoundCap);

    //    QPainter imgPainter(&pixmap);
    //    imgPainter.setPen(wavePen);
    //    imgPainter.setRenderHint(QPainter::Antialiasing);
    //    imgPainter.translate(0, half_radius);
    //    imgPainter.drawPath(path);
    //}

    //rect.moveTop(rect.top() + (rect.height() * 3) / 4);
    //painter.drawPixmap(rect.topLeft(), pixmap);
}