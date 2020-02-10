#ifndef CLIPBOARD_HPP
#define CLIPBOARD_HPP

#include "String32.hpp"
#include "Vector.hpp"

class Clipboard
{
private:
    Vector<String32> text;

    Clipboard();

    Clipboard(Clipboard const& other) = delete;
    Clipboard(Clipboard && other) = delete;

    Clipboard & operator=(Clipboard const& other) = delete;
    Clipboard & operator=(Clipboard && other) = delete;

public:
    static Clipboard & Instance();

    String32 Text() const;
    Vector<String32> const& MultiText() const;

    void SetText(StringView32 txt);
    void SetMultiText(Vector<String32> txt);
};

#endif // CLIPBOARD_HPP
