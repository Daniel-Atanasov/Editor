#include "Painter.hpp"

#include <QDebug>
#include <QTime>

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

void Painter::DrawText(QRect rect, String32 const& text, int flags)
{
    rect.moveTopLeft(Origin() + rect.topLeft());
    painter.drawText(rect, flags, QString::fromStdU32String(text));
}

//void Painter::DrawText(QRect rect, String32 const& text, int flags)
//{
////#ifdef QT_DEBUG_DRAW
////    if (qt_show_painter_debug_output)
////        printf("QPainter::drawText(), r=[%d,%d,%d,%d], flags=%d, str='%s'\n",
////            r.x(), r.y(), r.width(), r.height(), flags, str.toLatin1().constData());
////#endif
//    if (!painter.paintEngine() || text.empty || painter.pen().style() == Qt::NoPen)
//        return;
//
//    qt_format_text(d->state->font, rect, flags, 0, 0, 0, 0, 0, this);
//}

void Painter::DrawText(QRect rect, char32_t * text, int size, int flags)
{
    rect.moveTopLeft(Origin() + rect.topLeft());
    painter.drawText(rect, flags, QString::fromUcs4(text, size));
}
