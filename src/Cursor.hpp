#ifndef CURSOR_HPP
#define CURSOR_HPP

struct Position
{
    int y;
    int x;

    bool operator==(Position other) const noexcept;
    bool operator!=(Position other) const noexcept;
    bool operator<(Position other) const noexcept;
    bool operator<=(Position other) const noexcept;
    bool operator>(Position other) const noexcept;
    bool operator>=(Position other) const noexcept;
};

struct Cursor
{
    Position start;
    Position stop;

    bool operator==(Cursor other) const noexcept;
    bool operator!=(Cursor other) const noexcept;
};

#endif // CURSOR_HPP
