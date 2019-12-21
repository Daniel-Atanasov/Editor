#ifndef CURSOR_HPP
#define CURSOR_HPP


struct Position
{
    int y;
    int x;

    bool operator==(Position const& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(Position const& other) const
    {
        return !operator==(other);
    }

    bool operator<(Position const& other) const
    {
        return y < other.y || (y == other.y && x < other.x);
    }

    bool operator<=(Position const& other) const
    {
        return operator==(other) || operator<(other);
    }

    bool operator>(Position const& other) const
    {
        return !operator<=(other);
    }

    bool operator>=(Position const& other) const
    {
        return !operator<(other);
    }
};

struct Cursor
{
    Position start;
    Position stop;

    bool operator==(Cursor const& other) const
    {
        return start == other.start && stop == other.stop;
    }

    bool operator!=(Cursor const& other) const
    {
        return !operator==(other);
    }
};

#endif // CURSOR_HPP
