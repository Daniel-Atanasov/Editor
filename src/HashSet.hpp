#ifndef HASHSET_HPP
#define HASHSET_HPP

#include <unordered_set>

template <typename Type>
class HashSet : public std::unordered_set<Type>
{
public:
    HashSet() = default;
    HashSet(HashSet<Type> const&) = default;
    HashSet(HashSet<Type> &&) = default;

    HashSet(std::unordered_set<Type> const& other) : std::unordered_set<Type>(other)
    {
    }

    HashSet(std::unordered_set<Type> && other) : std::unordered_set<Type>(std::forward<std::unordered_set<Type>>(other))
    {
    }

    HashSet(std::initializer_list<Type> list) : std::unordered_set<Type>(list)
    {
    }

    template <typename T>
    HashSet(T begin, T end) : std::unordered_set<Type>(begin, end)
    {
    }

    HashSet<Type> & operator=(HashSet<Type> const&) = default;
    HashSet<Type> & operator=(HashSet<Type> &&) = default;

    HashSet<Type> & operator=(std::unordered_set<Type> const& other)
    {
        std::unordered_set<Type>::operator=(other);
        return *this;
    }

    HashSet<Type> & operator=(std::unordered_set<Type> && other)
    {
        std::unordered_set<Type>::operator=(std::forward<std::unordered_set<Type>>(other));
        return *this;
    }

    HashSet<Type> & operator=(std::initializer_list<Type> list)
    {
        std::unordered_set<Type>::operator=(list);
        return *this;
    }

    int size() const noexcept
    {
        return (int)std::unordered_set<Type>::size();
    }

    int count(Type const& key) const
    {
        return (int)std::unordered_set<Type>::count(key);
    }

    bool contains(Type const& key) const
    {
        return count(key) > 0;
    }
};

#endif // HASHSET_HPP
