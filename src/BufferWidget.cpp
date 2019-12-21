#include "BufferWidget.hpp"

#include <QDebug>

#include <QApplication>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>

#include "Painter.hpp"

QSize BufferWidget::VisibleCellArea()
{
    QSize const& cell_size = buffer.CellSize();

    return QSize(size().width() / cell_size.width(), size().height() / cell_size.height());
}

int BufferWidget::LastVisibleLine()
{
    return std::min(scrollBarVertical->value() + VisibleCellArea().height(), buffer.LineCount() - 1);
}

void BufferWidget::EnsureCursorIsVisible()
{
    //    Position pos = buffer
}

void BufferWidget::EnsureVisibleAreaIsStyled()
{
    buffer.EnsureStyled(LastVisibleLine());
}

void BufferWidget::ScrollUp(int amount)
{
    scrollBarVertical->setValue(scrollBarVertical->value() - amount);
}

void BufferWidget::ScrollDown(int amount)
{
    scrollBarVertical->setValue(scrollBarVertical->value() + amount);
    EnsureVisibleAreaIsStyled();
}

Position BufferWidget::ScreenToCell(QPoint pt, bool round)
{
    QSize const& cell_size = buffer.CellSize();

    int margin_width = buffer.LineNumberMarginWidth();

    Position pos;
    pos.x = (pt.x() - margin_width + round * cell_size.width() / 2) / cell_size.width();
    pos.y = pt.y() / cell_size.height() + scrollBarVertical->value();
    return pos;
}

void BufferWidget::UpdateScrollbar()
{
    QSize visible_area = VisibleCellArea();

    int lines = buffer.LineCount();
    int max_size = buffer.MaximumLineLength();

    scrollBarVertical->setMinimum(0);
    scrollBarVertical->setMaximum(lines - 1);

    scrollBarHorizontal->setMinimum(0);
    scrollBarHorizontal->setMaximum(max_size - 2);

    QSize size = VisibleCellArea();

    bool vshow = lines <= size.height();
    bool hshow = max_size <= size.width();

    scrollBarVertical->setHidden(false);
    scrollBarHorizontal->setHidden(false);
}

BufferWidget::BufferWidget(QWidget * parent) : QWidget(parent), timer(this)
{
    setupUi(this);

    timer.setSingleShot(true);
    timer.setInterval(250);

    connect(&timer, &QTimer::timeout,
        [this](...)
        {
            lexer.ClearKeywords();

            for (String32 const& identifier : Jass::ScrapeFunctions(buffer.Text()))
            {
                lexer.SetKeywordStyle(identifier, JASS_FUNCTION);
            }

            buffer.InvalidateStyles();
            EnsureVisibleAreaIsStyled();
            update();
        }
    );

    connect(scrollBarVertical, &QScrollBar::valueChanged,
        [this](...)
        {
            EnsureVisibleAreaIsStyled();
            update();
        }
    );

    keymap[Qt::Key_Left]  = [this] { buffer.CursorMoveLeft();  };
    keymap[Qt::Key_Up]    = [this] { buffer.CursorMoveUp();    };
    keymap[Qt::Key_Right] = [this] { buffer.CursorMoveRight(); };
    keymap[Qt::Key_Down]  = [this] { buffer.CursorMoveDown();  };

    keymap[Shift & Qt::Key_Left]  = [this] { buffer.CursorAdjustLeft();  };
    keymap[Shift & Qt::Key_Up]    = [this] { buffer.CursorAdjustUp();    };
    keymap[Shift & Qt::Key_Right] = [this] { buffer.CursorAdjustRight(); };
    keymap[Shift & Qt::Key_Down]  = [this] { buffer.CursorAdjustDown();  };

    keymap[Control & Qt::Key_Left]  = [this] { buffer.CursorMoveToPrevBorder(); };
    keymap[Control & Qt::Key_Right] = [this] { buffer.CursorMoveToNextBorder(); };

    keymap[Control & Shift & Qt::Key_Left]  = [this] { buffer.CursorAdjustToPrevBorder(); };
    keymap[Control & Shift & Qt::Key_Right] = [this] { buffer.CursorAdjustToNextBorder(); };

    keymap[Alt & Qt::Key_Up]   = [this] { buffer.CursorCloneUp();   };
    keymap[Alt & Qt::Key_Down] = [this] { buffer.CursorCloneDown(); };

    keymap[Qt::Key_Home] = [this] { buffer.CursorMoveToLineStart(); };
    keymap[Qt::Key_End]  = [this] { buffer.CursorMoveToLineEnd();   };

    keymap[Shift & Qt::Key_Home] = [this] { buffer.CursorAdjustToLineStart(); };
    keymap[Shift & Qt::Key_End]  = [this] { buffer.CursorAdjustToLineEnd();   };

    keymap[Control & Shift & Qt::Key_Home] = [this] { buffer.CursorAdjustToBufferStart(); };
    keymap[Control & Shift & Qt::Key_End]  = [this] { buffer.CursorAdjustToBufferEnd();   };

    keymap[Control & Qt::Key_Home] = [this] { buffer.CursorMoveToBufferStart(); };
    keymap[Control & Qt::Key_End]  = [this] { buffer.CursorMoveToBufferEnd();   };

    keymap[Control & Qt::Key_A] = [this] { buffer.CursorSelectAll(); };

    keymap[Qt::Key_Return] = [this] { buffer.CursorInsertText(U"\n"); };
    keymap[Qt::Key_Enter]  = [this] { buffer.CursorInsertText(U"\n"); };

    keymap[Qt::Key_Tab] = [this] { buffer.CursorInsertText(U"\t"); };

    keymap[Qt::Key_Backspace] = [this] { buffer.CursorDeletePrev(); };
    keymap[Qt::Key_Delete]    = [this] { buffer.CursorDeleteNext(); };

    keymap[Control & Qt::Key_Backspace] = [this] { buffer.CursorDeleteToPrevBorder(); };
    keymap[Control & Qt::Key_Delete]    = [this] { buffer.CursorDeleteToNextBorder(); };

    UpdateScrollbar();

    lexer.SetKeywordStyle(U"integer", JASS_TYPE);
    lexer.SetKeywordStyle(U"real", JASS_TYPE);
    lexer.SetKeywordStyle(U"boolean", JASS_TYPE);
    lexer.SetKeywordStyle(U"string", JASS_TYPE);
    lexer.SetKeywordStyle(U"code", JASS_TYPE);
    lexer.SetKeywordStyle(U"handle", JASS_TYPE);

    buffer.SetLexer(&lexer);
}

