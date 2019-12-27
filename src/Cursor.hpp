#ifndef CURSOR_HPP
#define CURSOR_HPP

struct Position
{
    int y;
    int x;

    bool operator==(Position const& other) const noexcept;
    bool operator!=(Position const& other) const noexcept;
    bool operator<(Position const& other) const noexcept;
    bool operator<=(Position const& other) const noexcept;
    bool operator>(Position const& other) const noexcept;
    bool operator>=(Position const& other) const noexcept;
};

struct Cursor
{
    Position start;
    Position stop;

    bool operator==(Cursor const& other) const noexcept;
    bool operator!=(Cursor const& other) const noexcept;
};

#endif // CURSOR_HPP
