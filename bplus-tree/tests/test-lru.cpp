#include "include/test-lru.h"
#include <cassert>
#include <iostream>

// ─────────────────────────────────────────
//  test_evict_empty
//  Evicting from an LRU with no unpinned
//  frames must always return false —
//  regardless of how many pinned frames exist.
// ─────────────────────────────────────────
void TEST_LRU::test_evict_empty()
{
    int frame_id;

    // ── Case 1: truly empty LRU ──
    lru = new LRU(10);
    assert(!lru->evict(frame_id));
    delete lru;

    // ── Case 2: one pinned frame ──
    lru = new LRU(10);
    lru->pin(1);
    assert(!lru->evict(frame_id));
    delete lru;

    // ── Case 3: many pinned frames ──
    lru = new LRU(100);
    for (int i = 0; i < 100; i++) lru->pin(i);
    assert(!lru->evict(frame_id));
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 4: unpin then re-pin → back to empty ──
    lru = new LRU(10);
    lru->pin(1);
    lru->unpin(1);
    assert(lru->Size() == 1);
    lru->pin(1);              // re-pin pulls it back out
    assert(lru->Size() == 0);
    assert(!lru->evict(frame_id));
    delete lru;

    // ── Case 5: evict all then evict again ──
    lru = new LRU(10);
    lru->pin(1);
    lru->unpin(1);
    assert(lru->evict(frame_id));
    assert(!lru->evict(frame_id)); // already empty after one eviction
    delete lru;

    std::cout << "[UNIT TEST PASS] test_evict_empty" << std::endl;
}

// ─────────────────────────────────────────
//  test_unpin_then_evict
//  After unpinning, a frame must become
//  evictable. Tests multiple frames and
//  verifies the returned frame_id is exact.
// ─────────────────────────────────────────
void TEST_LRU::test_unpin_then_evict()
{
    int frame_id;

    // ── Case 1: single pin → unpin → evict ──
    lru = new LRU(10);
    lru->pin(1);
    lru->unpin(1);
    assert(lru->evict(frame_id));
    assert(frame_id == 1);
    delete lru;

    // ── Case 2: only the unpinned frame is evicted ──
    lru = new LRU(10);
    for (int i = 1; i <= 5; i++) lru->pin(i);
    lru->unpin(3);
    assert(lru->evict(frame_id));
    assert(frame_id == 3);
    assert(!lru->evict(frame_id)); // rest are still pinned
    delete lru;

    // ── Case 3: unpin in order → evict in same order ──
    lru = new LRU(10);
    for (int i = 1; i <= 5; i++) lru->pin(i);
    lru->unpin(2);
    lru->unpin(4);
    lru->unpin(1);
    assert(lru->evict(frame_id)); assert(frame_id == 2);
    assert(lru->evict(frame_id)); assert(frame_id == 4);
    assert(lru->evict(frame_id)); assert(frame_id == 1);
    assert(!lru->evict(frame_id));
    delete lru;

    // ── Case 4: unpin, evict, re-pin, unpin, evict again ──
    lru = new LRU(10);
    lru->pin(7);
    lru->unpin(7);
    assert(lru->evict(frame_id)); assert(frame_id == 7);
    lru->pin(7);
    lru->unpin(7);
    assert(lru->evict(frame_id)); assert(frame_id == 7);
    assert(lru->Size() == 0);
    delete lru;

    std::cout << "[UNIT TEST PASS] test_unpin_then_evict" << std::endl;
}

