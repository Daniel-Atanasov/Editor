#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <vector>

template <typename Type>
class Vector : public std::vector<Type>
{
public:
    Vector() = default;
    Vector(Vector<Type> const&) = default;
    Vector(Vector<Type> &&) = default;

    explicit Vector(std::size_t size) : std::vector<Type>(size)
    {
    }

    Vector(std::vector<Type> const& other) : std::vector<Type>(other)
    {
    }

    Vector(std::vector<Type> && other) : std::vector<Type>(std::forward<std::vector<Type>>(other))
    {
    }

    Vector(std::initializer_list<Type> list) : std::vector<Type>(list)
    {
    }

    template <typename T>
    Vector(T begin, T end) : std::vector<Type>(begin, end)
    {
    }

    Vector<Type> & operator=(Vector const&) = default;
    Vector<Type> & operator=(Vector &&) = default;

    Vector<Type> & operator=(std::vector<Type> const& other)
    {
        std::vector<Type>::operator=(other);
        return *this;
    }

    Vector<Type> & operator=(std::vector<Type> && other)
    {
        std::vector<Type>::operator=(std::forward<std::vector<Type>>(other));
        return *this;
    }

    Vector<Type> & operator=(std::initializer_list<Type> list)
    {
        std::vector<Type>::operator=(list);
        return *this;
    }

    Vector<Type> & operator+=(Vector<Type> const& other)
    {
        for (Type const& item : other)
        {
            Vector<Type>::push_back(item);
        }
        return *this;
    }

    Vector<Type> & operator+=(Vector<Type> && other)
    {
        for (Type & item : other)
        {
            Vector<Type>::push_back(std::move(item));
        }
        return *this;
    }

    Vector<Type> & operator+=(std::vector<Type> const& other)
    {
        for (Type const& item : other)
        {
            Vector<Type>::push_back(item);
        }
        return *this;
    }

    Vector<Type> & operator+=(std::vector<Type> && other)
    {
        for (Type & item : other)
        {
            Vector<Type>::push_back(std::move(item));
        }
        return *this;
    }

    int size() const noexcept
    {
        return (int)std::vector<Type>::size();
    }

    Vector<Type> middle(int idx) const
    {
        return Vector<Type>(std::vector<Type>::begin() + idx, std::vector<Type>::end());
    }

    Vector<Type> middle(int idx, int count) const
    {
        return Vector<Type>(std::vector<Type>::begin() + idx, std::vector<Type>::begin() + idx + count);
    }

    using std::vector<Type>::insert;

    void insert(int idx, Type const& item, int count = 1)
    {
        std::vector<Type>::insert(std::vector<Type>::begin() + idx, count, item);
    }

    void insert(int idx, Type && item, int count = 1)
    {
        std::vector<Type>::insert(std::vector<Type>::begin() + idx, count, std::forward<Type>(item));
    }

    void push_front(Type const& item)
    {
        insert(0, item);
    }

    void push_front(Type && item)
    {
        insert(0, std::forward<Type>(item));
    }

    void pop_front()
    {
        remove(0);
    }

    void remove(int idx, int count = 1)
    {
        std::vector<Type>::erase(std::vector<Type>::begin() + idx, std::vector<Type>::begin() + idx + count);
    }
};

#endif // VECTOR_HPP
