#ifndef CLIPBOARD_HPP
#define CLIPBOARD_HPP

#include "String32.hpp"
#include "Vector.hpp"

class Clipboard
{
private:
    Clipboard();

    Clipboard(Clipboard const& other) = delete;
    Clipboard(Clipboard && other) = delete;

    Clipboard & operator=(Clipboard const& other) = delete;
    Clipboard & operator=(Clipboard && other) = delete;

public:
    static Clipboard & Instance();

    String32 Text() const;
};

#endif // CLIPBOARD_HPP