// ─────────────────────────────────────────
//  test_pin_prevents_eviction
//  Pinned frames must never be evicted.
//  Tests re-pinning an unpinned frame and
//  mixed sequences of pin/unpin.
// ─────────────────────────────────────────
void TEST_LRU::test_pin_prevents_eviction()
{
    int frame_id;

    // ── Case 1: single pinned frame ──
    lru = new LRU(10);
    lru->pin(1);
    assert(!lru->evict(frame_id));
    delete lru;

    // ── Case 2: unpin then re-pin → no longer evictable ──
    lru = new LRU(10);
    lru->pin(1);
    lru->unpin(1);
    assert(lru->Size() == 1);
    lru->pin(1);              // re-pin → removed from evictable pool
    assert(lru->Size() == 0);
    assert(!lru->evict(frame_id));
    delete lru;

    // ── Case 3: only unpinned frames are evicted ──
    lru = new LRU(10);
    for (int i = 1; i <= 5; i++) lru->pin(i);
    lru->unpin(2);
    lru->unpin(4);
    // re-pin 2 — should no longer be evictable
    lru->pin(2);
    assert(lru->Size() == 1); // only 4 remains
    assert(lru->evict(frame_id)); assert(frame_id == 4);
    assert(!lru->evict(frame_id)); // 1, 2, 3, 5 still pinned
    delete lru;

    // ── Case 4: rapid unpin/re-pin across many frames ──
    lru = new LRU(50);
    for (int i = 0; i < 50; i++) lru->pin(i);
    for (int i = 0; i < 50; i++) lru->unpin(i);
    assert(lru->Size() == 50);
    // re-pin even frames → only odd frames remain evictable
    for (int i = 0; i < 50; i += 2) lru->pin(i);
    assert(lru->Size() == 25);
    int evicted = 0;
    while (lru->evict(frame_id)) {
        assert(frame_id % 2 == 1); // must always be odd
        evicted++;
    }
    assert(evicted == 25);
    assert(lru->Size() == 0);
    delete lru;

    std::cout << "[UNIT TEST PASS] test_pin_prevents_eviction" << std::endl;
}

// ─────────────────────────────────────────
//  test_duplicate_unpin
//  Calling unpin multiple times on the same
//  frame must not corrupt size or order.
// ─────────────────────────────────────────
void TEST_LRU::test_duplicate_unpin()
{
    int frame_id;

    // ── Case 1: double unpin — size must stay 1 ──
    lru = new LRU(10);
    lru->pin(1);
    lru->unpin(1);
    lru->unpin(1); // duplicate
    assert(lru->Size() == 1);
    assert(lru->evict(frame_id)); assert(frame_id == 1);
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 2: 1000 duplicate unpins — size must still be 1 ──
    lru = new LRU(10);
    lru->pin(1);
    for (int i = 0; i < 1000; i++) lru->unpin(1);
    assert(lru->Size() == 1);
    assert(lru->evict(frame_id)); assert(frame_id == 1);
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 3: duplicate unpin does not change eviction order ──
    lru = new LRU(10);
    lru->pin(1); lru->pin(2); lru->pin(3);
    lru->unpin(1);
    lru->unpin(2); lru->unpin(2); // duplicate
    lru->unpin(3);
    assert(lru->Size() == 3);
    // order must still be 1, 2, 3
    assert(lru->evict(frame_id)); assert(frame_id == 1);
    assert(lru->evict(frame_id)); assert(frame_id == 2);
    assert(lru->evict(frame_id)); assert(frame_id == 3);
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 4: duplicate unpin then re-pin ──
    lru = new LRU(10);
    lru->pin(5);
    lru->unpin(5); lru->unpin(5);
    assert(lru->Size() == 1);
    lru->pin(5);   // re-pin → back to 0
    assert(lru->Size() == 0);
    assert(!lru->evict(frame_id));
    delete lru;

    std::cout << "[UNIT TEST PASS] test_duplicate_unpin" << std::endl;
}

