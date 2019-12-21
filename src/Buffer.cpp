#include "Buffer.hpp"

#include <algorithm>

#include <QDebug>
#include <QTime>

#include <QApplication>

#include <QFont>
#include <QFontMetrics>

#include "Clipboard.hpp"
#include "Theme.hpp"

#include "SpecialCharacters.hpp"

void Buffer::SetText(QString const& text)
{
    ClearCursors();

    lines.clear();
    styles.clear();

    lines.resize(1);
    styles.resize(1);

    styles[0].push_back(STYLE_DEFAULT);

    AddCursor({{0, 0}, {0, 0}});
    Insert(cursors[0], text.toStdU32String());
    cursors[0] = {{0, 0}, {0, 0}};
}

Position Buffer::FirstPosition()
{
    Position pos;
    pos.x = 0;
    pos.y = 0;
    return pos;
}

Position Buffer::LastPosition()
{
    Position pos;
    pos.y = LineCount() - 1;
    pos.x = LineLength(pos.y);
    return pos;
}

Position Buffer::DeleteAdjustedPosition(Position start, Position stop, Position pos)
{
    if (pos < start)                 return pos;
    if (pos >= start && stop >= pos) return start;

    if (stop.y == pos.y) pos.x -= stop.x - start.x;

    pos.y -= stop.y - start.y;
    return pos;
}

Position Buffer::NewlineAdjustedPosition(Position insertion_pos, Position pos)
{
    if (pos <= insertion_pos)     return pos;
    if (pos.y == insertion_pos.y) pos.x -= insertion_pos.x;
    pos.y++;
    return pos;
}

int Buffer::DistanceToPrevBorder(Position pos)
{
    int count = 0;
    do
    {
        pos = PrevPosition(pos);
        count++;
    }
    while (!IsOnBorder(pos));
    return count;
}

int Buffer::DistanceToNextBorder(Position pos)
{
    int count = 0;
    do
    {
        pos = NextPosition(pos);
        count++;
    }
    while (!IsOnBorder(pos));
    return count;
}

void Buffer::RemoveText(Cursor & cursor)
{
    Position start = cursor.start;
    Position stop  = cursor.stop;

    if (start > stop) std::swap(stop, start);

    start.x = std::min(start.x, LineLength(start.y));
    stop.x  = std::min(stop.x,  LineLength(stop.y));

    if (stop.y == start.y)
    {
        lines[start.y].remove(start.x, stop.x - start.x);
        styles[start.y].remove(start.x, stop.x - start.x);
    }
    else
    {
        lines[start.y].resize(start.x);
        lines[start.y] += lines[stop.y].middle(stop.x);

        lines.remove(start.y + 1, stop.y - start.y);

        styles[start.y].resize(start.x);
        styles[start.y] += styles[stop.y].middle(stop.x);

        styles.remove(start.y + 1, stop.y - start.y);
    }

    cursor.start = start;
    cursor.stop  = cursor.start;

    InvalidateStyles(cursor.start.y);

    for (Cursor & c : cursors)
    {
//        qDebug() << c.stop.x << c.stop.y;

        c.start = DeleteAdjustedPosition(start, stop, c.start);
        c.stop  = DeleteAdjustedPosition(start, stop, c.stop);

//        qDebug() << c.stop.x << c.stop.y;
    }
}

void Buffer::MoveLeft(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    int count = 1;
    if (modifiers & Qt::ControlModifier)
    {
        count = DistanceToPrevBorder(cursor.stop);
    }

    while(count--) cursor.stop = PrevPosition(cursor.stop);
}

void Buffer::MoveUp(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::AltModifier)
    {
        Cursor c;
        c.start = cursor.start;
        c.stop = cursor.stop;

        if (c.start.y != 0) c.start.y--;
        if (c.stop.y  != 0) c.stop.y--;

        AddCursor(c);
    }
    else if (cursor.stop.y != 0)
    {
        cursor.stop.y--;
    }
}

void Buffer::MoveRight(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    int count = 1;
    if (modifiers & Qt::ControlModifier)
    {
        count = DistanceToNextBorder(cursor.stop);
    }

    while(count--) cursor.stop = NextPosition(cursor.stop);
}

