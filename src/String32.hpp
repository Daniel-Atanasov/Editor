#ifndef STRING32_HPP
#define STRING32_HPP

#include <string>

class String32 : public std::u32string
{
public:
    String32() = default;
    String32(String32 const&) = default;
    String32(String32 &&) = default;

    String32(std::u32string const& other);
    String32(std::u32string && other);

    String32(char32_t const* other);

    String32(std::initializer_list<char32_t> list);

    template <typename T>
    String32(T begin, T end) : std::u32string(begin, end)
    {
    }

    String32 & operator=(String32 const&) = default;
    String32 & operator=(String32 &&) = default;

    String32 & operator=(std::u32string const& other);
    String32 & operator=(std::u32string && other);

    int size() const noexcept;

    int index_of(char32_t ch, int start = 0) const noexcept;

    int index_of_newline(int start = 0) const noexcept;

    bool contains(char32_t ch) const noexcept;
    bool contains(String32 const& other) const noexcept;

    String32 middle(int idx) const;
    String32 middle(int idx, int count) const;

    void insert(int idx, char32_t ch, int count = 1);

    void push_front(char32_t ch);
    void pop_front();

    void remove(int idx, int count = 1);
};

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
