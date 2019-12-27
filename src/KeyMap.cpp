#include "KeyMap.hpp"

FunctionWrapper::FunctionWrapper(std::function<int()> func) : function(std::move(func))
{
}

FunctionWrapper::FunctionWrapper(std::function<void()> func) :
    function([f = std::move(func)] { f(); return 0; })
{
}

FunctionWrapper & FunctionWrapper::operator=(std::function<int()> func)
{
    function = std::move(func);
    return *this;
}

FunctionWrapper & FunctionWrapper::operator=(std::function<void()> func)
{
    function = [f = std::move(func)] { f(); return 0; };
    return *this;
}

int FunctionWrapper::operator()() const
{
    return function();
}

FunctionWrapper::operator bool() const
{
    return (bool)function;
}

FunctionWrapper & KeyMap::operator[](Hotkey const& hotkey)
{
    return keymap[hotkey];
}

FunctionWrapper const& KeyMap::operator[](Hotkey const& hotkey) const
{
    return keymap[hotkey];
}

bool KeyMap::contains(Hotkey const& hotkey)
{
    return keymap.contains(hotkey);
}
