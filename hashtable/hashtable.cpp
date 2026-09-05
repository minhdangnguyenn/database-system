#include "./include/hashtable.hpp"
#include <vector>

template <typename K, typename V> HashTable<K, V>::HashTable(size_t capacity) {
    this->table.reserve(capacity);
}

template <typename K, typename V>
bool HashTable<K, V>::insert(const K &key, const V &value) {
    Entry entry = new Entry<K, V>(key, value);

    // TODO: what if capacity is full (size == capacity)
    this->table.insert(entry);
};
