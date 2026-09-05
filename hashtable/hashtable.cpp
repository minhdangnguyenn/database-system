#include "./include/hashtable.hpp"
#include <cmath>
#include <cstddef>
#include <cstring>
#include <vector>

constexpr double LOAD_FACTOR = 0.7;

template <typename K, typename V> HashTable<K, V>::HashTable(size_t capacity) {
    this->table.reserve(capacity);
}

static bool isPrime(int num) {
    if (num <= 1)
        return false; // Not prime
    for (int i = 2; i <= sqrt(num); i++) {
        if (num % i == 0)
            return false; // Found a divisor
    }
    return true; // It's prime
}

template <typename K, typename V>
bool HashTable<K, V>::insert(const K &key, const V &value) {
    // if full => resize
    if (this->capacity() * LOAD_FACTOR <= this->size()) {
        size_t new_capacity = this->capacity() * 2;
        // round up to the first prime numbers
        while (!isPrime(new_capacity)) {
            new_capacity++;
        }

        // resize is not enough here
        this->table.resize(new_capacity);
    }

    // if not full, just insert
    // check collision
    while (this->table[key])
        key++;
    this->table.insert(key, value);
};

template <typename K, typename V> size_t HashTable<K, V>::size() const {
    return this->table.size();
}

template <typename K, typename V> size_t HashTable<K, V>::capacity() const {
    return this->table.capacity();
}

template <typename K, typename V> bool HashTable<K, V>::erase(const K &key) {
    if (this->size() == 0)
        return false;
    this->table.erase(key);
    return true;
}
