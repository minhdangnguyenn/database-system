#ifndef TEST_LRU_H
#define TEST_LRU_H

#include "../../include/lru.h"

class TEST_LRU {
public:
    LRU* lru;
    void test_evict_empty();
    void test_unpin_then_evict();
    void test_pin_prevents_eviction();
    void test_duplicate_unpin();
    void test_lru_order();
    void test_size();
    void run_all();
};

#endif //TEST_LRU_H
