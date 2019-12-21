#ifndef HOTKEY_HPP
#define HOTKEY_HPP

#include <functional>

struct Hotkey
{
    bool control;
    bool shift;
    bool alt;

    int key;

    Hotkey(bool control, bool shift, bool alt, int key);
    Hotkey(int key);

    bool operator==(Hotkey other) const noexcept;
    bool operator!=(Hotkey other) const noexcept;
};


struct HotkeyModifier
{
private:
    bool control;
    bool shift;
    bool alt;

public:

    HotkeyModifier(bool control, bool shift, bool alt);

    HotkeyModifier operator&(HotkeyModifier other);

    Hotkey operator&(int key);
};

extern HotkeyModifier Control;
extern HotkeyModifier Shift;
extern HotkeyModifier Alt;

namespace std
{
    template <>
    struct hash<Hotkey>
    {
        std::size_t operator()(Hotkey hotkey) const noexcept
        {
            std::size_t hash = hotkey.key;
            hash <<= 32;
            hash |= ((int)hotkey.control << 2);
            hash |= ((int)hotkey.shift << 1);
            hash |= (int)hotkey.alt;
            return hash;
        }
    };
}

#endif // HOTKEY_HPP
