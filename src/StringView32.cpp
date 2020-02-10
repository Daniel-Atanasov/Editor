#include "StringView32.hpp"

#include <algorithm>

#include "SpecialCharacters.hpp"
#include "String32.hpp"

StringView32::StringView32(std::u32string_view other) noexcept : std::u32string_view(other)
{
}

StringView32::StringView32(String32 const& other) noexcept : StringView32(other.data(), other.data() + other.size())
{
}

StringView32::StringView32(char32_t const* begin, char32_t const* end) noexcept : std::u32string_view(begin, std::distance(begin, end))
{
}

StringView32::StringView32(std::initializer_list<char32_t> list) noexcept : StringView32(std::begin(list), std::end(list))
{
}

int StringView32::size() const noexcept
{
    return (int)std::u32string_view::size();
}

int StringView32::tab_adjusted_size() const noexcept
{
    return adjust_for_tabs(size());
}

int StringView32::adjust_for_tabs(int pos) const noexcept
{
    int count = 0;
    for (int idx = 0; idx < pos; idx++)
    {
        if (at(idx) == U'\t')
        {
            count += TabWidth(count);
        }
        else
        {
            count++;
        }
    }
    return count;
}

int StringView32::from_tab_adjusted(int pos) const noexcept
{
    int count = 0;
    for (int idx = 0; idx < size(); idx++)
    {
        if (at(idx) == U'\t')
        {
            count += TabWidth(count);
        }
        else
        {
            count++;
        }

        if (count > pos) return idx;
    }
    return size();
}

int StringView32::index_of(char32_t ch, int start) const noexcept
{
    for (int idx = start; idx < size(); idx++)
    {
        if (at(idx) == ch) return idx;
    }
    return -1;
}

int StringView32::index_of_newline(int start) const noexcept
{
    for (int idx = start; idx < size(); idx++)
    {
        char32_t ch = at(idx);
        if (IsLineBreak(ch)) return idx;
    }
    return -1;
}

bool StringView32::contains(char32_t ch) const noexcept
{
    return find(ch) != npos;
}

bool StringView32::contains(StringView32 const& other) const noexcept
{
    return find(other) != npos;
}

StringView32 StringView32::middle(int idx) const noexcept
{
    return StringView32(data() + idx, data() + size());
}

StringView32 StringView32::middle(int idx, int count) const noexcept
{
    return StringView32(data() + idx, data() + std::min(idx + count, size()));
}

StringView32 StringView32::middle_view(int idx) const noexcept
{
    return middle(idx);
}

StringView32 StringView32::middle_view(int idx, int count) const noexcept
{
    return middle(idx, count);
}
