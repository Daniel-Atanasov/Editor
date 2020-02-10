#ifndef STRING32_HPP
#define STRING32_HPP

#include <string>
#include <charconv>

#include "StringView32.hpp"

class String32 : public std::u32string
{
public:
    String32() = default;
    String32(String32 const&) = default;
    String32(String32 &&) = default;

    using std::u32string::u32string;

    String32(std::u32string const& other);
    String32(std::u32string && other);

    String32(StringView32 other);

    String32 & operator=(String32 const& other) = default;
    String32 & operator=(String32 && other) = default;

    using std::u32string::operator=;

    int size() const noexcept;
    int tab_adjusted_size() const noexcept;

    int adjust_for_tabs(int pos) const noexcept;
    int from_tab_adjusted(int pos) const noexcept;

    int index_of(char32_t ch, int start = 0) const noexcept;
    int index_of_newline(int start = 0) const noexcept;

    bool contains(char32_t ch) const noexcept;
    bool contains(StringView32 other) const noexcept;

    String32 middle(int idx) const;
    String32 middle(int idx, int count) const;

    StringView32 middle_view(int idx) const noexcept;
    StringView32 middle_view(int idx, int count) const noexcept;

    void insert(int idx, StringView32 text);
    void insert(int idx, char32_t ch, int count = 1);

    void push_front(char32_t ch);
    void pop_front();

    void remove(int idx, int count = 1);
};

template <typename Type>
String32 ToString32(Type value)
{
    union Helper
    {
        char32_t u32[64];
        char u8[64];
    };

    Helper h;

    std::to_chars_result res = std::to_chars(std::begin(h.u8), std::end(h.u8), value);
    int size = std::distance(h.u8, res.ptr);

    for (int idx = size - 1; idx >= 0; idx--)
    {
        h.u32[idx] = h.u8[idx];
    }

    return String32(h.u32, size);
}

namespace std
{
    template <>
    struct hash<String32>
    {
        auto operator()(String32 const& other) const
        {
            return std::hash<std::u32string>()(other);
        }
    };
}

#endif // STRING32_HPP
