#ifndef KEYMAP_HPP
#define KEYMAP_HPP

#include "HashMap.hpp"
#include "Hotkey.hpp"

class FunctionWrapper
{
private:
    std::function<int()> function;

public:
    FunctionWrapper() = default;
    FunctionWrapper(FunctionWrapper const& other) = default;
    FunctionWrapper(FunctionWrapper && other) = default;

    FunctionWrapper(std::function<int()> func);
    FunctionWrapper(std::function<void()> func);

    FunctionWrapper & operator=(FunctionWrapper const& other) = default;
    FunctionWrapper & operator=(FunctionWrapper && other) = default;

    FunctionWrapper & operator=(std::function<int()> func);
    FunctionWrapper & operator=(std::function<void()> func);

    template <typename Type>
    FunctionWrapper & operator=(Type func)
    {
        using FunctionType = std::function<std::result_of_t<Type()>()>;
        return operator=((FunctionType)func);
    }

    int operator()() const;

    explicit operator bool() const;
};

class KeyMap
{
private:
    HashMap<Hotkey, FunctionWrapper> keymap;

public:
    FunctionWrapper & operator[](Hotkey const& hotkey);
    FunctionWrapper const& operator[](Hotkey const& hotkey) const;

    bool contains(Hotkey const& hotkey);
};

#endif // KEYMAP_HPP