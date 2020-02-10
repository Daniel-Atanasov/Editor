#include "BufferWidget.hpp"

#include <QDebug>

#include <QApplication>
#include <QPixmap>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>

#include "Painter.hpp"
#include "Clipboard.hpp"

int BufferWidget::CellWidth()
{
    return dummyWidget->size().width() / buffer.CellSize().width();
}

int BufferWidget::CellHeight()
{
    return dummyWidget->size().height() / buffer.CellSize().height();
}

QSize BufferWidget::CellArea()
{
    return QSize(CellWidth(), CellHeight());
}

int BufferWidget::FirstVisibleLine()
{
    return scrollBarVertical->value();
}

int BufferWidget::LastVisibleLine()
{
    return std::min(FirstVisibleLine() + CellHeight(), buffer.LineCount() - 1);
}

Position BufferWidget::FirstVisiblePosition()
{
    Position pos;
    pos.y = FirstVisibleLine();
    pos.x = 0;
    return pos;
}

Position BufferWidget::LastVisiblePosition()
{
    Position pos;
    pos.y = LastVisibleLine();
    pos.x = std::min(CellWidth(), buffer.LineLength(pos.y));
    return pos;
}

void BufferWidget::EnsureCursorIsVisible()
{
    if (buffer.CursorCount() == 1)
    {

    }
}

void BufferWidget::EnsureVisibleAreaIsStyled()
{
    buffer.EnsureStyled(LastVisibleLine());
}

int BufferWidget::VScroll()
{
    return scrollBarVertical->value();
}

void BufferWidget::SetVScroll(int value)
{
    scrollBarVertical->setValue(value);
}

void BufferWidget::ScrollUp(int amount)
{
    SetVScroll(VScroll() - amount);
}

void BufferWidget::ScrollDown(int amount)
{
    SetVScroll(VScroll() + amount);
    EnsureVisibleAreaIsStyled();
}

Position BufferWidget::ScreenToCell(QPoint pt, bool round)
{
    QSize cs = buffer.CellSize();

    int mw = buffer.LineNumberMarginWidth();

    int py = pt.y();
    int px = pt.x() - mw;

    int sy = VScroll() + py / cs.height();
    int sx = px / cs.width();

    Position pos;
    pos.y = std::min(sy, buffer.LineCount() - 1);

    StringView32 line = buffer.LineAt(pos.y);
    pos.x = line.from_tab_adjusted(sx);

    if (round && pos.x < line.size())
    {
        int start = line.adjust_for_tabs(pos.x);
        char32_t ch = line[pos.x];

        int w = 1;
        if (ch == U'\t') w = TabWidth(start);

        int rem = px - start * cs.width();
        if (2 * rem > w * cs.width()) pos.x++;
    }

    return buffer.ClampPosition(pos);
}

void BufferWidget::UpdateScrollbar()
{
    int sy = buffer.LineCount();
    int sx = buffer.MaximumTabAdjustedLineLength();

    int cw = CellWidth();
    int ch = CellHeight();

    scrollBarVertical->setMinimum(0);
    scrollBarVertical->setMaximum(sy - 1);

    scrollBarHorizontal->setMinimum(0);
    scrollBarHorizontal->setMaximum(sx - 1);

    bool hshow = sx <= cw;
    bool vshow = sy <= ch;

    scrollBarHorizontal->setHidden(hshow);
    scrollBarVertical->setHidden(vshow);
}

