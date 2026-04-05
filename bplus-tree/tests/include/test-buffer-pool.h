#ifndef TEST_BUFFER_POOL_H
#define TEST_BUFFER_POOL_H

#include <cassert>
#include <string>

class TestBufferPool {
public:
    void pass(const std::string& name);

    // DiskManager tests
    void test_allocate_page_increments();
    void test_write_read_roundtrip();
    void test_multiple_pages_independent();
    void test_overwrite_page();
    void test_many_allocations_sequential();
    void test_fresh_page_is_zeroed();

    // BufferPool tests
    void test_create_new_page_returns_valid_id();
    void test_create_multiple_pages_unique_ids();
    void test_fetch_nonexistent_page_returns_null();
    void test_fetch_negative_page_id_returns_null();
    void test_fetch_existing_page_returns_data();
    void test_fetch_same_page_twice_cache_hit();
    void test_fill_to_capacity();
    void test_eviction_triggered_when_full();
    void test_correct_page_evicted();
    void test_all_pinned_returns_error();
    void test_written_data_persists_after_eviction();
    void test_clean_page_evicted_no_corruption();
    void test_dirty_flag_reset_after_reload();

    // LRU integration tests
    void test_lru_eviction_order();
    void test_fetch_updates_recency();
    void test_repin_prevents_early_eviction();
    void test_multiple_sequential_evictions();

    void run_all();
};

#endif // TEST_BUFFER_POOL_H
