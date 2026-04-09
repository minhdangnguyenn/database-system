#include <cstring>
#include <iostream>
#include <fstream>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include "../include/buffer-pool.h"
#include "../include/disk-manager.h"
#include "../include/lru-cache-naive.h"
#include "../include/lru.h"
#include "../include/test-data.h"

// ─────────────────────────────────────────
//  TIMING MACRO
// ─────────────────────────────────────────
#define TIMER_START \
    auto _t0 = std::chrono::high_resolution_clock::now();

#define TIMER_END(ops) \
    auto _t1 = std::chrono::high_resolution_clock::now(); \
    long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_t1 - _t0).count(); \
    long long elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(_t1 - _t0).count(); \
    long long ns_per_op  = (ops) > 0 ? elapsed_ns / (ops) : 0;

// ─────────────────────────────────────────
//  CSV EXPORT
// ─────────────────────────────────────────
void init_csv(std::ofstream& csv, const std::string& filename) {
    csv.open(filename);
    csv << "name,type,capacity,operations,key_range,time_ms,ns_per_op\n";
}

void write_csv_row(
    std::ofstream& csv,
    const std::string& name,
    const std::string& type,
    int capacity,
    int operations,
    int key_range,
    long long ms,
    long long ns_per_op
) {
    csv << name       << ","
        << type       << ","
        << capacity   << ","
        << operations << ","
        << key_range  << ","
        << ms         << ","
        << ns_per_op  << "\n";
}

// ─────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────
void pass(const std::string& name) {
    std::cout << "[PASS] " << name << std::endl;
}

void fail(const std::string& name) {
    std::cout << "[FAIL] " << name << std::endl;
}

// ═════════════════════════════════════════
//  LRU REPLACER — UNIT TESTS
//  Same operation counts as the Buffer Pool
//  tests so results are directly comparable.
// ═════════════════════════════════════════

// ─────────────────────────────────────────
//  BASIC  (mirrors BP basic section)
// ─────────────────────────────────────────

// Mirrors: test_basic
// Empty LRU → evict must return false.
// Pinned pages never appear as candidates.
void lru_test_evict_empty(std::ofstream& csv) {
    const int CAP = 2;          // same capacity as test_basic
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    assert(!lru.evict(frame_id)); ops++;     // truly empty
    lru.pin(1); ops++;
    lru.pin(2); ops++;
    assert(!lru.evict(frame_id)); ops++;     // all pinned — still nothing evictable
    assert(lru.Size() == 0);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Evict Empty", "LRU-TEST", CAP, ops, CAP, elapsed_ms, ns_per_op);
    pass("LRU Evict Empty");
}

// Mirrors: test_update_existing
// Unpin then evict — must return the correct frame_id.
void lru_test_unpin_then_evict(std::ofstream& csv) {
    const int CAP = 2;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    lru.pin(1); lru.pin(2); ops += 2;

    // unpin 1, evict → frame 1 comes out
    lru.unpin(1); ops++;
    assert(lru.Size() == 1);
    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 1);
    assert(lru.Size() == 0);

    // re-pin 1, unpin 2, then unpin 1
    // eviction order must be: 2 first (unpinned first), then 1
    lru.pin(1); ops++;
    lru.unpin(2); ops++;
    lru.unpin(1); ops++;
    assert(lru.Size() == 2);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 2);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 1);
    assert(lru.Size() == 0);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Unpin Then Evict", "LRU-TEST", CAP, ops, CAP, elapsed_ms, ns_per_op);
    pass("LRU Unpin Then Evict");
}

// Mirrors: test_capacity_one
// Capacity = 1. Pin → no eviction. Unpin → eviction succeeds.
void lru_test_capacity_one(std::ofstream& csv) {
    const int CAP = 1;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    lru.pin(1); ops++;
    assert(!lru.evict(frame_id)); ops++;   // pinned — not evictable

    lru.unpin(1); ops++;
    assert(lru.Size() == 1);
    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 1);
    assert(lru.Size() == 0);

    // pin a second frame, unpin, evict
    lru.pin(2); ops++;
    lru.unpin(2); ops++;
    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 2);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Capacity One", "LRU-TEST", CAP, ops, 2, elapsed_ms, ns_per_op);
    pass("LRU Capacity One");
}

