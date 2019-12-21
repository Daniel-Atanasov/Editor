#include "String32.hpp"

#include "SpecialCharacters.hpp"

String32::String32(std::u32string const& other) : std::u32string(other)
{
}

String32::String32(std::u32string && other) : std::u32string(std::forward<std::u32string>(other))
{
}

String32::String32(char32_t const* other) : std::u32string(other)
{
}

String32::String32(std::initializer_list<char32_t> list) : std::u32string(list)
{
}

String32 &String32::operator=(std::u32string const& other)
{
    std::u32string::operator=(other);
    return *this;
}

String32 &String32::operator=(std::u32string && other)
{
    std::u32string::operator=(std::forward<std::u32string>(other));
    return *this;
}

int String32::size() const noexcept
{
    return (int)std::u32string::size();
}

int String32::index_of(char32_t ch, int start) const noexcept
{
    for (int idx = start; idx < size(); idx++)
    {
        if (at(idx) == ch) return idx;
    }
    return -1;
}

int String32::index_of_newline(int start) const noexcept
{
    for (int idx = start; idx < size(); idx++)
    {
        char32_t ch = at(idx);
        if (IsLineBreak(ch)) return idx;
    }
    return -1;
}

bool String32::contains(char32_t ch) const noexcept
{
    return std::u32string::find(ch) != std::u32string::npos;
}

bool String32::contains(String32 const& other) const noexcept
{
    return std::u32string::find(other) != std::u32string::npos;
}

String32 String32::middle(int idx) const
{
    return String32(std::u32string::begin() + idx, std::u32string::end());
}

String32 String32::middle(int idx, int count) const
{
    return String32(std::u32string::begin() + idx, std::u32string::begin() + idx + count);
}

void String32::insert(int idx, char32_t ch, int count)
{
    std::u32string::insert(std::u32string::begin() + idx, count, ch);
}

void String32::push_front(char32_t ch)
{
    insert(0, ch);
}

void String32::pop_front()
{
    remove(0);
}

void String32::remove(int idx, int count)
{
    std::u32string::erase(std::u32string::begin() + idx, std::u32string::begin() + idx + count);
}
