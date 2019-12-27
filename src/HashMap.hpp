#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <unordered_map>

template <typename KeyType, typename ValueType>
class HashMap : public std::unordered_map<KeyType, ValueType>
{
private:
    using PairType = std::pair<KeyType const, ValueType>;

public:
    HashMap() = default;
    HashMap(HashMap<KeyType, ValueType> const&) = default;
    HashMap(HashMap<KeyType, ValueType> &&) = default;

    HashMap(std::unordered_map<KeyType,  ValueType> const& other) : std::unordered_map<KeyType, ValueType>(other)
    {
    }

    HashMap(std::unordered_map<KeyType, ValueType> && other) : std::unordered_map<KeyType, ValueType>(std::forward<std::unordered_map<Type>>(other))
    {
    }

    HashMap(std::initializer_list<PairType> list) : std::unordered_map<KeyType, ValueType>(list)
    {
    }

    template <typename T>
    HashMap(T begin, T end) : std::unordered_map<KeyType, ValueType>(begin, end)
    {
    }

    HashMap<KeyType, ValueType> & operator=(HashMap<KeyType, ValueType> const&) = default;
    HashMap<KeyType, ValueType> & operator=(HashMap<KeyType, ValueType> &&) = default;

    HashMap<KeyType, ValueType> & operator=(std::unordered_map<KeyType, ValueType> const& other)
    {
        std::unordered_map<KeyType, ValueType>::operator=(other);
        return *this;
    }

    HashMap<KeyType, ValueType> & operator=(std::unordered_map<KeyType, ValueType> && other)
    {
        std::unordered_map<KeyType, ValueType>::operator=(std::forward<std::unordered_map<KeyType, ValueType>>(other));
        return *this;
    }

    HashMap<KeyType, ValueType> & operator=(std::initializer_list<PairType> list)
    {
        std::unordered_map<KeyType, ValueType>::operator=(list);
        return *this;
    }

    ValueType & operator[](KeyType const& key)
    {
        return std::unordered_map<KeyType, ValueType>::operator[](key);
    }

    ValueType const& operator[](KeyType const& key) const
    {
        return std::unordered_map<KeyType, ValueType>::at(key);
    }

    int size() const noexcept
    {
        return (int)std::unordered_map<KeyType, ValueType>::size();
    }

    int count(KeyType const& key) const
    {
        return (int)std::unordered_map<KeyType, ValueType>::count(key);
    }

    bool contains(KeyType const& key) const
    {
        return count(key) > 0;
    }

	void remove(KeyType const& key)
	{
		erase(key);
	}
};

#endif // HASHMAP_HPP
