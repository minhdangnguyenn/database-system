#include <cstring>
#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include "buffer-pool.h"
#include "disk-manager.h"
#include "lru-cache-naive.h"
#include "test-data.h"

// THIS BENCHMARK IS FOR LRU-CACHE, I HAVENT CHANGED IT
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
    BufferPool cache(2);
    cache.pin(1, 1);
    cache.pin(2, 2);
    assert(cache.get(1) == 1);
    cache.pin(3, 3);
    assert(cache.get(2) == -1);
    cache.pin(4, 4);
    assert(cache.get(1) == -1);
    assert(cache.get(3) == 3);
    assert(cache.get(4) == 4);
    pass("Basic");
}

void test_update_existing() {
    BufferPool cache(2);
    cache.pin(1, 10);
    cache.pin(2, 20);
    cache.pin(1, 99);
    assert(cache.get(1) == 99);
    cache.pin(3, 30);
    assert(cache.get(2) == -1);
    assert(cache.get(3) == 30);
    pass("Update Existing");
}

void test_capacity_one() {
    BufferPool cache(1);
    cache.pin(1, 1);
    assert(cache.get(1) == 1);
    cache.pin(2, 2);
    assert(cache.get(1) == -1);
    assert(cache.get(2) == 2);
    pass("Capacity One");
}

void test_get_updates_recency() {
    BufferPool cache(2);
    cache.pin(1, 1);
    cache.pin(2, 2);
    cache.get(1);
    cache.pin(3, 3);
    assert(cache.get(1) == 1);
    assert(cache.get(2) == -1);
    assert(cache.get(3) == 3);
    pass("Get Updates Recency");
}

void test_miss() {
    BufferPool cache(3);
    assert(cache.get(99) == -1);
    cache.pin(1, 100);
    assert(cache.get(2) == -1);
    pass("Cache Miss");
}

// ─────────────────────────────────────────
//  COMPLEX TESTS
// ─────────────────────────────────────────
void test_many_evictions() {
    BufferPool cache(3);
    for (int i = 0; i < 100; i++) {
        cache.pin(i, i * 10);
    }
    assert(cache.get(99) == 990);
    assert(cache.get(98) == 980);
    assert(cache.get(97) == 970);
    assert(cache.get(96) == -1);
    assert(cache.get(0)  == -1);
    pass("Many Evictions");
}

void test_get_prevents_eviction() {
    BufferPool cache(3);
    cache.pin(1, 1);
    cache.pin(2, 2);
    cache.pin(3, 3);
    cache.get(1);
    cache.get(3);
    cache.pin(4, 4);
    assert(cache.get(2) == -1);
    assert(cache.get(1) == 1);
    assert(cache.get(3) == 3);
    assert(cache.get(4) == 4);
    pass("Get Prevents Eviction");
}

void test_alternating_operations() {
    BufferPool cache(3);
    cache.pin(1, 10);
    cache.pin(2, 20);
    assert(cache.get(1) == 10);
    cache.pin(3, 30);
    cache.pin(4, 40);
    assert(cache.get(2) == -1);
    assert(cache.get(1) == 10);
    assert(cache.get(3) == 30);
    assert(cache.get(4) == 40);
    pass("Alternating Operations");
}

void test_repeated_updates() {
    BufferPool cache(2);
    for (int i = 0; i < 1000; i++) {
        cache.pin(1, i);
    }
    assert(cache.get(1) == 999);
    pass("Repeated Updates");
}

void test_no_eviction_needed() {
    BufferPool cache(5);
    cache.pin(1, 10);
    cache.pin(2, 20);
    cache.pin(3, 30);
    cache.pin(4, 40);
    cache.pin(5, 50);
    assert(cache.get(1) == 10);
    assert(cache.get(2) == 20);
    assert(cache.get(3) == 30);
    assert(cache.get(4) == 40);
    assert(cache.get(5) == 50);
    pass("No Eviction Needed");
}

void test_large_capacity() {
    int cap = 1000;
    BufferPool cache(cap);
    for (int i = 0; i < cap; i++) {
        cache.pin(i, i * 2);
    }
    for (int i = 0; i < cap; i++) {
        assert(cache.get(i) == i * 2);
    }
    cache.pin(cap, cap * 2);
    pass("Large Capacity");
}

// ─────────────────────────────────────────
//  STRESS TESTS (NEW — larger sizes)
// ─────────────────────────────────────────
void test_stress_evictions() {
    BufferPool cache(100);
    for (int i = 0; i < 10000; i++) {
        cache.pin(i, i * 3);
    }
    // only last 100 should remain: 9900..9999
    for (int i = 9900; i < 10000; i++) {
        assert(cache.get(i) == i * 3);
    }
    assert(cache.get(9899) == -1);
    assert(cache.get(0)    == -1);
    pass("Stress Evictions");
}

void test_stress_repeated_updates() {
    BufferPool cache(10);
    for (int i = 0; i < 100000; i++) {
        cache.pin(i % 10, i);
    }
    // key k's last value = last i where i%10==k = 99990+k
    for (int k = 0; k < 10; k++) {
        assert(cache.get(k) == 99990 + k);
    }
    pass("Stress Repeated Updates");
}

