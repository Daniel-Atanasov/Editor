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

void Buffer::PaintTextMargin(Painter & painter, int first_line)
{
    QRect rect = painter.Rect();

    painter.FillRect(rect, QColor(38, 50, 56));

    int ch = CellSize().height();
    int cw = CellSize().width();

    int max_cy = std::min(rect.height() / ch + first_line + 1, LineCount());

    for (Cursor cursor : cursors)
    {
        Position start = cursor.start;
        Position stop  = cursor.stop;

        start.x = std::min(start.x, LineLength(start.y));
        stop.x  = std::min(stop.x, LineLength(stop.y));

        char32_t chr = CharacterAt(stop);

        StringView32 start_line = lines[start.y];
        StringView32 stop_line  = lines[stop.y];

        start.x = start_line.adjust_for_tabs(start.x);
        stop.x  = stop_line.adjust_for_tabs(stop.x);

        start.y -= first_line;
        stop.y  -= first_line;

        if (stop.y < 0)        continue;
        if (start.y >= max_cy) continue;

        Position pointer = stop;

        if (start > stop) std::swap(stop, start);

        painter.SetPen(QColor(0, 0, 0));
        if (start.y == stop.y)
        {
            int x = start.x * cw;
            int y = start.y * ch;
            int w = cw * (stop.x - start.x);
            int h = ch;
            QRect selection_rect(x, y, w, h);
            painter.FillRect(selection_rect, QColor(85, 85, 85));
            painter.DrawRect(selection_rect);
        }
        else
        {
            int top_x = start.x * cw;
            int top_y = start.y * ch;
            int top_w = rect.width() - top_x;
            int top_h = ch;
            QRect top_rect(top_x, top_y, top_w, top_h);
            painter.FillRect(top_rect, QColor(85, 85, 85));

            int bottom_x = 0;
            int bottom_y = stop.y * ch;
            int bottom_w = stop.x * cw;
            int bottom_h = ch;
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

        int w = 1;
        if (chr == U'\t')
        {
            w = 4 - (pointer.x & 0b11);
        }

        painter.SetPen(QColor(255, 204, 0));
        QRect cursor_rect(pointer.x * cw, pointer.y * ch, w * cw, ch);
        painter.DrawRect(cursor_rect);
    }

    Theme & theme = ThemeManager::Instance().CurrentTheme();

    int style = STYLE_DEFAULT;
    painter.SetPen(theme[style].forecolor);

    int max_x = rect.right();

    for (int y = first_line; y < max_cy; y++)
    {
        StringView32 line = lines[y];

        int left = 0;
        int top = (y - first_line) * ch;

        int length = LineLength(y);

        int total_padding = 0;
        int last_x = 0;
        for (int x = 0; x <= length; x++)
        {
            int next_style = styles[y][x];
            if (style != next_style || x == length || line[x] == U'\t')
            {
                if (line[last_x] == U'\t')
                {
                    int padding = TabWidth(left);

                    left += padding - 1;
                    total_padding += padding;
                }

                style = next_style;

                int size = x - last_x;
                if (size != 0)
                {
                    StringView32 segment = line.middle_view(last_x, size);
                    painter.DrawText(left * cw, top, size * cw, ch, segment);

                    left += size;

                    last_x = x;
                }

                painter.SetPen(theme[style].forecolor);
            }
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

void Buffer::UpdateFontMetrics()
{
    // TODO@Daniel:
    //  Kinda temporary
    metrics = QFontMetrics(font);
    cell_size = metrics.size(Qt::TextSingleLine, "W");
    baseline = metrics.ascent();
}

Buffer::Buffer() : lines(1), styles(1, { STYLE_DEFAULT }), font("Consolas", 9), metrics(font)
{
    lexer = nullptr;

    UpdateFontMetrics();

    style_pos.x = 0;
    style_pos.y = 0;
}

bool Buffer::Flag(int flag)
{
    return flags & flag;
}

void Buffer::SetFlag(int flag, bool value)
{
    if (value) flags |= flag;
    else       flags &= ~flag;
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
            Position & stop = cursor.stop;

            if (stop.y != 0)
            {
                int vx = lines[stop.y].adjust_for_tabs(stop.x);
                stop.y--;
                stop.x = lines[stop.y].from_tab_adjusted(vx);
            }
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
    int max_y = LineCount() - 1;
    while (count--)
    {
        for (Cursor & cursor : cursors)
        {
            Position & stop = cursor.stop;

            if (stop.y != max_y)
            {
                int vx = lines[stop.y].adjust_for_tabs(stop.x);
                stop.y++;
                stop.x = lines[stop.y].from_tab_adjusted(vx);
            }
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
    new_cursors.reserve(cursors.size());

    for (Cursor c : cursors)
    {
        Position pos = std::min(c.start, c.stop);

        if (pos.y != 0)
        {
            int vx = lines[pos.y].adjust_for_tabs(pos.x);
            pos.y--;
            pos.x = lines[pos.y].from_tab_adjusted(vx);
        }

        c.start = c.stop = pos;

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
    new_cursors.reserve(cursors.size());

    int max_y = LineCount() - 1;

    for (Cursor c : cursors)
    {
        Position pos = std::max(c.start, c.stop);

        if (pos.y != max_y)
        {
            int vx = lines[pos.y].adjust_for_tabs(pos.x);
            pos.y++;
            pos.x = lines[pos.y].from_tab_adjusted(vx);
        }

        c.start = c.stop = pos;

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

            StringView32 first_half  = cursor_line.middle_view(0, start.x);
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

int Buffer::ConvertTabsToSpaces()
{
    int total = 0;

    int line_count = LineCount();
    for (int y = 0; y < line_count; y++)
    {
        int count = 0;

        int x = 0;
        while ((x = lines[y].index_of(U'\t', x)) != -1)
        {
            int size = TabWidth(x);
            lines[y].replace(x, 1, StringView32(U"    ", size));
            styles[y].insert(x, styles[y][x], size - 1);
            count += size - 1;

            for (Cursor & c : cursors)
            {
                if (c.start.y == y && c.start.x > x) c.start.x += size - 1;
                if (c.stop.y  == y && c.stop.x  > x) c.stop.x  += size - 1;
            }
        }

        total += count;
    }

    return total;
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

void Buffer::AddCursor(Cursor cursor)
{
    auto pred = [](Cursor const& lhs, Cursor const& rhs) { return lhs.stop < rhs.stop; };
    cursors.insert(std::upper_bound(cursors.begin(), cursors.end(), cursor, pred), cursor);
    ConsolidateCursors();
}

int Buffer::CursorCount()
{
    return cursors.size();
}

Cursor Buffer::FirstCursor()
{
    return cursors.front();
}

Cursor Buffer::LastCursor()
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
    UpdateFontMetrics();
}

void Buffer::SetFontName(QString const& name)
{
    font.setFamily(name);
    UpdateFontMetrics();
}

int Buffer::DoPaste()
{
    String32 text = Clipboard::Instance().Text();
    return CursorInsertText(text);
}

int Buffer::DoCut()
{
    DoCopy();
    return CursorDeleteSelection();
}

void Buffer::DoCopy()
{
    String32 text;
    text.reserve(SelectionSizeTotal());
    for (int idx = 0; idx < cursors.size(); idx++)
    {
        Position start = cursors[idx].start;
        Position stop  = cursors[idx].stop;

        if (stop < start) std::swap(start, stop);

        if (idx) text += U'\n';

        text += Text(start, stop);
    }

    Clipboard::Instance().SetText(std::move(text));
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