// Mirrors: test_get_updates_recency
// Re-pinning a frame resets its recency — it should be evicted last.
void lru_test_pin_updates_recency(std::ofstream& csv) {
    const int CAP = 2;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    lru.pin(1); lru.pin(2); ops += 2;

    // unpin both
    lru.unpin(1); ops++;
    lru.unpin(2); ops++;
    assert(lru.Size() == 2);

    // re-pin frame 1 → pulls it out of evictable pool
    lru.pin(1); ops++;
    assert(lru.Size() == 1);

    // only frame 2 is evictable
    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 2);

    // now unpin frame 1 → evict it
    lru.unpin(1); ops++;
    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 1);
    assert(lru.Size() == 0);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Pin Updates Recency", "LRU-TEST", CAP, ops, 2, elapsed_ms, ns_per_op);
    pass("LRU Pin Updates Recency");
}

// Mirrors: test_miss
// Evicting from an LRU that has only pinned frames returns false.
void lru_test_evict_miss(std::ofstream& csv) {
    const int CAP = 3;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    assert(!lru.evict(frame_id)); ops++;    // empty
    lru.pin(1); ops++;
    assert(!lru.evict(frame_id)); ops++;    // 1 pinned — still miss
    lru.pin(2); lru.pin(3); ops += 2;
    assert(!lru.evict(frame_id)); ops++;    // all pinned — still miss

    TIMER_END(ops)

    write_csv_row(csv, "LRU Evict Miss", "LRU-TEST", CAP, ops, 3, elapsed_ms, ns_per_op);
    pass("LRU Evict Miss");
}

// ─────────────────────────────────────────
//  COMPLEX  (mirrors BP complex section)
// ─────────────────────────────────────────

// Mirrors: test_many_evictions
// Pin 100 frames, unpin last 3, verify only those 3 are evictable.
void lru_test_many_evictions(std::ofstream& csv) {
    const int CAP = 3;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    for (int i = 0; i < 100; i++) { lru.pin(i); ops++; }
    assert(!lru.evict(frame_id)); ops++;    // all pinned

    // unpin last 3 in order 99, 98, 97
    lru.unpin(99); lru.unpin(98); lru.unpin(97); ops += 3;
    assert(lru.Size() == 3);

    // eviction order must be 99 (LRU), then 98, then 97
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 99);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 98);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 97);
    assert(lru.Size() == 0);
    assert(!lru.evict(frame_id)); ops++;    // only pinned remain

    TIMER_END(ops)

    write_csv_row(csv, "LRU Many Evictions", "LRU-TEST", CAP, ops, 100, elapsed_ms, ns_per_op);
    pass("LRU Many Evictions");
}

// Mirrors: test_get_prevents_eviction
// Re-pinning prevents eviction — same logic as get() in BufferPool.
void lru_test_pin_prevents_eviction(std::ofstream& csv) {
    const int CAP = 3;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    lru.pin(1); lru.pin(2); lru.pin(3); ops += 3;

    // unpin all three
    lru.unpin(1); lru.unpin(2); lru.unpin(3); ops += 3;
    assert(lru.Size() == 3);

    // re-pin 1 and 3 → only frame 2 remains evictable
    lru.pin(1); lru.pin(3); ops += 2;
    assert(lru.Size() == 1);

    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 2);
    assert(lru.Size() == 0);

    // unpin 1 and 3
    lru.unpin(1); lru.unpin(3); ops += 2;

    // pin a 4th frame, unpin → LRU order is 1, 3, 4
    lru.pin(4); ops++;
    lru.unpin(4); ops++;
    assert(lru.Size() == 3);

    assert(lru.evict(frame_id)); ops++; assert(frame_id == 1);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 3);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 4);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Pin Prevents Eviction", "LRU-TEST", CAP, ops, 4, elapsed_ms, ns_per_op);
    pass("LRU Pin Prevents Eviction");
}

// Mirrors: test_alternating_operations
// Interleave pin / unpin / evict and verify exact outcomes.
void lru_test_alternating_operations(std::ofstream& csv) {
    const int CAP = 3;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    lru.pin(1); lru.pin(2); ops += 2;
    lru.unpin(1); ops++;                   // 1 evictable
    lru.pin(3); ops++;
    lru.unpin(3); ops++;                   // 3 evictable
    lru.pin(4); ops++;
    lru.unpin(2); ops++;                   // 2 evictable

    // eviction order: 1 (LRU), 3, 2
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 1);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 3);
    assert(lru.evict(frame_id)); ops++; assert(frame_id == 2);

    // frame 4 still pinned — not evictable
    assert(!lru.evict(frame_id)); ops++;

    TIMER_END(ops)

    write_csv_row(csv, "LRU Alternating Operations", "LRU-TEST", CAP, ops, 4, elapsed_ms, ns_per_op);
    pass("LRU Alternating Operations");
}

