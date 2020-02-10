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

    using std::vector<Type>::vector;
    
    Vector<Type> & operator=(Vector const&) = default;
    Vector<Type> & operator=(Vector &&) = default;

    using std::vector<Type>::operator=;

    Vector<Type> & operator+=(std::vector<Type> const& other)
    {
        reserve(size() + other.size());

        for (Type const& item : other)
        {
            Vector<Type>::push_back(item);
        }

        return *this;
    }

    Vector<Type> & operator+=(std::vector<Type> && other)
    {
        reserve(size() + other.size());

        for (Type & item : other)
        {
            Vector<Type>::push_back(std::move(item));
        }

        return *this;
    }

    Vector<Type> & operator+=(std::initializer_list<Type> other)
    {
        reserve(size() + other.size());

        for (Type const& item : other)
        {
            Vector<Type>::push_back(item);
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
