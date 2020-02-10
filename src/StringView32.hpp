#ifndef STRINGVIEW32_HPP
#define STRINGVIEW32_HPP

#include <string_view>

class String32;
class StringView32 : public std::u32string_view
{
public:
    StringView32() noexcept = default;
    StringView32(StringView32 const&) noexcept = default;
    StringView32(StringView32 &&) noexcept = default;

    using std::u32string_view::u32string_view;

    StringView32(std::u32string_view other) noexcept;
    StringView32(String32 const& other) noexcept;
    StringView32(char32_t const* begin, char32_t const* end) noexcept;
    StringView32(std::initializer_list<char32_t> list) noexcept;

    StringView32 & operator=(StringView32 const& other) noexcept = default;
    StringView32 & operator=(StringView32 && other) noexcept = default;

    using std::u32string_view::operator=;

    int size() const noexcept;
    int tab_adjusted_size() const noexcept;

    int adjust_for_tabs(int pos) const noexcept;
    int from_tab_adjusted(int pos) const noexcept;

    int index_of(char32_t ch, int start = 0) const noexcept;
    int index_of_newline(int start = 0) const noexcept;

    bool contains(char32_t ch) const noexcept;
    bool contains(StringView32 const& other) const noexcept;

    StringView32 middle(int idx) const noexcept;
    StringView32 middle(int idx, int count) const noexcept;

    StringView32 middle_view(int idx) const noexcept;
    StringView32 middle_view(int idx, int count) const noexcept;
};

template <typename Type>
StringView32 ToStringView32(Type value)
{
    union Helper
    {
        char32_t u32[64];
        char u8[64];
    };

    static Helper h;

    std::to_chars_result res = std::to_chars(std::begin(h.u8), std::end(h.u8), value);
    int size = std::distance(h.u8, res.ptr);

    for (int idx = size - 1; idx >= 0; idx--)
    {
        h.u32[idx] = h.u8[idx];
    }

    return StringView32(h.u32, size);
}

namespace std
{
    template <>
    struct hash<StringView32>
    {
        auto operator()(StringView32 const& other) const
        {
            return std::hash<std::u32string_view>()(other);
        }
    };
}

#endif // STRINGVIEW32_HPP