void Buffer::MoveDown(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::AltModifier)
    {
        Cursor c;
        c.start = cursor.start;
        c.stop = cursor.stop;

        if (c.start.y != LineCount() - 1) c.start.y++;
        if (c.stop.y  != LineCount() - 1) c.stop.y++;

        AddCursor(c);
    }
    else if (cursor.stop.y != LineCount() - 1)
    {
        cursor.stop.y++;
    }
}

void Buffer::MoveHome(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier)
    {
        cursor.stop.y = 0;
    }

    cursor.stop.x = 0;
}

void Buffer::MoveEnd(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier)
    {
        cursor.stop.y = std::max(0, LineCount() - 1);
    }

    cursor.stop.x = std::max(0, LineLength(cursor.stop.y));
}

void Buffer::SelectAllOld(Cursor & cursor, Qt::KeyboardModifiers)
{
    cursor.start.x = 0;
    cursor.start.y = 0;
    cursor.stop.x  = lines.back().size();
    cursor.stop.y  = std::max(0, LineCount() - 1);
}

void Buffer::DeleteNext(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    int count = (cursor.stop == cursor.start);
    if ((modifiers & Qt::ControlModifier) && count)
    {
        count = DistanceToPrevBorder(cursor.stop);
    }

    while (count--) MoveLeft(cursor);

    RemoveText(cursor);
}

void Buffer::DeletePrev(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    int count = (cursor.stop == cursor.start);
    if ((modifiers & Qt::ControlModifier) && count)
    {
        count = DistanceToNextBorder(cursor.stop);
    }

    while (count--) MoveRight(cursor);

    RemoveText(cursor);
}

void Buffer::InsertNewLine(Cursor & cursor, Qt::KeyboardModifiers modifiers)
{
    Position insertion_pos = cursor.stop;
    Position & pos = cursor.stop;

    int right = std::max(0, LineLength(pos.y) - pos.x);

    lines.insert(pos.y + 1, lines[pos.y].middle(pos.x));
    lines[pos.y].remove(pos.x, right);

    styles.insert(pos.y + 1, styles[pos.y].middle(pos.x));
    styles[pos.y].remove(pos.x, right + 1);

    int style;
    if (pos.x != 0) style = StyleAt(PrevPosition(pos));
    else            style = STYLE_DEFAULT;
    styles[pos.y].push_back(style);

    for (Cursor & c : cursors)
    {
        c.start = NewlineAdjustedPosition(insertion_pos, c.start);
        c.stop  = NewlineAdjustedPosition(insertion_pos, c.stop);
    }

    if (!(modifiers & Qt::ControlModifier)) MoveRight(cursor);
}

void Buffer::Insert(Cursor & cursor, char32_t ch, Qt::KeyboardModifiers)
{
    if (IsLineBreak(ch))
    {
        InsertNewLine(cursor);
    }
    else
    {
        Position & start = cursor.start;
        Position & stop = cursor.stop;

        stop.x = std::min(stop.x, LineLength(stop.y));

        int style;
        if (stop.x != 0) style = StyleAt(PrevPosition(stop));
        else             style = STYLE_DEFAULT;

        int count = 1;
        if (ch == '\t')
        {
            count = 4 - stop.x % 4;
            lines[stop.y].insert(stop.x, ' ', count);
            styles[stop.y].insert(stop.x, style, count);
        }
        else
        {
            lines[stop.y].insert(stop.x, ch);
            styles[stop.y].insert(stop.x, style);
        }

        for (Cursor & c : cursors)
        {
            if (c.start.x > start.x && c.start.y == start.y) c.start.x += count;
            if (c.stop.x  > stop.x  && c.stop.y  == stop.y)  c.stop.x  += count;
        }

        stop = NextPosition(stop, count);
    }

    InvalidateStyles(cursor.start.y);
}

void Buffer::Insert(Cursor & cursor, String32 const& text, Qt::KeyboardModifiers modifiers)
{
    for (char32_t ch : text) Insert(cursor, ch, modifiers);
}

