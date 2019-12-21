#ifndef CLIPBOARD_HPP
#define CLIPBOARD_HPP

#include "String32.hpp"

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

    String32 Text();
};

#endif // CLIPBOARD_HPP
