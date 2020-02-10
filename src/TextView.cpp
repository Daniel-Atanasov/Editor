#include "TextView.hpp"

TextView::TextView(char32_t const* text) : TextView(StringView32(text))
{
}

TextView::TextView(StringView32 text) : lines(1)
{
    int line_idx = 0;
    int start = 0;
    while (start < text.size())
    {
        int stop = text.index_of_newline(start);
        if (stop == -1)
        {
            lines[line_idx++] = text.middle_view(start);
            break;
        }
        
        lines[line_idx++] = text.middle_view(start, stop - start);
        lines.resize(lines.size() + 1);
        
        start = stop + 1;
    }
}

TextView::TextView(String32 const& text) : TextView((StringView32)text)
{
}

TextView::TextView(Vector<StringView32> other) : lines(std::move(other))
{
}

TextView::TextView(Vector<String32> const& other) : lines(other.size())
{
    for (int idx = 0; idx < other.size(); idx++)
    {
        lines[idx] = other[idx];
    }
}

TextView & TextView::operator=(char32_t const* text)
{
    return *this = TextView(text);
}

TextView & TextView::operator=(StringView32 text)
{
    return *this = TextView(text);
}

TextView & TextView::operator=(String32 const& text)
{
    return *this = TextView(text);
}

TextView & TextView::operator=(Vector<StringView32> other)
{
    return *this = TextView(std::move(other));
}

TextView & TextView::operator=(Vector<String32> const& other)
{
    return *this = TextView(other);
}