void Buffer::HandleCursorInput(QKeyEvent *event)
{
    bool text_changed = false;
    bool cursor_moved = false;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    QString input = event->text();
    for (Cursor & cursor : cursors)
    {
        switch (event->key())
        {
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_CapsLock:
            break;
        case Qt::Key_Escape:
            cursor = cursors[0];
            break;
        case Qt::Key_Left:
            MoveLeft(cursor, modifiers);
            cursor_moved = true;
            break;
        case Qt::Key_Up:
            MoveUp(cursor, modifiers);
            cursor_moved = true;
            break;
        case Qt::Key_Right:
            MoveRight(cursor, modifiers);
            cursor_moved = true;
            break;
        case Qt::Key_Down:
            MoveDown(cursor, modifiers);
            cursor_moved = true;
            break;
        case Qt::Key_Home:
            MoveHome(cursor, modifiers);
            cursor_moved = true;
            break;
        case Qt::Key_End:
            MoveEnd(cursor, modifiers);
            cursor_moved = true;
            break;
        case Qt::Key_Backspace:
            DeleteNext(cursor, modifiers);
            text_changed = true;
            break;
        case Qt::Key_Delete:
            DeletePrev(cursor, modifiers);
            text_changed = true;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            RemoveText(cursor);
            InsertNewLine(cursor, modifiers);
            text_changed = true;
            break;
        case Qt::Key_Tab:
            RemoveText(cursor);
            do    Insert(cursor, ' ');
            while (cursor.stop.x % 4 != 0);
            text_changed = true;
            break;
        default:
            text_changed = true;
            RemoveText(cursor);
            Insert(cursor, input.toStdU32String());
            break;
        }

        if (text_changed || (cursor_moved && !(modifiers & Qt::ShiftModifier)))
        {
            cursor.start = cursor.stop;
        }

        //if (text_changed) emit TextChanged();
    }
}

void Buffer::HandleKeyboardInput(QKeyEvent * event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();

    int line_count = LineCount();

    if (modifiers & Qt::ControlModifier)
    {
        switch (event->key())
        {
        case Qt::Key_A:
            cursors.resize(1);
            cursors[0].start.y = 0;
            cursors[0].start.x = 0;
            cursors[0].stop.y = std::max(LineCount() - 1, 0);
            cursors[0].stop.x = lines.back().size();
            break;
        default:
            HandleCursorInput(event);
        }
    }
    else
    {
        HandleCursorInput(event);
    }

    //if (LineCount() != line_count) emit LineCountChanged();
}



void Buffer::PaintTextMargin(Painter & painter, int scroll)
{
    QRect rect = painter.Rect();

    painter.FillRect(rect, QColor(38, 50, 56));

    int height = CellSize().height();
    int width  = CellSize().width();

    int last_line = std::min(rect.height() / height + scroll + 1, LineCount());

    for (Cursor const& cursor : cursors)
    {
        Position start = cursor.start;
        Position stop  = cursor.stop;

        start.x = std::min(start.x, LineLength(start.y));
        stop.x  = std::min(stop.x,  LineLength(stop.y));

        start.y -= scroll;
        stop.y  -= scroll;

        Position pointer = stop;

        if (start > stop) std::swap(stop, start);

        painter.SetPen(QColor(0, 0, 0));
        if (start.y == stop.y)
        {
            int x = start.x * width;
            int y = start.y * height;
            int w = width * (stop.x - start.x);
            int h = height;
            QRect selection_rect(x, y, w, h);
            painter.FillRect(selection_rect, QColor(85, 85, 85));
            painter.DrawRect(selection_rect);
        }
        else
        {
            int top_x = start.x * width;
            int top_y = start.y * height;
            int top_w = rect.width() - top_x;
            int top_h = height;
            QRect top_rect(top_x, top_y, top_w, top_h);
            painter.FillRect(top_rect, QColor(85, 85, 85));

            int bottom_x = 0;
            int bottom_y = stop.y * height;
            int bottom_w = stop.x * width;
            int bottom_h = height;
            QRect bottom_rect(bottom_x, bottom_y, bottom_w, bottom_h);
            painter.FillRect(bottom_rect, QColor(85, 85, 85));

            int mid_x = 0;
            int mid_y = top_y + top_h;
            int mid_w = rect.width();
            int mid_h = bottom_y - mid_y;
            QRect mid_rect(mid_x, mid_y, mid_w, mid_h);
            painter.FillRect(mid_rect, QColor(85, 85, 85));

            painter.DrawLine(top_x, top_y, top_w, 0);
            painter.DrawLine(top_x, top_y, 0, top_h);

            painter.DrawLine(bottom_x, bottom_y + bottom_h, bottom_w, 0);
            painter.DrawLine(bottom_w, bottom_y, 0, bottom_h);

            painter.DrawLine(mid_x, mid_y, top_x, 0);
            painter.DrawLine(bottom_w, bottom_y, mid_w - bottom_w, 0);
        }

        painter.SetPen(QColor(255, 204, 0));
        QRect cursor_rect(pointer.x * width, pointer.y * height, width, height);
        painter.DrawRect(cursor_rect);
    }

    Theme & theme = ThemeManager::Instance().CurrentTheme();

    int style = STYLE_DEFAULT;
    painter.SetPen(theme[style].forecolor);

    for (int y = scroll; y < last_line; y++)
    {
        int x0 = 0;
        int x = 0;
        for (; x < LineLength(y); x++)
        {
            int next_style = styles[y][x];
            if (style != next_style)
            {
                style = next_style;

                int size = x - x0;
                if (size != 0)
                {
                    int px = x0 * width;
                    int py = (y - scroll) * height;
                    QRect rect = QRect(px, py, width * size, height);
                    painter.DrawText(rect, &lines[y][x0], size);

                    x0 = x;
                }

                painter.SetPen(theme[style].forecolor);
            }
        }

        int size = x - x0;
        if (size != 0)
        {
            int px = x0 * width;
            int py = (y - scroll) * height;
            QRect rect = QRect(px, py, width * size, height);
            painter.DrawText(rect, &lines[y][x0], size);

            x0 = x;
        }
    }
}

