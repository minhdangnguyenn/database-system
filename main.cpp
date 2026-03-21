#include <iostream>
#include <cassert>
#include "lru-cache.h"

void test_basic() {
    std::cout << "=== Test Basic ===" << std::endl;
    int CAP = 4;
    LRUCache cache(CAP);

    cache.put(1, 1);
    cache.put(2, 2);
    assert(cache.get(1) == 1);   // returns 1
    cache.put(3, 3);             // evicts key 2
    assert(cache.get(2) == -1);  // returns -1 (not found)
    cache.put(4, 4);             // evicts key 1
    assert(cache.get(1) == -1);  // returns -1 (not found)
    assert(cache.get(3) == 3);   // returns 3
    assert(cache.get(4) == 4);   // returns 4
    std::cout << "PASSED!" << std::endl;
}

void test_update_existing() {
    std::cout << "=== Test Update Existing Key ===" << std::endl;
    LRUCache cache(2);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(1, 99);            // update key 1 — should not evict
    assert(cache.get(1) == 99);  // returns updated value
    cache.put(3, 30);            // evicts key 2 (least recently used)
    assert(cache.get(2) == -1);  // returns -1 (evicted)
    assert(cache.get(3) == 30);  // returns 30
    std::cout << "PASSED!" << std::endl;
}

void test_capacity_one() {
    std::cout << "=== Test Capacity 1 ===" << std::endl;
    LRUCache cache(1);

    cache.put(1, 1);
    assert(cache.get(1) == 1);
    cache.put(2, 2);             // evicts key 1
    assert(cache.get(1) == -1);  // evicted
    assert(cache.get(2) == 2);
    std::cout << "PASSED!" << std::endl;
}

void test_get_updates_recency() {
    std::cout << "=== Test Get Updates Recency ===" << std::endl;
    LRUCache cache(2);

    cache.put(1, 1);
    cache.put(2, 2);
    cache.get(1);                // access key 1 — now most recently used
    cache.put(3, 3);             // should evict key 2, NOT key 1
    assert(cache.get(1) == 1);  // still exists
    assert(cache.get(2) == -1); // evicted
    assert(cache.get(3) == 3);
    std::cout << "PASSED!" << std::endl;
}

void test_miss() {
    std::cout << "=== Test Cache Miss ===" << std::endl;
    LRUCache cache(3);

    assert(cache.get(99) == -1); // never inserted
    cache.put(1, 100);
    assert(cache.get(2) == -1);  // not inserted
    std::cout << "PASSED!" << std::endl;
}

int main() {
    test_basic();
    test_update_existing();
    test_capacity_one();
    test_get_updates_recency();
    test_miss();

    std::cout << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << "   All tests passed! ✅        " << std::endl;
    std::cout << "================================" << std::endl;

    return 0;
}
