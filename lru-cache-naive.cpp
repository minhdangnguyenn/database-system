#include "lru-cache-naive.h"

LRUCacheNaive::LRUCacheNaive(int capacity) {
    this->capacity = capacity;
}

int LRUCacheNaive::get(int key) {
    // Loop through to find the key — O(n)
    for (int i = 0; i < cache.size(); i++) {
        if (cache[i].getKey() == key) {
            int value = cache[i].getValue();

            // Move to front (MRU) — O(n)
            cache.erase(cache.begin() + i);
            cache.insert(cache.begin(), Page(key, value));

            return value;
        }
    }
    return -1; // not found
}

void LRUCacheNaive::put(int key, int value) {
    // Check if key already exists — O(n)
    for (int i = 0; i < cache.size(); i++) {
        if (cache[i].getKey() == key) {
            // Update value and move to front
            cache.erase(cache.begin() + i);
            cache.insert(cache.begin(), Page(key, value));
            return;
        }
    }

    // If cache is full — evict LRU (last element)
    if (cache.size() == capacity) {
        cache.pop_back(); // evict LRU — O(1)
    }

    // Add new page to front
    cache.insert(cache.begin(), Page(key, value));
}
