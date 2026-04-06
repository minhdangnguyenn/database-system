#include "include/test-buffer-pool.h"
#include "../include/disk-manager.h"
#include "../include/test-data.h"
#include "../include/buffer-pool.h"
#include <cstring>
#include <iostream>


void TestBufferPool::pass(const std::string& name) {
    std::cout << "[PASS] " << name << std::endl;
}

// ─────────────────────────────────────────
//  DiskManager tests
// ─────────────────────────────────────────

void TestBufferPool::test_allocate_page_increments() {
    std::remove("test_alloc.db");
    DiskManager dm("test_alloc.db");

    int id0 = dm.allocate_page();
    int id1 = dm.allocate_page();
    int id2 = dm.allocate_page();

    assert(id1 == id0 + 1);
    assert(id2 == id1 + 1);

    std::remove("test_alloc.db");
    pass("test_allocate_page_increments");
}

void TestBufferPool::test_write_read_roundtrip() {
    std::remove("test_rw.db");
    DiskManager dm("test_rw.db");

    int page_id = dm.allocate_page();

    char write_buf[PAGE_SIZE] = {};
    snprintf(write_buf, PAGE_SIZE, "hello buffer pool");
    dm.write_page(page_id, write_buf);

    char read_buf[PAGE_SIZE] = {};
    dm.read_page(page_id, read_buf);

    assert(memcmp(write_buf, read_buf, PAGE_SIZE) == 0);

    std::remove("test_rw.db");
    pass("test_write_read_roundtrip");
}

void TestBufferPool::test_multiple_pages_independent() {
    std::remove("test_multi.db");
    DiskManager dm("test_multi.db");

    int id_a = dm.allocate_page();
    int id_b = dm.allocate_page();

    char buf_a[PAGE_SIZE] = {};
    char buf_b[PAGE_SIZE] = {};
    snprintf(buf_a, PAGE_SIZE, "page A data");
    snprintf(buf_b, PAGE_SIZE, "page B data");

    dm.write_page(id_a, buf_a);
    dm.write_page(id_b, buf_b);

    char read_a[PAGE_SIZE] = {};
    char read_b[PAGE_SIZE] = {};
    dm.read_page(id_a, read_a);
    dm.read_page(id_b, read_b);

    assert(memcmp(read_a, buf_a, PAGE_SIZE) == 0);
    assert(memcmp(read_b, buf_b, PAGE_SIZE) == 0);
    assert(memcmp(read_a, read_b, PAGE_SIZE) != 0);

    std::remove("test_multi.db");
    pass("test_multiple_pages_independent");
}

void TestBufferPool::test_overwrite_page() {
    std::remove("test_overwrite.db");
    DiskManager dm("test_overwrite.db");

    int id = dm.allocate_page();

    char buf1[PAGE_SIZE] = {};
    char buf2[PAGE_SIZE] = {};
    snprintf(buf1, PAGE_SIZE, "first write");
    snprintf(buf2, PAGE_SIZE, "second write overwrite");

    dm.write_page(id, buf1);
    dm.write_page(id, buf2);

    char read_buf[PAGE_SIZE] = {};
    dm.read_page(id, read_buf);

    assert(memcmp(read_buf, buf2, PAGE_SIZE) == 0);
    assert(memcmp(read_buf, buf1, PAGE_SIZE) != 0);

    std::remove("test_overwrite.db");
    pass("test_overwrite_page");
}

void TestBufferPool::test_many_allocations_sequential() {
    std::remove("test_many.db");
    DiskManager dm("test_many.db");

    const int N = 20;
    int ids[N];
    for (int i = 0; i < N; i++) {
        ids[i] = dm.allocate_page();
    }
    for (int i = 1; i < N; i++) {
        assert(ids[i] == ids[i - 1] + 1);
    }

    std::remove("test_many.db");
    pass("test_many_allocations_sequential");
}

void TestBufferPool::test_fresh_page_is_zeroed() {
    std::remove("test_zero.db");
    DiskManager dm("test_zero.db");

    int id = dm.allocate_page();
    char buf[PAGE_SIZE];
    memset(buf, 0xFF, PAGE_SIZE);
    dm.read_page(id, buf);

    char zeros[PAGE_SIZE] = {};
    assert(memcmp(buf, zeros, PAGE_SIZE) == 0);

    std::remove("test_zero.db");
    pass("test_fresh_page_is_zeroed");
}

// ─────────────────────────────────────────
//  BufferPool tests
// ─────────────────────────────────────────

void TestBufferPool::test_create_new_page_returns_valid_id() {
    std::remove("mydb.db");
    BufferPool bp(4);

    int page_id = bp.create_new_page();
    assert(page_id >= 0);

    std::remove("mydb.db");
    pass("test_create_new_page_returns_valid_id");
}