void test_stress_mixed() {
    BufferPool cache(500);
    // fill cache
    for (int i = 0; i < 500; i++) {
        cache.pin(i, i * 7);
    }
    // read half to make them MRU
    for (int i = 0; i < 250; i++) {
        assert(cache.get(i) == i * 7);
    }
    // insert 250 new keys — should evict keys 250..499
    for (int i = 500; i < 750; i++) {
        cache.pin(i, i * 7);
    }
    // keys 0..249 should still be there (accessed recently)
    for (int i = 0; i < 250; i++) {
        assert(cache.get(i) == i * 7);
    }
    // keys 250..499 should be evicted
    for (int i = 250; i < 500; i++) {
        assert(cache.get(i) == -1);
    }
    // keys 500..749 should be there
    for (int i = 500; i < 750; i++) {
        assert(cache.get(i) == i * 7);
    }
    pass("Stress Mixed");
}

void test_stress_large_capacity() {
    int cap = 100000;
    BufferPool cache(cap);
    for (int i = 0; i < cap; i++) {
        cache.pin(i, i * 5);
    }
    for (int i = 0; i < cap; i++) {
        assert(cache.get(i) == i * 5);
    }
    // evict oldest
    for (int i = cap; i < cap + 1000; i++) {
        cache.pin(i, i * 5);
    }
    for (int i = 0; i < 1000; i++) {
        assert(cache.get(i) == -1);
    }
    pass("Stress Large Capacity");
}

// ─────────────────────────────────────────
//  BENCHMARK — OPTIMIZED
// ─────────────────────────────────────────
void benchmark(const std::string& name, int capacity, int operations, int key_range) {
    BufferPool cache(capacity);
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> keyDist(0, key_range - 1);
    std::uniform_int_distribution<int> opDist(0, 1);
    std::uniform_int_distribution<int> valDist(0, 9999);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < operations; i++) {
        int key = keyDist(gen);
        if (opDist(gen) == 0) cache.get(key);
        else cache.pin(key, valDist(gen));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "[O(1)]  " << name << std::endl;
    std::cout << "  Operations : " << operations << std::endl;
    std::cout << "  Capacity   : " << capacity << std::endl;
    std::cout << "  Key range  : " << key_range << std::endl;
    std::cout << "  Time       : " << ms << " ms" << std::endl;
    std::cout << "  Per op     : " << (ns / operations) << " ns" << std::endl;
    std::cout << std::endl;
}

// ─────────────────────────────────────────
//  BENCHMARK — NAIVE
// ─────────────────────────────────────────
void benchmark_naive(const std::string& name, int capacity, int operations, int key_range) {
    LRUCacheNaive cache(capacity);
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> keyDist(0, key_range - 1);
    std::uniform_int_distribution<int> opDist(0, 1);
    std::uniform_int_distribution<int> valDist(0, 9999);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < operations; i++) {
        int key = keyDist(gen);
        if (opDist(gen) == 0) cache.get(key);
        else cache.put(key, valDist(gen));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "[NAIVE] " << name << std::endl;
    std::cout << "  Operations : " << operations << std::endl;
    std::cout << "  Capacity   : " << capacity << std::endl;
    std::cout << "  Key range  : " << key_range << std::endl;
    std::cout << "  Time       : " << ms << " ms" << std::endl;
    std::cout << "  Per op     : " << (ns / operations) << " ns" << std::endl;
    std::cout << std::endl;
}

// ─────────────────────────────────────────
//  COMPARE BOTH
// ─────────────────────────────────────────
void compare(const std::string& name, int capacity, int operations, int key_range) {
    std::cout << "--- " << name << " ---" << std::endl;
    benchmark(name, capacity, operations, key_range);
    benchmark_naive(name, capacity, operations, key_range);
}

// This is test for disk manager
void test_disk_write_read() {
    std::remove("test.db");  // ensure clean start — no leftover file
    {
        DiskManager dm("test.db");
        int id = dm.allocatePage();
        std::cout << "allocated page id: " << id << std::endl;

        char buf[PAGE_SIZE];
        memset(buf, 0, PAGE_SIZE);
        snprintf(buf, PAGE_SIZE, "%s", TEST_STRING);
        dm.writePage(id, buf);
        std::cout << "wrote to page: " << id << std::endl;
        std::cout << "first 20 chars written: " << std::string(buf, 20) << std::endl;
    }
    {
        DiskManager dm("test.db");
        char buf[PAGE_SIZE];
        memset(buf, 0, PAGE_SIZE);
        dm.readPage(0, buf);
        std::cout << "first 20 chars read: " << std::string(buf, 20) << std::endl;
        std::cout << "match: " << (strcmp(buf, TEST_STRING) == 0) << std::endl;
        assert(strcmp(buf, TEST_STRING) == 0);
    }
    std::remove("test.db");
    pass("Disk Write Read");
}

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
    std::cout << "         STRESS TESTS          " << std::endl;
    std::cout << "===============================" << std::endl;
    test_stress_evictions();
    test_stress_repeated_updates();
    test_stress_mixed();
    test_stress_large_capacity();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   O(1) vs NAIVE BENCHMARKS   " << std::endl;
    std::cout << "===============================" << std::endl;
    compare("Small cache, high contention",  10,      5000000, 20);
    compare("Medium cache, normal use",      1000,    5000000, 2000);
    compare("Large cache, high eviction",    100,     5000000, 100000);

    // Naive skipped for large capacity — too slow!
    std::cout << "--- Large cache, low eviction (O(1) only — Naive too slow!) ---" << std::endl;
    benchmark("Large cache, low eviction",   100000,  5000000, 100000);
    benchmark("Massive cache",               1000000, 5000000, 1000000);

    test_disk_write_read();

    std::cout << "===============================" << std::endl;
    std::cout << "     All tests passed! ✅      " << std::endl;
    std::cout << "===============================" << std::endl;

    return 0;
}
