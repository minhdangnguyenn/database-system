#include "./include/hashtable.hpp"
#include <cmath>
#include <cstring>
#include <optional>
#include <vector>

constexpr double LOAD_FACTOR = 0.7;

template <typename K, typename V> HashTable<K, V>::HashTable(size_t capacity) {
    this->table.resize(capacity);
    // for (int i = 0; i < capacity; i++) {
    //     this->table[i] = {nullptr, nullptr};
    // }
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
__attribute__((hot)) inline size_t HashTable<K, V>::hashfunction(const K &key) {
    // convert key into a number that can be used to hash
    // if key is smth like str, it does not support mod capacity
    return std::hash<K>{}(key) % this->capacity();
}

template <typename K, typename V>
bool HashTable<K, V>::insert(const K &key, const V &value) {
    // if full => resize
    if (this->capacity() * LOAD_FACTOR <= this->size()) {

        // resize is not enough here, need to rehash existing entries
        std::vector<std::optional<std::pair<K, V>>> existing_entries;
        for (auto i = 0; i < this->table.size(); i++) {
            if (this->table[i]) {
                existing_entries.push_back(this->table[i]);
            }
        }
        // resize is not enough here
        size_t new_capacity = this->capacity() * 2;
        // round up to the first prime numbers
        while (!isPrime(new_capacity)) {
            new_capacity++;
        }
        // this->table.resize(new_capacity);
        std::vector<std::optional<std::pair<K, V>>> new_table(new_capacity);
        for (int i = 0; i < existing_entries.size(); i++) {
            auto idx = this->hashfunction(existing_entries[i]->first);
            // collision handle
            while (this->table[idx]) {
                idx = (idx + 1) % new_capacity;
            }
            new_table[idx]->first = existing_entries[i]->first;
            new_table[idx]->second = existing_entries[i]->second;
        }
    }

    // then insert
    // check collision
    size_t idx = this->hashfunction(key);

    // check whether collision happens
    while (idx < this->table.size()) {
        idx++;
    }
    this->table[idx] = {key, value};
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

    int idx;
    for (int i = 0; i < this->table.size(); i++) {
        if (this->table[i]->first == key) {
            idx = i;
        }
    }

    this->table.erase(this->table.begin() + idx);

    return true;
}
