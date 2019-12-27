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

void Buffer::SetText(TextView const& text)
{
    lines.clear();
    styles.clear();

    lines.resize(1);
    styles.resize(1);

    styles[0].push_back(STYLE_DEFAULT);

    Cursor cursor = { {0, 0}, {0, 0} };

    cursors = { cursor };
    CursorReplaceText(text);
    cursors = { cursor };
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
        stop.x  = std::min(stop.x, LineLength(stop.y));

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

    // TODO@Daniel:
    //  There is a visible slowdown in Debug with this, likely cause by switching QPen constantly
    //  This could be optimized in a couple of ways, though I don't think it should be a problem in the first place?
    //  Judging from VerySleepy benchmarks, it appears that there are some JSON shenanigans happening underneath
    //  Minimizing pen changes should be a decent speedup
    //  Ignoring whitespaces to reduce draw calls might be a bigger one, though

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

                    StringView32 segment = lines[y].middle_view(x0, size);
                    painter.DrawText(rect, segment);

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

            StringView32 segment = lines[y].middle_view(x0, size);
            painter.DrawText(rect, segment);

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
        int flags = Qt::AlignVCenter | Qt::AlignRight;
        painter.DrawText(line_rect, ToStringView32(idx), flags);
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

    SetText(U"This is somekind of test");

    style_pos.x = 0;
    style_pos.y = 0;
}

int Buffer::SelectionSizeTotal()
{
    int count = 0;

    for (Cursor const& cursor : cursors)
    {
        Position start = cursor.start;
        Position stop  = cursor.stop;

        if (stop < start) std::swap(start, stop);

        count += TextSize(start, stop);
    }

    return count;
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
        if (c.stop.y != 0) c.stop.y--;

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
        if (c.stop.y != max_y) c.stop.y++;

        new_cursors.push_back(c);
    }

    for (Cursor cursor : new_cursors)
    {
        AddCursor(cursor);
    }
}

int Buffer::CursorDeletePrev()
{
    for (Cursor & cursor : cursors)
    {
        if (cursor.start == cursor.stop)
        {
            cursor.stop = PrevPosition(cursor.stop);
        }
    }

    ConsolidateCursors();
    return CursorDeleteSelection();
}

int Buffer::CursorDeleteNext()
{
    for (Cursor & cursor : cursors)
    {
        if (cursor.start == cursor.stop)
        {
            cursor.stop = NextPosition(cursor.stop);
        }
    }

    ConsolidateCursors();
    return CursorDeleteSelection();
}

int Buffer::CursorDeleteToPrevBorder()
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
    return CursorDeleteSelection();
}

int Buffer::CursorDeleteToNextBorder()
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
    return CursorDeleteSelection();
}

int Buffer::CursorReplaceText(TextView const& text)
{
    int deleted_count = CursorDeleteSelection();

    StringView32 first_line = text.FirstLine();
    StringView32 last_line = text.LastLine();

    int line_count = text.LineCount();
    for (int idx = 0; idx < cursors.size(); idx++)
    {
        Position & start = cursors[idx].start;
        Position & stop  = cursors[idx].stop;

        int style;
        if (start.x != 0) style = StyleAt(PrevPosition(start));
        else              style = STYLE_DEFAULT;

        if (line_count == 1)
        {
            int size = first_line.size();

            lines[start.y].insert(start.x, first_line);
            styles[start.y].insert(start.x, style, first_line.size());

            for (int inner_idx = idx + 1; inner_idx < cursors.size(); inner_idx++)
            {
                Cursor & cursor = cursors[inner_idx];

                Position & st = cursor.start;
                Position & sp = cursor.stop;

                if (st.y == start.y && st.x >= start.x) st.x += size;
                if (sp.y == start.y && sp.x >= start.x) sp.x += size;
            }

            stop.x += size;
        }
        else
        {
            int added_line_count = line_count - 1;

            lines.insert(start.y + 1, {}, added_line_count);
            styles.insert(start.y + 1, {}, added_line_count);

            String32 & cursor_line = lines[start.y];

            StringView32 first_half = cursor_line.middle_view(0, start.x);
            StringView32 second_half = cursor_line.middle_view(start.x);

            Position pos = start;
            for (int inner_idx = idx; inner_idx < cursors.size(); inner_idx++)
            {
                Cursor & cursor = cursors[inner_idx];

                Position & st = cursor.start;
                Position & sp = cursor.stop;

                int dx = last_line.size() - first_half.size();

                if (st.y == pos.y && st.x >= pos.x)
                {
                    st.x += dx;
                    st.y += added_line_count;
                }
                else if (st.y > pos.y)
                {
                    st.y += added_line_count;
                }

                if (sp.y == pos.y && sp.x >= pos.x)
                {
                    sp.x += dx;
                    sp.y += added_line_count;
                }
                else if (sp.y > pos.y)
                {
                    sp.y += added_line_count;
                }
            }
            start = pos;

            int last_line_idx = start.y + added_line_count;

            lines[last_line_idx].reserve(second_half.size() + last_line.size());

            lines[last_line_idx] = second_half;
            styles[last_line_idx] = styles[start.y].middle(start.x, second_half.size());

            for (int line_idx = 1; line_idx < line_count; line_idx++)
            {
                Position line_start = LineStart(start.y + line_idx);
                StringView32 line_text = text.LineAt(line_idx);

                lines[line_start.y].insert(0, line_text);

                styles[line_start.y].insert(0, style, line_text.size());
                styles[line_start.y].push_back(style);
            }

            lines[start.y].reserve(first_half.size() + first_line.size());
            lines[start.y].resize(first_half.size());
            lines[start.y] += first_line;
            styles[start.y].resize(first_half.size());
            styles[start.y].resize(first_half.size() + first_line.size() + 1, style);
        }
    }

    return deleted_count + text.TextSize() * cursors.size();
}

int Buffer::CursorInsertText(TextView const& text)
{
    int size = CursorReplaceText(text);
    SelectionClear();
    return size;
}

int Buffer::CursorDeleteSelection()
{
    int size = SelectionSizeTotal();

    for (Cursor & cursor : cursors)
    {
        Position start = cursor.start;
        Position stop  = cursor.stop;

        if (start > stop) std::swap(stop, start);

        start.x = std::min(start.x, LineLength(start.y));
        stop.x  = std::min(stop.x, LineLength(stop.y));

        if (stop.y == start.y)
        {
            lines[start.y].remove(start.x, stop.x - start.x);
            styles[start.y].remove(start.x, stop.x - start.x);
        }
        else
        {
            lines[start.y].resize(start.x);
            lines[start.y] += lines[stop.y].middle_view(stop.x);

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

    return -size;
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
        lexer->Style(style_pos, pos);

        style_pos = pos;
    }
}

void Buffer::EnsureStyled(int line_idx)
{
    if (line_idx == LineCount() - 1)
    {
        EnsureStyled(LastPosition());
    }
    else
    {
        EnsureStyled(LineStart(line_idx + 1));
    }
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

int Buffer::DoPaste()
{
    String32 text = Clipboard::Instance().Text();
    return CursorInsertText(text);
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
    SetPointSize(std::max(font.pointSize() - amount, 1));
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
