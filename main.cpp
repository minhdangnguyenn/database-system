#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include "lru-cache.h"

// ─────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────
void pass(const std::string& name) {
    std::cout << "[PASS] " << name << std::endl;
}

void fail(const std::string& name) {
    std::cout << "[FAIL] " << name << std::endl;
}

// ─────────────────────────────────────────
//  BASIC TESTS
// ─────────────────────────────────────────
void test_basic() {
    LRUCache cache(2);
    cache.put(1, 1);
    cache.put(2, 2);
    assert(cache.get(1) == 1);
    cache.put(3, 3);             // evicts key 2
    assert(cache.get(2) == -1);
    cache.put(4, 4);             // evicts key 1
    assert(cache.get(1) == -1);
    assert(cache.get(3) == 3);
    assert(cache.get(4) == 4);
    pass("Basic");
}

void test_update_existing() {
    LRUCache cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(1, 99);
    assert(cache.get(1) == 99);
    cache.put(3, 30);
    assert(cache.get(2) == -1);
    assert(cache.get(3) == 30);
    pass("Update Existing");
}

void test_capacity_one() {
    LRUCache cache(1);
    cache.put(1, 1);
    assert(cache.get(1) == 1);
    cache.put(2, 2);
    assert(cache.get(1) == -1);
    assert(cache.get(2) == 2);
    pass("Capacity One");
}

void test_get_updates_recency() {
    LRUCache cache(2);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.get(1);
    cache.put(3, 3);
    assert(cache.get(1) == 1);
    assert(cache.get(2) == -1);
    assert(cache.get(3) == 3);
    pass("Get Updates Recency");
}

void test_miss() {
    LRUCache cache(3);
    assert(cache.get(99) == -1);
    cache.put(1, 100);
    assert(cache.get(2) == -1);
    pass("Cache Miss");
}

// ─────────────────────────────────────────
//  COMPLEX TESTS
// ─────────────────────────────────────────

// Many evictions in sequence
void test_many_evictions() {
    LRUCache cache(3);
    for (int i = 0; i < 100; i++) {
        cache.put(i, i * 10);
    }
    // Only last 3 should exist: 97, 98, 99
    assert(cache.get(99) == 990);
    assert(cache.get(98) == 980);
    assert(cache.get(97) == 970);
    assert(cache.get(96) == -1);  // evicted
    assert(cache.get(0)  == -1);  // evicted long ago
    pass("Many Evictions");
}

// Get should prevent eviction
void test_get_prevents_eviction() {
    LRUCache cache(3);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);

    cache.get(1);  // 1 is now MRU → order: 1, 3, 2
    cache.get(3);  // 3 is now MRU → order: 3, 1, 2

    cache.put(4, 4);  // evicts 2 (LRU)
    assert(cache.get(2) == -1);  // evicted
    assert(cache.get(1) == 1);   // still exists
    assert(cache.get(3) == 3);   // still exists
    assert(cache.get(4) == 4);   // just added
    pass("Get Prevents Eviction");
}

// Alternating puts and gets
void test_alternating_operations() {
    LRUCache cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    assert(cache.get(1) == 10);  // 1 is MRU
    cache.put(3, 30);
    cache.put(4, 40);            // evicts 2 (LRU)
    assert(cache.get(2) == -1);
    assert(cache.get(1) == 10);
    assert(cache.get(3) == 30);
    assert(cache.get(4) == 40);
    pass("Alternating Operations");
}

// Update same key many times
void test_repeated_updates() {
    LRUCache cache(2);
    for (int i = 0; i < 1000; i++) {
        cache.put(1, i);
    }
    assert(cache.get(1) == 999);  // last value
    pass("Repeated Updates");
}

// All keys same — no eviction should happen
void test_no_eviction_needed() {
    LRUCache cache(5);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);
    cache.put(5, 50);
    assert(cache.get(1) == 10);
    assert(cache.get(2) == 20);
    assert(cache.get(3) == 30);
    assert(cache.get(4) == 40);
    assert(cache.get(5) == 50);
    pass("No Eviction Needed");
}

// Large capacity stress test
void test_large_capacity() {
    int cap = 1000;
    LRUCache cache(cap);
    for (int i = 0; i < cap; i++) {
        cache.put(i, i * 2);
    }
    for (int i = 0; i < cap; i++) {
        assert(cache.get(i) == i * 2);
    }
    // Add one more — evicts key 0 (LRU after all gets moved recency)
    cache.put(cap, cap * 2);
    pass("Large Capacity");
}

// ─────────────────────────────────────────
//  BENCHMARK
// ─────────────────────────────────────────
void benchmark(const std::string& name, int capacity, int operations, int key_range) {
    LRUCache cache(capacity);
    std::mt19937 gen(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<int> keyDist(0, key_range - 1);
    std::uniform_int_distribution<int> opDist(0, 1);   // 0 = get, 1 = put
    std::uniform_int_distribution<int> valDist(0, 9999);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < operations; i++) {
        int key = keyDist(gen);
        if (opDist(gen) == 0) {
            cache.get(key);
        } else {
            cache.put(key, valDist(gen));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "[BENCH] " << name << std::endl;
    std::cout << "  Operations : " << operations << std::endl;
    std::cout << "  Capacity   : " << capacity << std::endl;
    std::cout << "  Key range  : " << key_range << std::endl;
    std::cout << "  Time       : " << ms << " ms" << std::endl;
    std::cout << "  Per op     : " << (ns / operations) << " ns" << std::endl;
    std::cout << std::endl;
}

// ─────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────
int main() {
    std::cout << "===============================" << std::endl;
    std::cout << "         BASIC TESTS           " << std::endl;
    std::cout << "===============================" << std::endl;
    test_basic();
    test_update_existing();
    test_capacity_one();
    test_get_updates_recency();
    test_miss();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "        COMPLEX TESTS          " << std::endl;
    std::cout << "===============================" << std::endl;
    test_many_evictions();
    test_get_prevents_eviction();
    test_alternating_operations();
    test_repeated_updates();
    test_no_eviction_needed();
    test_large_capacity();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "          BENCHMARKS           " << std::endl;
    std::cout << "===============================" << std::endl;
    benchmark("Small cache, high contention",  10,      1000000, 20);
    benchmark("Medium cache, normal use",      1000,    1000000, 2000);
    benchmark("Large cache, low eviction",     100000,  1000000, 100000);
    benchmark("Large cache, high eviction",    100,     1000000, 100000);

    std::cout << "===============================" << std::endl;
    std::cout << "     All tests passed! ✅      " << std::endl;
    std::cout << "===============================" << std::endl;

    return 0;
}