void Buffer::PaintLineNumberMargin(Painter & painter, int scroll)
{
    QRect rect = painter.Rect();

    painter.FillRect(rect, QColor(30, 42, 48));
    painter.SetPen(QColor(191, 191, 191));

    int height = CellSize().height();
    int width  = CellSize().width();

    int last_line = std::min(rect.height() / height + scroll + 1, LineCount());

    QRect line_rect(0, 0, rect.width() - width, height);
    for (int idx = scroll + 1; idx <= last_line; idx++)
    {
        QString number = QString::number(idx);
        int flags = Qt::AlignVCenter | Qt::AlignRight;
        painter.DrawText(line_rect, number.toStdU32String(), flags);
        line_rect.moveTop(line_rect.top() + height);
    }
}

void Buffer::UpdateCellSize()
{
    cell_size = QFontMetrics(font).size(Qt::TextSingleLine, "W");
}

Buffer::Buffer()
{
    lexer = nullptr;

    font = QFont("Consolas NF", 9);
    UpdateCellSize();

    SetText("This is somekind of test");

    style_pos.x = 0;
    style_pos.y = 0;
//    StartStyling(style_pos);

//    SetStyle(10, STYLE_KEYWORD);
}

char32_t Buffer::CharacterAt(Position pos)
{
    if (pos.x == LineLength(pos.y)) return U'\n';

    return lines[pos.y][pos.x];
}

int Buffer::TextSize()
{
    return TextSize(FirstPosition(), LastPosition());
}

int Buffer::TextSize(Position start)
{
    return TextSize(start, LastPosition());
}

int Buffer::TextSize(Position start, Position stop)
{
    if (start.y == stop.y)
    {
        return stop.x - stop.y;
    }

    int count = LineLength(start.y) - start.x + stop.x + 1;

    for (int idx = start.y + 1; idx < stop.y; idx++)
    {
        count += LineLength(idx) + 1;
    }

    return count;
}

String32 Buffer::Text()
{
    return Text(FirstPosition(), LastPosition());
}

String32 Buffer::Text(Position start)
{
    return Text(start, LastPosition());
}

String32 Buffer::Text(Position start, Position stop)
{
    String32 text;
//    text.resize(TextSize(start, stop));

    int idx = 0;
    while (start < stop)
    {
        text.push_back(CharacterAt(start));

        start = NextPosition(start);
        idx++;
    }

    return text;
}

int Buffer::LineCount()
{
    return lines.size();
}

int Buffer::LineLength(int line_idx)
{
    return lines[line_idx].size();
}