BufferWidget::BufferWidget(QWidget * parent) : QWidget(parent), timer(this)
{
    setupUi(this);

    timer.setSingleShot(true);
    timer.setInterval(250);

    connect(&timer, &QTimer::timeout,
        [this](...)
        {
            qDebug() << "Updating lexer...";

            lexer.ClearKeywords();

            lexer.SetKeywordStyle(Jass::Scrape(buffer.Text()));

            lexer.SetKeywordStyle(U"integer", JASS_TYPE);
            lexer.SetKeywordStyle(U"real", JASS_TYPE);
            lexer.SetKeywordStyle(U"boolean", JASS_TYPE);
            lexer.SetKeywordStyle(U"string", JASS_TYPE);
            lexer.SetKeywordStyle(U"code", JASS_TYPE);
            lexer.SetKeywordStyle(U"handle", JASS_TYPE);

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

    keymap[Control & Qt::Key_Up]   = [this] { ScrollUp();   };
    keymap[Control & Qt::Key_Down] = [this] { ScrollDown(); };

    keymap[Qt::Key_Home] = [this] { buffer.CursorMoveToLineStart(); };
    keymap[Qt::Key_End]  = [this] { buffer.CursorMoveToLineEnd();   };

    keymap[Shift & Qt::Key_Home] = [this] { buffer.CursorAdjustToLineStart(); };
    keymap[Shift & Qt::Key_End]  = [this] { buffer.CursorAdjustToLineEnd();   };

    keymap[Control & Shift & Qt::Key_Home] = [this] { buffer.CursorAdjustToBufferStart(); };
    keymap[Control & Shift & Qt::Key_End]  = [this] { buffer.CursorAdjustToBufferEnd();   };

    keymap[Control & Qt::Key_Home] = [this] { buffer.CursorMoveToBufferStart(); };
    keymap[Control & Qt::Key_End]  = [this] { buffer.CursorMoveToBufferEnd();   };

    keymap[Control & Qt::Key_A] = [this] { buffer.CursorSelectAll(); };

    keymap[Qt::Key_Return] = [this] { return buffer.CursorInsertText(U"\n"); };
    keymap[Qt::Key_Enter]  = [this] { return buffer.CursorInsertText(U"\n"); };

    keymap[Control & Qt::Key_Return] = [this] { return buffer.CursorInsertText(U"\n"); };
    keymap[Control & Qt::Key_Enter]  = [this] { return buffer.CursorInsertText(U"\n"); };

    keymap[Qt::Key_Tab] = [this] { return buffer.CursorInsertText(U"\t"); };

    keymap[Qt::Key_Backspace] = [this] { return buffer.CursorDeletePrev(); };
    keymap[Qt::Key_Delete]    = [this] { return buffer.CursorDeleteNext(); };

    keymap[Control & Qt::Key_Backspace] = [this] { return buffer.CursorDeleteToPrevBorder(); };
    keymap[Control & Qt::Key_Delete]    = [this] { return buffer.CursorDeleteToNextBorder(); };

    keymap[Control & Qt::Key_C] = [this] { buffer.DoCopy();         };
    keymap[Control & Qt::Key_V] = [this] { return buffer.DoPaste(); };
    keymap[Control & Qt::Key_X] = [this] { return buffer.DoCut();   };

    keymap[Control & Alt & Qt::Key_L] = [this] { return buffer.ConvertTabsToSpaces(); };

    UpdateScrollbar();

    buffer.SetLexer(&lexer);
}

void BufferWidget::mousePressEvent(QMouseEvent * event)
{
    setFocus();

    Position pos = ScreenToCell(event->pos());

    if (pos.y < buffer.LineCount())
    {
        Qt::KeyboardModifiers modifiers = event->modifiers();

        bool control = modifiers & Qt::ControlModifier;
        bool shift = modifiers & Qt::ShiftModifier;

        if (shift)
        {
            Cursor fc = buffer.FirstCursor();
            Cursor lc = buffer.LastCursor();

            buffer.ClearCursors();

            Cursor cursor;
            cursor.start = std::min({ pos, fc.start, fc.stop });
            cursor.stop  = std::max({ pos, lc.start, lc.stop });
            buffer.AddCursor(cursor);
        }
        else
        {
            if (!control) buffer.ClearCursors();

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

    if (keymap.contains(hotkey))
    {
        int changes = keymap[hotkey]();

        if (changes != 0) timer.start();
    }
    else if (!text.empty())
    {
        buffer.CursorInsertText(text);
        timer.start();
    }

    EnsureVisibleAreaIsStyled();

    buffer.ConsolidateCursors();

    {
        UpdateScrollbar();
    }

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
