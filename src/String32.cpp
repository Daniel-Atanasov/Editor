#include "String32.hpp"

#include <algorithm>

#include "SpecialCharacters.hpp"

String32::String32(std::u32string const& other) : std::u32string(other)
{
}

String32::String32(std::u32string && other) : std::u32string(std::forward<std::u32string>(other))
{
}

String32::String32(StringView32 other) : std::u32string(other)
{
}

int String32::size() const noexcept
{
    return (int)std::u32string::size();
}

int String32::tab_adjusted_size() const noexcept
{
    StringView32 view = *this;
    return view.tab_adjusted_size();
}

int String32::adjust_for_tabs(int pos) const noexcept
{
    StringView32 view = *this;
    return view.adjust_for_tabs(pos);
}

int String32::from_tab_adjusted(int pos) const noexcept
{
    StringView32 view = *this;
    return view.from_tab_adjusted(pos);
}

int String32::index_of(char32_t ch, int start) const noexcept
{
    StringView32 view = *this;
    return view.index_of(ch, start);
}

int String32::index_of_newline(int start) const noexcept
{
    StringView32 view = *this;
    return view.index_of_newline(start);
}

bool String32::contains(char32_t ch) const noexcept
{
    StringView32 view = *this;
    return view.contains(ch);
}

bool String32::contains(StringView32 other) const noexcept
{
    StringView32 view = *this;
    return view.contains(other);
}

String32 String32::middle(int idx) const
{
    return String32(std::u32string::begin() + idx, std::u32string::end());
}

String32 String32::middle(int idx, int count) const
{
    return String32(begin() + idx, begin() + std::min(idx + count, size()));
}

StringView32 String32::middle_view(int idx) const noexcept
{
    StringView32 view = *this;
    return view.middle(idx);
}

StringView32 String32::middle_view(int idx, int count) const noexcept
{
    StringView32 view = *this;
    return view.middle(idx, count);
}

void String32::insert(int idx, StringView32 text)
{
    std::u32string::insert(idx, text.data(), text.size());
}

void String32::insert(int idx, char32_t ch, int count)
{
    std::u32string::insert(begin() + idx, count, ch);
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
    erase(begin() + idx, begin() + idx + count);
}