int Buffer::MaximumLineLength()
{
    int max = 0;
    for (String32 const& line : lines)
    {
        max = std::max(max, line.size());
    }
    return max;
}

Position Buffer::LineStart(int line_idx)
{
    Position pos;
    pos.y = line_idx;
    pos.x = 0;
    return pos;
}

Position Buffer::LineEnd(int line_idx)
{
    Position pos;
    pos.y = line_idx;
    pos.x = LineLength(line_idx);
    return pos;
}

void Buffer::SelectionClear()
{
    for (Cursor & cursor : cursors)
    {
        cursor.start = cursor.stop;
    }
}

void Buffer::CursorAdjustToMinPosition()
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop = std::min(cursor.stop, cursor.start);
    }
}

void Buffer::CursorAdjustToMaxPosition()
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop = std::max(cursor.stop, cursor.start);
    }
}

void Buffer::CursorAdjustLeft(int count)
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop = PrevPosition(cursor.stop, count);
    }
}

void Buffer::CursorAdjustUp(int count)
{
    while (count--)
    {
        for (Cursor & cursor : cursors)
        {
            cursor.stop.y = std::max(cursor.stop.y - 1, 0);
        }
    }
}

void Buffer::CursorAdjustRight(int count)
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop = NextPosition(cursor.stop, count);
    }
}

void Buffer::CursorAdjustDown(int count)
{
    while (count--)
    {
        for (Cursor & cursor : cursors)
        {
            cursor.stop.y = std::min(cursor.stop.y + 1, LineCount() - 1);
        }
    }
}

void Buffer::CursorAdjustToPrevBorder()
{
    for (Cursor & cursor : cursors)
    {
        int count = DistanceToPrevBorder(cursor.stop);
        cursor.stop = PrevPosition(cursor.stop, count);
    }
}

void Buffer::CursorAdjustToNextBorder()
{
    for (Cursor & cursor : cursors)
    {
        int count = DistanceToNextBorder(cursor.stop);
        cursor.stop = NextPosition(cursor.stop, count);
    }
}

void Buffer::CursorAdjustToLineStart()
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop.x = 0;
    }
}

void Buffer::CursorAdjustToLineEnd()
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop.x = LineLength(cursor.stop.y);
    }
}

void Buffer::CursorAdjustToBufferStart()
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop = FirstPosition();
    }
}

void Buffer::CursorAdjustToBufferEnd()
{
    for (Cursor & cursor : cursors)
    {
        cursor.stop = LastPosition();
    }
}

void Buffer::CursorMoveLeft(int count)
{
    for (Cursor & cursor : cursors)
    {
        int actual_count = count;
        if (cursor.start != cursor.stop)
        {
            actual_count--;
            cursor.stop = std::min(cursor.stop, cursor.start);
        }
        cursor.stop = PrevPosition(cursor.stop, actual_count);
    }
    SelectionClear();
}

void Buffer::CursorMoveUp(int count)
{
    CursorAdjustToMinPosition();
    CursorAdjustUp(count);
    SelectionClear();
}

void Buffer::CursorMoveRight(int count)
{
    for (Cursor & cursor : cursors)
    {
        int actual_count = count;
        if (cursor.start != cursor.stop)
        {
            actual_count--;
            cursor.stop = std::max(cursor.stop, cursor.start);
        }
        cursor.stop = NextPosition(cursor.stop, actual_count);
    }
    SelectionClear();
}

void Buffer::CursorMoveDown(int count)
{
    CursorAdjustToMaxPosition();
    CursorAdjustDown(count);
    SelectionClear();
}

void Buffer::CursorMoveToPrevBorder()
{
    CursorAdjustToPrevBorder();
    SelectionClear();
}

void Buffer::CursorMoveToNextBorder()
{
    CursorAdjustToNextBorder();
    SelectionClear();
}

void Buffer::CursorMoveToLineStart()
{
    CursorAdjustToLineStart();
    SelectionClear();
}

void Buffer::CursorMoveToLineEnd()
{
    CursorAdjustToLineEnd();
    SelectionClear();
}

void Buffer::CursorMoveToBufferStart()
{
    Cursor cursor;
    cursor.start = cursor.stop = FirstPosition();

    cursors = { cursor };
}