// ─────────────────────────────────────────
//  test_lru_order
//  Eviction must always pick the least
//  recently unpinned frame. Tests forward,
//  reverse, and interleaved unpin orders.
// ─────────────────────────────────────────
void TEST_LRU::test_lru_order()
{
    int frame_id;

    // ── Case 1: forward unpin order ──
    lru = new LRU(100);
    for (int i = 0; i < 100; i++) lru->pin(i);
    for (int i = 0; i < 100; i++) lru->unpin(i);
    for (int i = 0; i < 100; i++) {
        assert(lru->evict(frame_id));
        assert(frame_id == i);
    }
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 2: reverse unpin order ──
    lru = new LRU(100);
    for (int i = 0; i < 100; i++) lru->pin(i);
    for (int i = 99; i >= 0; i--) lru->unpin(i);
    for (int i = 99; i >= 0; i--) {
        assert(lru->evict(frame_id));
        assert(frame_id == i);
    }
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 3: interleaved order ──
    // unpin order: 4, 1, 3, 0, 2
    // eviction must follow that exact order
    lru = new LRU(10);
    for (int i = 0; i < 5; i++) lru->pin(i);
    int order[] = {4, 1, 3, 0, 2};
    for (int x : order) lru->unpin(x);
    for (int x : order) {
        assert(lru->evict(frame_id));
        assert(frame_id == x);
    }
    assert(lru->Size() == 0);
    delete lru;

    // ── Case 4: re-pin resets recency ──
    // pin 0,1,2 → unpin all → re-pin 0 → unpin 0
    // eviction order must be: 1, 2, 0 (0 is now MRU)
    lru = new LRU(10);
    lru->pin(0); lru->pin(1); lru->pin(2);
    lru->unpin(0); lru->unpin(1); lru->unpin(2);
    lru->pin(0);   // re-pin 0 → removed from evictable pool
    lru->unpin(0); // unpin again → now MRU
    assert(lru->evict(frame_id)); assert(frame_id == 1);
    assert(lru->evict(frame_id)); assert(frame_id == 2);
    assert(lru->evict(frame_id)); assert(frame_id == 0);
    assert(lru->Size() == 0);
    delete lru;

    std::cout << "[UNIT TEST PASS] test_lru_order" << std::endl;
}

// ─────────────────────────────────────────
//  test_size
//  Size() must count only evictable
//  (unpinned) frames. Tests every
//  transition: pin, unpin, re-pin, evict.
// ─────────────────────────────────────────
void TEST_LRU::test_size()
{
    int frame_id;
    lru = new LRU(100);

    // ── empty ──
    assert(lru->Size() == 0);

    // ── pin 10 → size stays 0 ──
    for (int i = 0; i < 10; i++) lru->pin(i);
    assert(lru->Size() == 0);

    // ── unpin 1 → size becomes 1 ──
    lru->unpin(1);
    assert(lru->Size() == 1);

    // ── unpin 4 more → size becomes 5 ──
    for (int i = 2; i <= 5; i++) lru->unpin(i);
    assert(lru->Size() == 5);

    // ── re-pin frame 3 → removed from pool, size drops to 4 ──
    lru->pin(3);
    assert(lru->Size() == 4);

    // ── evict 1 → size drops to 3 ──
    assert(lru->evict(frame_id));
    assert(lru->Size() == 3);

    // ── evict remaining 3 → size reaches 0 ──
    assert(lru->evict(frame_id)); assert(lru->Size() == 2);
    assert(lru->evict(frame_id)); assert(lru->Size() == 1);
    assert(lru->evict(frame_id)); assert(lru->Size() == 0);

    // ── unpin frame 3 again (was re-pinned earlier) → size 1 ──
    lru->unpin(3);
    assert(lru->Size() == 1);
    assert(lru->evict(frame_id)); assert(frame_id == 3);
    assert(lru->Size() == 0);

    // ── evict on empty → false, size stays 0 ──
    assert(!lru->evict(frame_id));
    assert(lru->Size() == 0);

    // ── large batch: pin 50, unpin 25, check size each step ──
    for (int i = 0; i < 50; i++) lru->pin(i);
    assert(lru->Size() == 0);
    for (int i = 0; i < 25; i++) {
        lru->unpin(i);
        assert(lru->Size() == i + 1);
    }
    assert(lru->Size() == 25);

    // evict all 25 one by one
    for (int i = 24; i >= 0; i--) {
        assert(lru->evict(frame_id));
        assert(lru->Size() == i);
    }
    assert(lru->Size() == 0);

    std::cout << "[UNIT PASS] test_size" << std::endl;
    delete lru;
}

void TEST_LRU::run_all() {
    test_evict_empty();
    test_unpin_then_evict();
    test_pin_prevents_eviction();
    test_duplicate_unpin();
    test_lru_order();
    test_size();
}
