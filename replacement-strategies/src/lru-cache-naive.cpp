#include "../include/lru-cache-naive.h"

// O(n) — linear scan to remove from eviction candidates
void LRUReplacerNaive::pin(Page *page) {
    for (int i = 0; i < (int)cache.size(); i++) {
        if (cache[i]->getKey() == page->getKey()) {
            cache.erase(cache.begin() + i);
            return;
        }
    }
}

// O(n) — linear scan to avoid duplicates, then insert at front
void LRUReplacerNaive::unpin(Page *page) {
    for (int i = 0; i < (int)cache.size(); i++) {
        if (cache[i]->getKey() == page->getKey()) {
            return; // already a candidate
        }
    }
    cache.insert(cache.begin(), page); // MRU at front
}

// O(1) — LRU is always at the back
Page *LRUReplacerNaive::evict() {
    if (cache.empty())
        return nullptr;

    Page *victim = cache.back();
    cache.pop_back();
    return victim;
}