// Mirrors: test_repeated_updates
// Duplicate unpin 1000 times — size must always stay 1.
void lru_test_duplicate_unpin(std::ofstream& csv) {
    const int CAP = 2;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    lru.pin(1); ops++;
    for (int i = 0; i < 1000; i++) { lru.unpin(1); ops++; }
    assert(lru.Size() == 1);              // must not grow beyond 1

    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == 1);
    assert(lru.Size() == 0);
    assert(!lru.evict(frame_id)); ops++;

    TIMER_END(ops)

    write_csv_row(csv, "LRU Duplicate Unpin", "LRU-TEST", CAP, ops, 1, elapsed_ms, ns_per_op);
    pass("LRU Duplicate Unpin");
}

// Mirrors: test_no_eviction_needed
// All frames fit — pin all, unpin all, evict all cleanly.
void lru_test_no_eviction_needed(std::ofstream& csv) {
    const int CAP = 5;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    for (int i = 1; i <= 5; i++) { lru.pin(i); ops++; }
    assert(lru.Size() == 0);

    for (int i = 1; i <= 5; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == 5);

    for (int i = 1; i <= 5; i++) {
        assert(lru.evict(frame_id)); ops++;
        assert(frame_id == i);            // eviction follows pin/unpin order
    }
    assert(lru.Size() == 0);

    TIMER_END(ops)

    write_csv_row(csv, "LRU No Eviction Needed", "LRU-TEST", CAP, ops, 5, elapsed_ms, ns_per_op);
    pass("LRU No Eviction Needed");
}

// Mirrors: test_large_capacity
// 1000 frames pinned and unpinned — verify full eviction sequence.
void lru_test_large_capacity(std::ofstream& csv) {
    const int CAP = 1000;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    for (int i = 0; i < CAP; i++) { lru.pin(i);   ops++; }
    for (int i = 0; i < CAP; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == CAP);

    for (int i = 0; i < CAP; i++) {
        assert(lru.evict(frame_id)); ops++;
        assert(frame_id == i);
    }
    assert(lru.Size() == 0);

    // one extra pin beyond capacity range — must still work
    lru.pin(CAP); ops++;
    lru.unpin(CAP); ops++;
    assert(lru.evict(frame_id)); ops++;
    assert(frame_id == CAP);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Large Capacity", "LRU-TEST", CAP, ops, CAP, elapsed_ms, ns_per_op);
    pass("LRU Large Capacity");
}

// ─────────────────────────────────────────
//  STRESS  (mirrors BP stress section)
// ─────────────────────────────────────────

// Mirrors: test_stress_evictions
// Pin 10,000 frames, unpin last 100, verify exact eviction order.
void lru_test_stress_evictions(std::ofstream& csv) {
    const int CAP       = 100;
    const int KEY_RANGE = 10000;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    for (int i = 0; i < KEY_RANGE; i++) { lru.pin(i); ops++; }
    assert(lru.Size() == 0);

    // unpin last 100 in order 9900..9999
    for (int i = 9900; i < KEY_RANGE; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == CAP);

    // eviction order must be 9900, 9901, ..., 9999
    for (int i = 9900; i < KEY_RANGE; i++) {
        assert(lru.evict(frame_id)); ops++;
        assert(frame_id == i);
    }
    assert(lru.Size() == 0);
    assert(!lru.evict(frame_id)); ops++;    // rest are still pinned

    TIMER_END(ops)

    write_csv_row(csv, "LRU Stress Evictions", "LRU-TEST", CAP, ops, KEY_RANGE, elapsed_ms, ns_per_op);
    pass("LRU Stress Evictions");
}