void Buffer::CursorMoveToBufferEnd()
{
    Cursor cursor;
    cursor.start = cursor.stop = LastPosition();

    cursors = { cursor };
}

void Buffer::CursorSelectAll()
{
    Cursor cursor;

    cursor.start = FirstPosition();
    cursor.stop  = LastPosition();

    cursors = { cursor };
}

void Buffer::CursorCloneUp()
{
    Vector<Cursor> new_cursors;

    for (Cursor cursor : cursors)
    {
        Cursor c = cursor;

        if (c.start.y != 0) c.start.y--;
        if (c.stop.y  != 0) c.stop.y--;

        new_cursors.push_back(c);
    }

    for (Cursor cursor : new_cursors)
    {
        AddCursor(cursor);
    }
}

void Buffer::CursorCloneDown()
{
    Vector<Cursor> new_cursors;

    int max_y = LineCount() - 1;

    for (Cursor cursor : cursors)
    {
        Cursor c = cursor;

        if (c.start.y != max_y) c.start.y++;
        if (c.stop.y  != max_y) c.stop.y++;

        new_cursors.push_back(c);
    }

    for (Cursor cursor : new_cursors)
    {
        AddCursor(cursor);
    }
}

void Buffer::CursorDeletePrev()
{
    for (Cursor & cursor : cursors)
    {
        if (cursor.start == cursor.stop)
        {
            cursor.stop = PrevPosition(cursor.stop);
        }
    }

    ConsolidateCursors();
    CursorDeleteSelection();
}

void Buffer::CursorDeleteNext()
{
    for (Cursor & cursor : cursors)
    {
        if (cursor.start == cursor.stop)
        {
            cursor.stop = NextPosition(cursor.stop);
        }
    }

    ConsolidateCursors();
    CursorDeleteSelection();
}

void Buffer::CursorDeleteToPrevBorder()
{
    for (Cursor & cursor : cursors)
    {
        if (cursor.start == cursor.stop)
        {
            int count = DistanceToPrevBorder(cursor.stop);
            cursor.stop = PrevPosition(cursor.stop, count);
        }
    }

    ConsolidateCursors();
    CursorDeleteSelection();
}

void Buffer::CursorDeleteToNextBorder()
{
    for (Cursor & cursor : cursors)
    {
        if (cursor.start == cursor.stop)
        {
            int count = DistanceToNextBorder(cursor.stop);
            cursor.stop = NextPosition(cursor.stop, count);
        }
    }

    ConsolidateCursors();
    CursorDeleteSelection();
}

void Buffer::CursorReplaceText(String32 const& text)
{
    CursorDeleteSelection();

    for (Cursor & cursor : cursors)
    {
        Insert(cursor, text);
    }

    //    int start = 0;

    //    while (start < text.size())
    //    {
    //        int stop = text.index_of_newline(start);
    //        if (stop == -1) stop = text.size();

    //        for (int outer = 0; outer < cursors.size(); outer++)
    //        {
    //            Cursor & cursor = cursors[outer];

    //            int style;
    //            if (cursor.stop == FirstPosition()) style = STYLE_DEFAULT;
    //            else                                style = StyleAt(cursor.stop);

    //            for (int inner = outer + 1; inner < cursors.size(); inner++)
    //            {

    //            }
    //        }
    //    }

    //    for (Cursor & cursor : cursors)
    //    {
    //        if (IsLineBreak(ch))
    //        {
    //            InsertNewLine(cursor);
    //        }
    //        else
    //        {
    //            Position & start = cursor.start;
    //            Position & stop = cursor.stop;

    //            stop.x = std::min(stop.x, LineLength(stop.y));

    //            int style;
    //            if (stop.x != 0) style = StyleAt(PrevPosition(stop));
    //            else             style = STYLE_DEFAULT;

    //            lines[stop.y].insert(stop.x, ch);
    //            styles[stop.y].insert(stop.x, style);

//            for (Cursor & c : cursors)
//            {
//                if (c.start.x > start.x && c.start.y == start.y) c.start.x++;
//                if (c.stop.x  > stop.x  && c.stop.y  == stop.y)  c.stop.x++;
//            }

//            MoveRight(cursor);
//        }

//        InvalidateStyles(cursor.start.y);
//    }
}

void Buffer::CursorReplaceText(Vector<String32> const& text)
{
    CursorDeleteSelection();
}