void BufferWidget::mousePressEvent(QMouseEvent * event)
{
    setFocus();

    Position pos = ScreenToCell(event->pos());

    if (pos.y < buffer.LineCount())
    {
        Qt::KeyboardModifiers modifiers = event->modifiers();

        if (!(modifiers & Qt::ControlModifier)) buffer.ClearCursors();

        if (pos.x < 0)
        {
            buffer.AddLineSelection(pos.y);
        }
        else
        {
            Cursor cursor;
            cursor.start = cursor.stop = pos;
            buffer.AddCursor(cursor);
        }
    }

    update();
}

void BufferWidget::mouseReleaseEvent(QMouseEvent * event)
{

}

void BufferWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    setFocus();

    Position pos = ScreenToCell(event->pos(), false);

    if (pos.y < buffer.LineCount())
    {
        Qt::KeyboardModifiers modifiers = event->modifiers();

        if (!(modifiers & Qt::ControlModifier)) buffer.ClearCursors();

        if (pos.x < 0) buffer.AddLineSelection(pos.y);
        else           buffer.AddWordSelection(pos);
    }

    update();
}

void BufferWidget::mouseMoveEvent(QMouseEvent * event)
{

}

void BufferWidget::wheelEvent(QWheelEvent * event)
{
    setFocus();

    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

    int delta = event->angleDelta().y();
    delta = (delta > 0) - (delta < 0);
    if (modifiers & Qt::ControlModifier)
    {
        if (delta > 0) buffer.ZoomIn();
        if (delta < 0) buffer.ZoomOut();
    }
    else if (scrollBarVertical->isVisible())
    {
        if (delta > 0) ScrollUp(2);
        if (delta < 0) ScrollDown(2);
    }
    update();
}

void BufferWidget::keyPressEvent(QKeyEvent * event)
{
    setFocus();

    Qt::KeyboardModifiers modifiers = event->modifiers();

    bool control = modifiers & Qt::ControlModifier;
    bool shift = modifiers & Qt::ShiftModifier;
    bool alt = modifiers & Qt::AltModifier;

    int key = event->key();

    int line_count = buffer.LineCount();

    String32 text = event->text().toStdU32String();

    Hotkey hotkey(control, shift, alt, key);

    bool is_hotkey = false;
    if (keymap.count(hotkey))
    {
        is_hotkey = true;
        keymap[hotkey]();
    }
    else if (keymap.count(key))
    {
        is_hotkey = true;
        keymap[key]();
    }
    else if (!text.empty())
    {
        is_hotkey = false;
        buffer.CursorInsertText(text);
    }

    EnsureVisibleAreaIsStyled();

    buffer.ConsolidateCursors();

    if (buffer.LineCount() != line_count)
    {
        UpdateScrollbar();
    }

    timer.start();

    update();
}

void BufferWidget::paintEvent(QPaintEvent * event)
{
    Painter painter(this);
    buffer.Paint(painter, scrollBarVertical->value());

    if (!hasFocus())
    {
        QRect r = rect();
        painter.SetClipRect(r);
        painter.FillRect(r, QColor(0, 0, 0, 63));
    }
}

void BufferWidget::resizeEvent(QResizeEvent * event)
{
    UpdateScrollbar();
    EnsureVisibleAreaIsStyled();
}
