#ifndef TEXTVIEW_HPP
#define TEXTVIEW_HPP

#include "TextContainer.hpp"
#include "StringView32.hpp"
#include "String32.hpp"
#include "Vector.hpp"

class TextView : public TextContainer<TextView>
{
private:
    friend class TextContainer<TextView>;

    Vector<StringView32> lines;

public:
    TextView() = default;
    TextView(TextView const& other) = default;
    TextView(TextView && other) = default;

    TextView(char32_t const* text);
    TextView(StringView32 text);
    TextView(String32 const& text);
    TextView(Vector<StringView32> other);
    TextView(Vector<String32> const& other);

    TextView & operator=(TextView const& other) = default;
    TextView & operator=(TextView && other) = default;

    // TODO@Daniel:
    //  The following are mostly redundant, unless they get optmized

    TextView & operator=(char32_t const* text);
    TextView & operator=(StringView32 text);
    TextView & operator=(String32 const& text);
    TextView & operator=(Vector<StringView32> other);
    TextView & operator=(Vector<String32> const& other);
};

#endif // TEXTVIEW_HPP