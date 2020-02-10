#include "Cursor.hpp"

bool Position::operator==(Position other) const noexcept
{
    return x == other.x && y == other.y;
}

bool Position::operator!=(Position other) const noexcept
{
    return !operator==(other);
}

bool Position::operator<(Position other) const noexcept
{
    return y < other.y || (y == other.y && x < other.x);
}

bool Position::operator<=(Position other) const noexcept
{
    return operator==(other) || operator<(other);
}

bool Position::operator>(Position other) const noexcept
{
    return !operator<=(other);
}

bool Position::operator>=(Position other) const noexcept
{
    return !operator<(other);
}

bool Cursor::operator==(Cursor other) const noexcept
{
    return start == other.start && stop == other.stop;
}

bool Cursor::operator!=(Cursor other) const noexcept
{
    return !operator==(other);
}