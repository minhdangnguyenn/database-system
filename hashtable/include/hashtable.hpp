#include <cstddef>
#include <vector>

template <typename K, typename V> struct Entry {
    K key;
    V value;
    bool is_occupied;
};

template <typename K, typename V> class HashTable {
private:
    std::vector<Entry<K, V>> table;

public:
    HashTable(size_t capacity);
    bool insert(const K &key, const V &value);
    bool erase(const K &key);
    size_t size() const;
    size_t capacity() const;
    bool empty();
};