// Mirrors: test_stress_repeated_updates
// 100,000 cycles of pin(i%10) + unpin(i%10). Size must always be <= 10.
void lru_test_stress_repeated_updates(std::ofstream& csv) {
    const int CAP       = 10;
    const int ROUNDS    = 100000;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    for (int i = 0; i < ROUNDS; i++) {
        int key = i % CAP;
        lru.pin(key);   ops++;
        lru.unpin(key); ops++;
        assert(lru.Size() <= CAP);
    }

    // drain all evictable frames
    int evicted = 0;
    while (lru.evict(frame_id)) { ops++; evicted++; }
    assert(evicted <= CAP);
    assert(lru.Size() == 0);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Stress Repeated Updates", "LRU-TEST", CAP, ops, CAP, elapsed_ms, ns_per_op);
    pass("LRU Stress Repeated Updates");
}

// Mirrors: test_stress_mixed
// 500 frames, read half to update recency, insert 250 new ones.
// Verify eviction picks the correct victims.
void lru_test_stress_mixed(std::ofstream& csv) {
    const int CAP = 500;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    // pin all 500
    for (int i = 0; i < CAP; i++) { lru.pin(i); ops++; }

    // unpin 0..499 in order
    for (int i = 0; i < CAP; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == CAP);

    // re-pin 0..249 → they leave the evictable pool
    for (int i = 0; i < 250; i++) { lru.pin(i); ops++; }
    assert(lru.Size() == 250);          // only 250..499 remain evictable

    // evict the 250 LRU frames (250..499)
    for (int i = 250; i < CAP; i++) {
        assert(lru.evict(frame_id)); ops++;
        assert(frame_id == i);
    }
    assert(lru.Size() == 0);

    // pin 250 new frames (500..749), unpin all
    for (int i = CAP; i < 750; i++) { lru.pin(i);   ops++; }
    for (int i = CAP; i < 750; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == 250);

    // unpin frames 0..249 too
    for (int i = 0; i < 250; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == 500);

    // drain all
    int evicted = 0;
    while (lru.evict(frame_id)) { ops++; evicted++; }
    assert(evicted == 500);
    assert(lru.Size() == 0);

    TIMER_END(ops)

    write_csv_row(csv, "LRU Stress Mixed", "LRU-TEST", CAP, ops, 750, elapsed_ms, ns_per_op);
    pass("LRU Stress Mixed");
}

// Mirrors: test_stress_large_capacity
// 100,000 frames, unpin last 1,000, verify they are evicted first.
void lru_test_stress_large_capacity(std::ofstream& csv) {
    const int CAP       = 100000;
    const int KEY_RANGE = CAP;
    LRU lru(CAP);
    int frame_id;
    int ops = 0;

    TIMER_START

    for (int i = 0; i < KEY_RANGE; i++) { lru.pin(i); ops++; }
    assert(lru.Size() == 0);

    // unpin last 1,000 frames
    for (int i = KEY_RANGE - 1000; i < KEY_RANGE; i++) { lru.unpin(i); ops++; }
    assert(lru.Size() == 1000);

    // evict all 1,000 in LRU order
    for (int i = KEY_RANGE - 1000; i < KEY_RANGE; i++) {
        assert(lru.evict(frame_id)); ops++;
        assert(frame_id == i);
    }
    assert(lru.Size() == 0);
    assert(!lru.evict(frame_id)); ops++;   // rest are still pinned

    TIMER_END(ops)

    write_csv_row(csv, "LRU Stress Large Capacity", "LRU-TEST", CAP, ops, KEY_RANGE, elapsed_ms, ns_per_op);
    pass("LRU Stress Large Capacity");
}

// ═════════════════════════════════════════
//  BUFFER POOL — UNIT TESTS  (unchanged)
// ═════════════════════════════════════════

// ─────────────────────────────────────────
//  BASIC TESTS
// ─────────────────────────────────────────
void test_basic() {
    BufferPool cache(2);
    cache.pin(1, 1); cache.pin(2, 2);
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
    cache.pin(1, 10); cache.pin(2, 20); cache.pin(1, 99);
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
    cache.pin(1, 1); cache.pin(2, 2);
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
    for (int i = 0; i < 100; i++) cache.pin(i, i * 10);
    assert(cache.get(99) == 990);
    assert(cache.get(98) == 980);
    assert(cache.get(97) == 970);
    assert(cache.get(96) == -1);
    assert(cache.get(0)  == -1);
    pass("Many Evictions");
}

