#ifndef TEXTCONTAINER_HPP
#define TEXTCONTAINER_HPP

#include <QString>
#include <QDebug>

#include "StringView32.hpp"
#include "String32.hpp"
#include "Cursor.hpp"
#include "SpecialCharacters.hpp"

class TextView;

template <typename DerivedType, typename TextView = TextView>
class TextContainer
{
private:
    auto & Lines() noexcept
    {
        DerivedType * that = (DerivedType*)this;
        return that->lines;
    }

    auto const& Lines() const noexcept
    {
        DerivedType const* that = (DerivedType const*)this;
        return that->lines;
    }

protected:
    Position FirstPosition() const noexcept
    {
        Position pos;
        pos.x = 0;
        pos.y = 0;
        return pos;
    }

    Position LastPosition() const noexcept
    {
        Position pos;
        pos.y = LineCount() - 1;
        pos.x = LineLength(pos.y);
        return pos;
    }

    int DistanceToPrevBorder(Position pos) const noexcept
    {
        int count = 0;
        do
        {
            pos = PrevPosition(pos);
            count++;
        } while (!IsOnBorder(pos));
        return count;
    }

    int DistanceToNextBorder(Position pos) const noexcept
    {
        int count = 0;
        do
        {
            pos = NextPosition(pos);
            count++;
        } while (!IsOnBorder(pos));
        return count;
    }

public:
    StringView32 LineAt(int idx) const noexcept
    {
        return Lines()[idx];
    }

    StringView32 FirstLine() const noexcept
    {
        return LineAt(0);
    }

    StringView32 LastLine() const noexcept
    {
        return LineAt(LineCount() - 1);
    }
    
    char32_t CharacterAt(Position pos) const noexcept
    {
        if (pos.x == LineLength(pos.y)) return U'\n';

        return Lines()[pos.y][pos.x];
    }

    int TextSize() const noexcept
    {
        return TextSize(FirstPosition(), LastPosition());
    }

    int TextSize(Position start) const noexcept
    {
        return TextSize(start, LastPosition());
    }

    int TextSize(Position start, Position stop) const noexcept
    {
        if (start.y == stop.y)
        {
            return stop.x - start.x;
        }

        int count = LineLength(start.y) - start.x + stop.x + 1;

        for (int idx = start.y + 1; idx < stop.y; idx++)
        {
            count += LineLength(idx) + 1;
        }

        return count;
    }

    TextView View() const
    {
        return TextView(Lines());
    }

    TextView View(Position start) const
    {
        return View(start, LastPosition());
    }

    TextView View(Position start, Position stop) const
    {
        if (start.y == stop.y)
        {
            return Lines()[start.y].middle_view(start.x, stop.x - start.x);
        }

        Vector<StringView32> lines;
        lines.reserve(stop.y - start.y + 1);

        lines.push_back(Lines()[start.y].middle_view(start.x));

        for (int idx = start.y = 1; idx < stop.y idx++)
        {
            lines.push_back(Lines()[idx]);
        }

        lines.push_back(Lines()[stop.y].middle_view(0, stop.x));

        return lines;
    }

    String32 Text() const
    {
        return Text(FirstPosition(), LastPosition());
    }

    String32 Text(Position start) const
    {
        return Text(start, LastPosition());
    }

    String32 Text(Position start, Position stop) const
    {
        if (start.y == stop.y)
        {
            return Lines()[start.y].middle(start.x, stop.x - start.x);
        }

        String32 text;
        text.reserve(TextSize(start, stop));

        text += Lines()[start.y].middle_view(start.x);
        text += U'\n';
        for (int idx = start.y + 1; idx < stop.y; idx++)
        {
            text += Lines()[idx];
            text += U'\n';
        }
        text += Lines()[stop.y].middle_view(0, stop.x);

        return text;
    }

    int LineCount() const noexcept
    {
        return Lines().size();
    }

    int LineLength(int line_idx) const noexcept
    {
        return Lines()[line_idx].size();
    }

    int TabAdjustedLineLength(int idx) const noexcept
    {
        return Lines()[idx].tab_adjusted_size();
    }

    int MaximumLineLength() const noexcept
    {
        int max = 0;
        for (int idx = 0; idx < LineCount(); idx++)
        {
            max = std::max(max, LineLength(idx));
        }
        return max;
    }

    int MaximumTabAdjustedLineLength() const noexcept
    {
        int max = 0;
        for (int idx = 0; idx < LineCount(); idx++)
        {
            max = std::max(max, TabAdjustedLineLength(idx));
        }
        return max;
    }

    Position LineStart(int idx) const noexcept
    {
        Position pos;
        pos.y = idx;
        pos.x = 0;
        return pos;
    }

    Position LineEnd(int idx) const noexcept
    {
        Position pos;
        pos.y = idx;
        pos.x = LineLength(idx);
        return pos;
    }

    Position ClampPosition(Position pos) const noexcept
    {
        Position last_pos = LastPosition();
        if (pos > last_pos)
        {
            return last_pos;
        }

        pos.x = std::min(pos.x, LineLength(pos.y));

        return pos;
    }

    bool IsOnBorder(Position pos) const noexcept
    {
        int line_length = LineLength(pos.y);
        if (pos.x == 0)           return pos.y == 0 || line_length != 0;
        if (pos.x == line_length) return true;

        char32_t ch1 = Lines()[pos.y][pos.x - 1];
        char32_t ch2 = Lines()[pos.y][pos.x];

        if (IsSpace(ch1))          return !IsSpace(ch2);
        if (IsIdentifierChar(ch1)) return !IsIdentifierChar(ch2);

        return true;
    }

    Position PrevPosition(Position pos, int count = 1) const noexcept
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

    Position NextPosition(Position pos, int count = 1) const noexcept
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
};

#endif // TEXTCONTAINER_HPP