#ifndef TEXTCONTAINER_HPP
#define TEXTCONTAINER_HPP

#include <QString>
#include <QDebug>

#include "StringView32.hpp"
#include "String32.hpp"
#include "Cursor.hpp"
#include "SpecialCharacters.hpp"

template <typename DerivedType>
class TextContainer
{
private:
    auto & Lines()
    {
        DerivedType * that = (DerivedType*)this;
        return that->lines;
    }

    auto const& Lines() const
    {
        DerivedType const* that = (DerivedType const*)this;
        return that->lines;
    }

protected:
    Position FirstPosition() const
    {
        Position pos;
        pos.x = 0;
        pos.y = 0;
        return pos;
    }

    Position LastPosition() const
    {
        Position pos;
        pos.y = LineCount() - 1;
        pos.x = LineLength(pos.y);
        return pos;
    }

    int DistanceToPrevBorder(Position pos) const
    {
        int count = 0;
        do
        {
            pos = PrevPosition(pos);
            count++;
        } while (!IsOnBorder(pos));
        return count;
    }

    int DistanceToNextBorder(Position pos) const
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
    StringView32 LineAt(int idx) const
    {
        return Lines()[idx];
    }

    StringView32 FirstLine() const
    {
        return LineAt(0);
    }

    StringView32 LastLine() const
    {
        return LineAt(LineCount() - 1);
    }
    
    char32_t CharacterAt(Position pos) const
    {
        if (pos.x == LineLength(pos.y)) return U'\n';

        return Lines()[pos.y][pos.x];
    }

    int TextSize() const
    {
        return TextSize(FirstPosition(), LastPosition());
    }

    int TextSize(Position start) const
    {
        return TextSize(start, LastPosition());
    }

    int TextSize(Position start, Position stop) const
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

    int LineCount() const
    {
        return Lines().size();
    }

    int LineLength(int line_idx) const
    {
        return Lines()[line_idx].size();
    }

    int MaximumLineLength() const
    {
        int max = 0;
        for (int idx = 0; idx < LineCount(); idx++)
        {
            max = std::max(max, LineLength(idx));
        }
        return max;
    }

    Position LineStart(int line_idx) const
    {
        Position pos;
        pos.y = line_idx;
        pos.x = 0;
        return pos;
    }

    Position LineEnd(int line_idx) const
    {
        Position pos;
        pos.y = line_idx;
        pos.x = LineLength(line_idx);
        return pos;
    }

    bool IsOnBorder(Position pos) const
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

    Position PrevPosition(Position pos, int count = 1) const
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

    Position NextPosition(Position pos, int count = 1) const
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