void TestBufferPool::test_create_multiple_pages_unique_ids() {
    std::remove("mydb.db");
    BufferPool bp(4);

    int id0 = bp.create_new_page();
    int id1 = bp.create_new_page();
    int id2 = bp.create_new_page();

    assert(id0 != id1);
    assert(id1 != id2);
    assert(id0 != id2);

    std::remove("mydb.db");
    pass("test_create_multiple_pages_unique_ids");
}

void TestBufferPool::test_fetch_nonexistent_page_returns_null() {
    std::remove("mydb.db");
    BufferPool bp(4);

    char* data = bp.fetch_page(9999);
    assert(data == nullptr);

    std::remove("mydb.db");
    pass("test_fetch_nonexistent_page_returns_null");
}

void TestBufferPool::test_fetch_negative_page_id_returns_null() {
    std::remove("mydb.db");
    BufferPool bp(4);

    char* data = bp.fetch_page(-1);
    assert(data == nullptr);

    std::remove("mydb.db");
    pass("test_fetch_negative_page_id_returns_null");
}

void TestBufferPool::test_fetch_existing_page_returns_data() {
    std::remove("mydb.db");
    BufferPool bp(4);

    int page_id = bp.create_new_page();
    assert(page_id >= 0);

    char* data = bp.fetch_page(page_id);
    assert(data != nullptr);

    std::remove("mydb.db");
    pass("test_fetch_existing_page_returns_data");
}

void TestBufferPool::test_fetch_same_page_twice_cache_hit() {
    std::remove("mydb.db");
    BufferPool bp(4);

    int page_id = bp.create_new_page();
    char* first  = bp.fetch_page(page_id);
    char* second = bp.fetch_page(page_id);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(first == second);

    std::remove("mydb.db");
    pass("test_fetch_same_page_twice_cache_hit");
}

void TestBufferPool::test_fill_to_capacity() {
    std::remove("mydb.db");
    const int CAP = 4;
    BufferPool bp(CAP);

    for (int i = 0; i < CAP; i++) {
        int page_id = bp.create_new_page();
        assert(page_id >= 0);
    }

    std::remove("mydb.db");
    pass("test_fill_to_capacity");
}

void TestBufferPool::test_eviction_triggered_when_full() {
    std::remove("mydb.db");
    const int CAP = 4;
    BufferPool bp(CAP);

    int ids[CAP];
    for (int i = 0; i < CAP; i++) {
        ids[i] = bp.create_new_page();
        assert(ids[i] >= 0);
    }

    for (int i = 0; i < CAP; i++) {
        bp.unpin_page(ids[i], false);
    }

    int new_id = bp.create_new_page();
    assert(new_id >= 0);

    std::remove("mydb.db");
    pass("test_eviction_triggered_when_full");
}

void TestBufferPool::test_correct_page_evicted() {
    std::remove("mydb.db");
    const int CAP = 2;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    int id_b = bp.create_new_page();

    bp.unpin_page(id_a, false);
    bp.unpin_page(id_b, false);

    int id_c = bp.create_new_page();
    assert(id_c >= 0);

    char* data_b = bp.fetch_page(id_b);
    assert(data_b != nullptr);

    std::remove("mydb.db");
    pass("test_correct_page_evicted");
}

void TestBufferPool::test_all_pinned_returns_error() {
    std::remove("mydb.db");
    const int CAP = 2;
    BufferPool bp(CAP);

    int id0 = bp.create_new_page();
    int id1 = bp.create_new_page();
    assert(id0 >= 0);
    assert(id1 >= 0);

    int id2 = bp.create_new_page();
    assert(id2 == -1);

    std::remove("mydb.db");
    pass("test_all_pinned_returns_error");
}

void TestBufferPool::test_written_data_persists_after_eviction() {
    std::remove("mydb.db");
    const int CAP = 2;
    BufferPool bp(CAP);

    int page_id = bp.create_new_page();
    char* data = bp.fetch_page(page_id);
    assert(data != nullptr);
    snprintf(data, PAGE_SIZE, "persistent data");

    bp.unpin_page(page_id, true);

    int other_a = bp.create_new_page();
    assert(other_a >= 0);

    char* data2 = bp.fetch_page(page_id);
    assert(data2 != nullptr);
    assert(strncmp(data2, "persistent data", 15) == 0);

    std::remove("mydb.db");
    pass("test_written_data_persists_after_eviction");
}

void TestBufferPool::test_clean_page_evicted_no_corruption() {
    std::remove("mydb.db");
    const int CAP = 2;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    int id_b = bp.create_new_page();

    bp.unpin_page(id_a, false);
    bp.unpin_page(id_b, false);

    int id_c = bp.create_new_page();
    assert(id_c >= 0);

    char* data_b = bp.fetch_page(id_b);
    assert(data_b != nullptr);

    std::remove("mydb.db");
    pass("test_clean_page_evicted_no_corruption");
}