void Buffer::CursorInsertText(String32 const& text)
{
    CursorReplaceText(text);
    SelectionClear();
}

void Buffer::CursorInsertText(Vector<String32> const& text)
{
    CursorReplaceText(text);
    SelectionClear();
}

void Buffer::CursorDeleteSelection()
{
    for (Cursor & cursor : cursors)
    {
        Position start = cursor.start;
        Position stop  = cursor.stop;

        if (start > stop) std::swap(stop, start);

        start.x = std::min(start.x, LineLength(start.y));
        stop.x  = std::min(stop.x,  LineLength(stop.y));

        if (stop.y == start.y)
        {
            lines[start.y].remove(start.x, stop.x - start.x);
            styles[start.y].remove(start.x, stop.x - start.x);
        }
        else
        {
            lines[start.y].resize(start.x);
            lines[start.y] += lines[stop.y].middle(stop.x);

            lines.remove(start.y + 1, stop.y - start.y);

            styles[start.y].resize(start.x);
            styles[start.y] += styles[stop.y].middle(stop.x);

            styles.remove(start.y + 1, stop.y - start.y);
        }

        cursor.start = start;
        cursor.stop  = cursor.start;

        InvalidateStyles(cursor.start.y);

        for (Cursor & c : cursors)
        {
            c.start = DeleteAdjustedPosition(start, stop, c.start);
            c.stop  = DeleteAdjustedPosition(start, stop, c.stop);
        }
    }
}

void Buffer::ConsolidateCursors()
{
    for (int outer = 0; outer < cursors.size(); outer++)
    {
        Position start_a = cursors[outer].start;
        Position stop_a = cursors[outer].stop;

        bool is_backward_a = start_a > stop_a;
        if (is_backward_a) std::swap(start_a, stop_a);

        for (int inner = cursors.size() - 1; inner > outer; inner--)
        {
            Position start_b = cursors[inner].start;
            Position stop_b = cursors[inner].stop;

            bool is_backward_b = start_b > stop_b;
            if (is_backward_b)     std::swap(start_b, stop_b);
            if (start_b > stop_a)  continue;
            if (start_a > stop_b)  continue;
            if (start_b < start_a) std::swap(start_a, start_b);
            if (stop_b > stop_a)   std::swap(stop_a, stop_b);

            cursors.remove(inner);

            cursors[outer].start = start_a;
            cursors[outer].stop  = stop_a;

            bool is_backward = is_backward_a || is_backward_b;
            if (is_backward) std::swap(cursors[outer].start, cursors[outer].stop);
        }
    }
}

bool Buffer::IsOnBorder(Position pos)
{
    int line_length = LineLength(pos.y);
    if (pos.x == 0)           return pos.y == 0 || line_length != 0;
    if (pos.x == line_length) return true;

    char32_t ch1 = lines[pos.y][pos.x - 1];
    char32_t ch2 = lines[pos.y][pos.x];

    if (IsSpace(ch1))          return !IsSpace(ch2);
    if (IsIdentifierChar(ch1)) return !IsIdentifierChar(ch2);

    return true;
}

Position Buffer::PrevPosition(Position pos, int count)
{
    while (count != 0)
    {
        int line_size = LineLength(pos.y);

        pos.x = std::min(pos.x, line_size);
        if (pos.x != 0)
        {
            int new_x = std::max(0, pos.x - count);
            count -= pos.x - new_x;
            pos.x = new_x;
        }
        else if (pos.y != 0)
        {
            count--;
            pos.y--;
            pos.x = LineLength(pos.y);
        }
        else
        {
            break;
        }
    }
    return pos;
}

Position Buffer::NextPosition(Position pos, int count)
{
    while (count != 0)
    {
        int line_size = LineLength(pos.y);

        pos.x = std::min(pos.x, line_size);
        if (pos.x != line_size)
        {
            int new_x = std::min(line_size, pos.x + count);
            count -= new_x - pos.x;
            pos.x = new_x;
        }
        else if (pos.y != LineCount() - 1)
        {
            count--;
            pos.y++;
            pos.x = 0;
        }
        else
        {
            break;
        }
    }
    return pos;
}