void test_get_prevents_eviction() {
    BufferPool cache(3);
    cache.pin(1,1); cache.pin(2,2); cache.pin(3,3);
    cache.get(1); cache.get(3);
    cache.pin(4, 4);
    assert(cache.get(2) == -1);
    assert(cache.get(1) == 1);
    assert(cache.get(3) == 3);
    assert(cache.get(4) == 4);
    pass("Get Prevents Eviction");
}

void test_alternating_operations() {
    BufferPool cache(3);
    cache.pin(1,10); cache.pin(2,20);
    assert(cache.get(1) == 10);
    cache.pin(3,30); cache.pin(4,40);
    assert(cache.get(2) == -1);
    assert(cache.get(1) == 10);
    assert(cache.get(3) == 30);
    assert(cache.get(4) == 40);
    pass("Alternating Operations");
}

void test_repeated_updates() {
    BufferPool cache(2);
    for (int i = 0; i < 1000; i++) cache.pin(1, i);
    assert(cache.get(1) == 999);
    pass("Repeated Updates");
}

void test_no_eviction_needed() {
    BufferPool cache(5);
    cache.pin(1,10); cache.pin(2,20); cache.pin(3,30);
    cache.pin(4,40); cache.pin(5,50);
    assert(cache.get(1)==10); assert(cache.get(2)==20);
    assert(cache.get(3)==30); assert(cache.get(4)==40);
    assert(cache.get(5)==50);
    pass("No Eviction Needed");
}

void test_large_capacity() {
    int cap = 1000;
    BufferPool cache(cap);
    for (int i = 0; i < cap; i++) cache.pin(i, i * 2);
    for (int i = 0; i < cap; i++) assert(cache.get(i) == i * 2);
    cache.pin(cap, cap * 2);
    pass("Large Capacity");
}

// ─────────────────────────────────────────
//  STRESS TESTS
// ─────────────────────────────────────────
void test_stress_evictions() {
    BufferPool cache(100);
    for (int i = 0; i < 10000; i++) cache.pin(i, i * 3);
    for (int i = 9900; i < 10000; i++) assert(cache.get(i) == i * 3);
    assert(cache.get(9899) == -1);
    assert(cache.get(0)    == -1);
    pass("Stress Evictions");
}

void test_stress_repeated_updates() {
    BufferPool cache(10);
    for (int i = 0; i < 100000; i++) cache.pin(i % 10, i);
    for (int k = 0; k < 10; k++) assert(cache.get(k) == 99990 + k);
    pass("Stress Repeated Updates");
}

void test_stress_mixed() {
    BufferPool cache(500);
    for (int i = 0; i < 500; i++) cache.pin(i, i * 7);
    for (int i = 0; i < 250; i++) assert(cache.get(i) == i * 7);
    for (int i = 500; i < 750; i++) cache.pin(i, i * 7);
    for (int i = 0; i < 250; i++) assert(cache.get(i) == i * 7);
    for (int i = 250; i < 500; i++) assert(cache.get(i) == -1);
    for (int i = 500; i < 750; i++) assert(cache.get(i) == i * 7);
    pass("Stress Mixed");
}

void test_stress_large_capacity() {
    int cap = 100000;
    BufferPool cache(cap);
    for (int i = 0; i < cap; i++) cache.pin(i, i * 5);
    for (int i = 0; i < cap; i++) assert(cache.get(i) == i * 5);
    for (int i = cap; i < cap + 1000; i++) cache.pin(i, i * 5);
    for (int i = 0; i < 1000; i++) assert(cache.get(i) == -1);
    pass("Stress Large Capacity");
}

// ─────────────────────────────────────────
//  BENCHMARKS — O(1) vs NAIVE
// ─────────────────────────────────────────
void benchmark(
    const std::string& name,
    int capacity,
    int operations,
    int key_range,
    std::ofstream& csv
) {
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
    auto npo = ns / operations;

    std::cout << "[O(1)]  " << name << std::endl;
    std::cout << "  Operations : " << operations << std::endl;
    std::cout << "  Capacity   : " << capacity   << std::endl;
    std::cout << "  Key range  : " << key_range  << std::endl;
    std::cout << "  Time       : " << ms         << " ms" << std::endl;
    std::cout << "  Per op     : " << npo        << " ns" << std::endl;
    std::cout << std::endl;

    write_csv_row(csv, name, "O1", capacity, operations, key_range, ms, npo);
}