void TestBufferPool::test_dirty_flag_reset_after_reload() {
    std::remove("mydb.db");
    const int CAP = 2;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    char* data = bp.fetch_page(id_a);
    snprintf(data, PAGE_SIZE, "dirty content");
    bp.unpin_page(id_a, true);

    int id_b = bp.create_new_page();
    assert(id_b >= 0);

    char* reloaded = bp.fetch_page(id_a);
    assert(reloaded != nullptr);
    assert(strncmp(reloaded, "dirty content", 13) == 0);

    std::remove("mydb.db");
    pass("test_dirty_flag_reset_after_reload");
}

// ─────────────────────────────────────────
//  LRU integration tests
// ─────────────────────────────────────────

void TestBufferPool::test_lru_eviction_order() {
    std::remove("mydb.db");
    const int CAP = 3;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    int id_b = bp.create_new_page();
    int id_c = bp.create_new_page();

    bp.unpin_page(id_a, false);
    bp.unpin_page(id_b, false);
    bp.unpin_page(id_c, false);

    int id_d = bp.create_new_page();
    assert(id_d >= 0);

    char* data_b = bp.fetch_page(id_b);
    char* data_c = bp.fetch_page(id_c);
    assert(data_b != nullptr);
    assert(data_c != nullptr);

    std::remove("mydb.db");
    pass("test_lru_eviction_order");
}

void TestBufferPool::test_fetch_updates_recency() {
    std::remove("mydb.db");
    const int CAP = 2;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    int id_b = bp.create_new_page();

    bp.unpin_page(id_a, false);
    bp.unpin_page(id_b, false);

    bp.fetch_page(id_a);
    bp.unpin_page(id_a, false);

    int id_c = bp.create_new_page();
    assert(id_c >= 0);

    char* data_a = bp.fetch_page(id_a);
    assert(data_a != nullptr);

    std::remove("mydb.db");
    pass("test_fetch_updates_recency");
}

void TestBufferPool::test_repin_prevents_early_eviction() {
    std::remove("mydb.db");
    const int CAP = 3;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    int id_b = bp.create_new_page();
    int id_c = bp.create_new_page();

    bp.unpin_page(id_a, false);
    bp.unpin_page(id_b, false);
    bp.unpin_page(id_c, false);

    bp.fetch_page(id_a);
    bp.unpin_page(id_a, false);

    int id_d = bp.create_new_page();
    assert(id_d >= 0);

    char* data_a = bp.fetch_page(id_a);
    char* data_c = bp.fetch_page(id_c);
    assert(data_a != nullptr);
    assert(data_c != nullptr);

    std::remove("mydb.db");
    pass("test_repin_prevents_early_eviction");
}

void TestBufferPool::test_multiple_sequential_evictions() {
    std::remove("mydb.db");
    const int CAP = 3;
    BufferPool bp(CAP);

    int id_a = bp.create_new_page();
    int id_b = bp.create_new_page();
    int id_c = bp.create_new_page();

    bp.unpin_page(id_a, false);
    bp.unpin_page(id_b, false);
    bp.unpin_page(id_c, false);

    int id_d = bp.create_new_page();
    assert(id_d >= 0);
    bp.unpin_page(id_d, false);

    int id_e = bp.create_new_page();
    assert(id_e >= 0);
    bp.unpin_page(id_e, false);

    char* data_c = bp.fetch_page(id_c);
    char* data_d = bp.fetch_page(id_d);
    assert(data_c != nullptr);
    assert(data_d != nullptr);

    std::remove("mydb.db");
    pass("test_multiple_sequential_evictions");
}

// ─────────────────────────────────────────
//  Runner
// ─────────────────────────────────────────

void TestBufferPool::run_all() {
    std::cout << "===============================" << std::endl;
    std::cout << "   DISK MANAGER TESTS          " << std::endl;
    std::cout << "===============================" << std::endl;
    test_allocate_page_increments();
    test_write_read_roundtrip();
    test_multiple_pages_independent();
    test_overwrite_page();
    test_many_allocations_sequential();
    test_fresh_page_is_zeroed();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   BUFFER POOL TESTS           " << std::endl;
    std::cout << "===============================" << std::endl;
    test_create_new_page_returns_valid_id();
    test_create_multiple_pages_unique_ids();
    test_fetch_nonexistent_page_returns_null();
    test_fetch_negative_page_id_returns_null();
    test_fetch_existing_page_returns_data();
    test_fetch_same_page_twice_cache_hit();
    test_fill_to_capacity();
    test_eviction_triggered_when_full();
    test_correct_page_evicted();
    test_all_pinned_returns_error();
    test_written_data_persists_after_eviction();
    test_clean_page_evicted_no_corruption();
    test_dirty_flag_reset_after_reload();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   LRU INTEGRATION TESTS       " << std::endl;
    std::cout << "===============================" << std::endl;
    test_lru_eviction_order();
    test_fetch_updates_recency();
    test_repin_prevents_early_eviction();
    test_multiple_sequential_evictions();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   All tests passed! ✅        " << std::endl;
    std::cout << "===============================" << std::endl;
}