int Buffer::StyleAt(Position pos)
{
    if (pos.x > LineLength(pos.y)) return STYLE_DEFAULT;
    return styles[pos.y][pos.x];
}

void Buffer::StartStyling(Position pos)
{
    style_pos = std::min(pos, style_pos);
}

void Buffer::SetStyle(int count, int style)
{
    while (count--)
    {
        if (style_pos.y > LineCount()) break;

        if (style_pos.x <= LineLength(style_pos.y))
        {
            styles[style_pos.y][style_pos.x] = style;
        }
        style_pos = NextPosition(style_pos);
    }
}

void Buffer::EnsureStyled(Position pos)
{
    if (lexer != nullptr && style_pos <= pos)
    {
        lexer->Style(style_pos, NextPosition(pos));

        style_pos = pos;
    }
}

void Buffer::EnsureStyled(int line_idx)
{
    EnsureStyled(LineEnd(line_idx));
}

void Buffer::InvalidateStyles()
{
    style_pos = FirstPosition();
}

void Buffer::InvalidateStyles(Position pos)
{
    style_pos = pos;
}

void Buffer::InvalidateStyles(int line_idx)
{
    style_pos = std::min(style_pos, LineStart(line_idx));
}

void Buffer::SetLexer(Lexer * new_lexer)
{
    lexer = new_lexer;
    new_lexer->SetParent(this);
}

QSize const& Buffer::CellSize()
{
    return cell_size;
}

int Buffer::LineNumberMarginWidth()
{
    int count = (int)std::log10(LineCount()) + 1;
    return CellSize().width() * (count + 1);
}

void Buffer::ClearCursors()
{
    cursors.clear();
}

void Buffer::AddCursor(Cursor const& cursor)
{
    auto pred = [](Cursor const& lhs, Cursor const& rhs) { return lhs.stop < rhs.stop; };
    cursors.insert(std::upper_bound(cursors.begin(), cursors.end(), cursor, pred), cursor);
    ConsolidateCursors();
}

Cursor const& Buffer::FirstCursor()
{
    return cursors.front();
}

Cursor const& Buffer::LastCursor()
{
    return cursors.back();
}

void Buffer::AddLineSelection(int line_idx)
{
    Cursor cursor;
    cursor.start.x = 0;
    cursor.start.y = line_idx;
    cursor.stop.x = LineLength(line_idx);
    cursor.stop.y = line_idx;
    AddCursor(cursor);
}

void Buffer::AddWordSelection(Position pos)
{
    int distance_next = DistanceToNextBorder(pos);
    int distance_prev = IsOnBorder(pos) ? 0 : DistanceToPrevBorder(pos);

    Cursor cursor;
    cursor.start = PrevPosition(pos, distance_prev);
    cursor.stop = NextPosition(pos, distance_next);
    AddCursor(cursor);
}

void Buffer::SetPointSize(int size)
{
    font.setPointSize(size);
    UpdateCellSize();
}

void Buffer::SetFontName(QString const& name)
{
    font.setFamily(name);
    UpdateCellSize();
}

void Buffer::DoPaste()
{
    int line_count = LineCount();

    String32 text = Clipboard::Instance().Text();
    for (Cursor & cursor : cursors)
    {
        RemoveText(cursor);
        Insert(cursor, text);
        cursor.start = cursor.stop;
    }
}

void Buffer::DoCopy()
{

}

void Buffer::ZoomIn(int amount)
{
    SetPointSize(std::min(font.pointSize() + amount, 75));
}

void Buffer::ZoomOut(int amount)
{
    SetPointSize(std::max(font.pointSize() - amount, 0));
}

void Buffer::KeyPress(QKeyEvent * event)
{
    HandleKeyboardInput(event);
    ConsolidateCursors();
}

void Buffer::Paint(Painter & painter, int scroll)
{
    QRect rect = painter.Rect();

    int margin_width = LineNumberMarginWidth();

    QRect text_rect = rect.adjusted(margin_width, 0, 0, 0);
    QRect margin_rect(rect.topLeft(), QSize(margin_width, rect.height()));

    painter.SetFont(font);

    painter.SetClipRect(text_rect);
    PaintTextMargin(painter, scroll);

    painter.SetClipRect(margin_rect);
    PaintLineNumberMargin(painter, scroll);
}