void benchmark_naive(
    const std::string& name,
    int capacity,
    int operations,
    int key_range,
    std::ofstream& csv
) {
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
    auto npo = ns / operations;

    std::cout << "[NAIVE] " << name << std::endl;
    std::cout << "  Operations : " << operations << std::endl;
    std::cout << "  Capacity   : " << capacity   << std::endl;
    std::cout << "  Key range  : " << key_range  << std::endl;
    std::cout << "  Time       : " << ms         << " ms" << std::endl;
    std::cout << "  Per op     : " << npo        << " ns" << std::endl;
    std::cout << std::endl;

    write_csv_row(csv, name, "NAIVE", capacity, operations, key_range, ms, npo);
}

void compare(
    const std::string& name,
    int capacity,
    int operations,
    int key_range,
    std::ofstream& csv
) {
    std::cout << "--- " << name << " ---" << std::endl;
    benchmark(name, capacity, operations, key_range, csv);
    benchmark_naive(name, capacity, operations, key_range, csv);
}

// ─────────────────────────────────────────
//  DISK MANAGER TEST
// ─────────────────────────────────────────
void test_disk_write_read() {
    std::remove("test.db");
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

// ═════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════
int main() {
    std::ofstream csv;
    init_csv(csv, "benchmark_results.csv");

    auto run_start = std::chrono::high_resolution_clock::now();

    // ─────────────────────────────────────
    std::cout << "===============================" << std::endl;
    std::cout << "   LRU REPLACER — BASIC TESTS  " << std::endl;
    std::cout << "===============================" << std::endl;
    lru_test_evict_empty(csv);
    lru_test_unpin_then_evict(csv);
    lru_test_capacity_one(csv);
    lru_test_pin_updates_recency(csv);
    lru_test_evict_miss(csv);

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "  LRU REPLACER — COMPLEX TESTS " << std::endl;
    std::cout << "===============================" << std::endl;
    lru_test_many_evictions(csv);
    lru_test_pin_prevents_eviction(csv);
    lru_test_alternating_operations(csv);
    lru_test_duplicate_unpin(csv);
    lru_test_no_eviction_needed(csv);
    lru_test_large_capacity(csv);

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "  LRU REPLACER — STRESS TESTS  " << std::endl;
    std::cout << "===============================" << std::endl;
    lru_test_stress_evictions(csv);
    lru_test_stress_repeated_updates(csv);
    lru_test_stress_mixed(csv);
    lru_test_stress_large_capacity(csv);

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "    BUFFER POOL — BASIC TESTS  " << std::endl;
    std::cout << "===============================" << std::endl;
    test_basic();
    test_update_existing();
    test_capacity_one();
    test_get_updates_recency();
    test_miss();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   BUFFER POOL — COMPLEX TESTS " << std::endl;
    std::cout << "===============================" << std::endl;
    test_many_evictions();
    test_get_prevents_eviction();
    test_alternating_operations();
    test_repeated_updates();
    test_no_eviction_needed();
    test_large_capacity();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   BUFFER POOL — STRESS TESTS  " << std::endl;
    std::cout << "===============================" << std::endl;
    test_stress_evictions();
    test_stress_repeated_updates();
    test_stress_mixed();
    test_stress_large_capacity();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   O(1) vs NAIVE BENCHMARKS    " << std::endl;
    std::cout << "===============================" << std::endl;
    compare("Small cache - high contention", 10,      5000000, 20,      csv);
    compare("Medium cache - normal use",     1000,    5000000, 2000,    csv);
    compare("Large cache - high eviction",   100,     5000000, 100000,  csv);

    std::cout << "--- Large cache, low eviction (O(1) only) ---" << std::endl;
    benchmark("Large cache - low eviction",  100000,  5000000, 100000,  csv);
    benchmark("Massive cache",              1000000, 5000000, 1000000, csv);

    csv.close();
    std::cout << std::endl;
    std::cout << "Results exported → benchmark_results.csv" << std::endl;

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "       DISK MANAGER TEST       " << std::endl;
    std::cout << "===============================" << std::endl;
    test_disk_write_read();

    auto run_end = std::chrono::high_resolution_clock::now();
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        run_end - run_start).count();

    std::cout << "===============================" << std::endl;
    std::cout << "     All tests passed! ✅      " << std::endl;
    std::cout << "  Total time : " << total_ms << " ms" << std::endl;
    std::cout << "===============================" << std::endl;

    return 0;
}
