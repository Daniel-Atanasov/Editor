#include "Hotkey.hpp"

#include <QDebug>

Hotkey::Hotkey(bool control, bool shift, bool alt, int key) :
    control(control),
    shift(shift),
    alt(alt),
    key(key)
{
}

Hotkey::Hotkey(int key) :
    control(false),
    shift(false),
    alt(false),
    key(key)
{
}

bool Hotkey::operator==(Hotkey other) const noexcept
{
    return
        (control == other.control) &&
        (shift   == other.shift) &&
        (alt     == other.alt) &&
        (key     == other.key);
}

bool Hotkey::operator!=(Hotkey other) const noexcept
{
    return !operator==(other);
}

HotkeyModifier::HotkeyModifier(bool control, bool shift, bool alt) :
    control(control),
    shift(shift),
    alt(alt)
{
}

HotkeyModifier HotkeyModifier::operator&(HotkeyModifier other)
{
    HotkeyModifier copy = *this;
    copy.control = control || other.control;
    copy.shift   = shift   || other.shift;
    copy.alt     = alt     || other.alt;
    return copy;
}

Hotkey HotkeyModifier::operator&(int key)
{
    return Hotkey(control, shift, alt, key);
}

HotkeyModifier Control(true, false, false);
HotkeyModifier Shift(false, true, false);
HotkeyModifier Alt(false, false, true);
