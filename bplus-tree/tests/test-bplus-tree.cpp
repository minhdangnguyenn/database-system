#include "include/test-bplus-tree.h"
#include "../include/b-plus-tree.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <vector>

// helper identical to your existing code
static void write_int(char *page, int offset, int value) {
    *(int *)(page + offset) = value;
}

void TestBPlusTree::pass(const std::string &name) {
    std::cout << "[PASS] " << name << std::endl;
}

// ============================================================================
//  BASIC INSERT + LOOKUP TESTS (your original tests)
// ============================================================================

void TestBPlusTree::test_insert_and_lookup_single_key() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    assert(tree.lookup(10) == 100);

    std::remove("mydb.db");
    pass("test_insert_and_lookup_single_key");
}

void TestBPlusTree::test_insert_and_lookup_multiple_keys() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);

    std::remove("mydb.db");
    pass("test_insert_and_lookup_multiple_keys");
}

void TestBPlusTree::test_lookup_nonexistent_key_returns_sentinel() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    assert(tree.lookup(999) == -1);

    std::remove("mydb.db");
    pass("test_lookup_nonexistent_key_returns_sentinel");
}

void TestBPlusTree::test_insert_reverse_order() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(30, 300);
    tree.insert(20, 200);
    tree.insert(10, 100);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);

    std::remove("mydb.db");
    pass("test_insert_reverse_order");
}

void TestBPlusTree::test_insert_random_order() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(50, 500);
    tree.insert(10, 100);
    tree.insert(40, 400);
    tree.insert(20, 200);
    tree.insert(30, 300);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);
    assert(tree.lookup(40) == 400);
    assert(tree.lookup(50) == 500);

    std::remove("mydb.db");
    pass("test_insert_random_order");
}

// ============================================================================
// SPLIT TESTS
// ============================================================================

void TestBPlusTree::test_leaf_split_all_keys_accessible() {
    std::remove("mydb.db");
    BufferPool bp(20);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    tree.insert(40, 400);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);
    assert(tree.lookup(40) == 400);

    std::remove("mydb.db");
    pass("test_leaf_split_all_keys_accessible");
}

void TestBPlusTree::test_multiple_splits_all_keys_accessible() {
    std::remove("mydb.db");
    BufferPool bp(50);
    BPlusTree tree(&bp);

    const int N = 20;
    for (int i = 1; i <= N; i++)
        tree.insert(i * 10, i * 100);
    for (int i = 1; i <= N; i++)
        assert(tree.lookup(i * 10) == i * 100);

    std::remove("mydb.db");
    pass("test_multiple_splits_all_keys_accessible");
}

void TestBPlusTree::test_root_split_keys_accessible() {
    std::remove("mydb.db");
    BufferPool bp(50);
    BPlusTree tree(&bp);

    const int N = 30;
    for (int i = 1; i <= N; i++)
        tree.insert(i, i * 5);
    for (int i = 1; i <= N; i++)
        assert(tree.lookup(i) == i * 5);

    std::remove("mydb.db");
    pass("test_root_split_keys_accessible");
}

// ============================================================================
// DELETE TESTS
// ============================================================================

void TestBPlusTree::test_delete_single_key() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.remove(10);
    assert(tree.lookup(10) == -1);

    std::remove("mydb.db");
    pass("test_delete_single_key");
}

void TestBPlusTree::test_delete_nonexistent_key_no_crash() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.remove(999);
    assert(tree.lookup(10) == 100);

    std::remove("mydb.db");
    pass("test_delete_nonexistent_key_no_crash");
}

void TestBPlusTree::test_delete_some_keys_others_remain() {
    std::remove("mydb.db");
    BufferPool bp(20);
    BPlusTree tree(&bp);

    for (int i = 1; i <= 10; i++)
        tree.insert(i * 10, i * 100);
    for (int i = 1; i <= 10; i += 2)
        tree.remove(i * 10);

    for (int i = 1; i <= 10; i += 2)
        assert(tree.lookup(i * 10) == -1);
    for (int i = 2; i <= 10; i += 2)
        assert(tree.lookup(i * 10) == i * 100);

    std::remove("mydb.db");
    pass("test_delete_some_keys_others_remain");
}

void TestBPlusTree::test_delete_all_keys() {
    std::remove("mydb.db");
    BufferPool bp(20);
    BPlusTree tree(&bp);

    const int N = 10;
    for (int i = 1; i <= N; i++)
        tree.insert(i, i * 10);
    for (int i = 1; i <= N; i++)
        tree.remove(i);

    for (int i = 1; i <= N; i++)
        assert(tree.lookup(i) == -1);

    std::remove("mydb.db");
    pass("test_delete_all_keys");
}

// ============================================================================
// RANGE SCAN TESTS
// ============================================================================

void TestBPlusTree::test_range_scan_basic() {
    std::remove("mydb.db");
    BufferPool bp(20);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    tree.insert(40, 400);
    tree.insert(50, 500);

    std::vector<int> results;
    tree.range_scan(20, 40, results);

    assert(results.size() == 3);
    assert(results[0] == 200);
    assert(results[1] == 300);
    assert(results[2] == 400);

    std::remove("mydb.db");
    pass("test_range_scan_basic");
}

void TestBPlusTree::test_range_scan_empty_range() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.insert(50, 500);

    std::vector<int> results;
    tree.range_scan(20, 40, results);
    assert(results.size() == 0);

    std::remove("mydb.db");
    pass("test_range_scan_empty_range");
}

void TestBPlusTree::test_range_scan_full_range() {
    std::remove("mydb.db");
    BufferPool bp(20);
    BPlusTree tree(&bp);

    const int N = 5;
    for (int i = 1; i <= N; i++)
        tree.insert(i * 10, i * 100);

    std::vector<int> results;
    tree.range_scan(10, 50, results);
    assert(results.size() == 5);

    std::remove("mydb.db");
    pass("test_range_scan_full_range");
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

void TestBPlusTree::test_insert_duplicate_key_overwrites() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(10, 100);
    tree.insert(10, 999);
    assert(tree.lookup(10) == 999);

    std::remove("mydb.db");
    pass("test_insert_duplicate_key_overwrites");
}

void TestBPlusTree::test_lookup_empty_tree() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    assert(tree.lookup(10) == -1);

    std::remove("mydb.db");
    pass("test_lookup_empty_tree");
}

void TestBPlusTree::test_insert_boundary_keys() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    tree.insert(0, 1);
    tree.insert(INT_MAX, 2);

    assert(tree.lookup(0) == 1);
    assert(tree.lookup(INT_MAX) == 2);

    std::remove("mydb.db");
    pass("test_insert_boundary_keys");
}

// ============================================================================
// RAW PAGE TESTS
// ============================================================================

void TestBPlusTree::test_lookup_single_key_raw_page() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0);
    write_int(page, 4, 1);
    write_int(page, 8, -1);
    write_int(page, 12, 42);
    write_int(page, 16, 99);

    assert(tree.lookup(42) == 99);
    assert(tree.lookup(41) == -1);
    assert(tree.lookup(43) == -1);

    std::remove("mydb.db");
    pass("test_lookup_single_key_raw_page");
}

void TestBPlusTree::test_lookup_multiple_keys_raw_page() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0);
    write_int(page, 4, 3);
    write_int(page, 8, -1);

    write_int(page, 12, 10);
    write_int(page, 16, 20);
    write_int(page, 20, 30);

    write_int(page, 24, 100);
    write_int(page, 28, 200);
    write_int(page, 32, 300);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);
    assert(tree.lookup(15) == -1);

    std::remove("mydb.db");
    pass("test_lookup_multiple_keys_raw_page");
}

void TestBPlusTree::test_lookup_inner_to_leaf_raw_page() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *root = bp.fetch_page(0);
    write_int(root, 0, 1);
    write_int(root, 4, 1);
    write_int(root, 8, -1);
    write_int(root, 12, 20);
    write_int(root, 16, 1);
    write_int(root, 20, 2);

    char *left = bp.fetch_page(1);
    write_int(left, 0, 0);
    write_int(left, 4, 1);
    write_int(left, 8, 2);
    write_int(left, 12, 10);
    write_int(left, 16, 100);

    char *right = bp.fetch_page(2);
    write_int(right, 0, 0);
    write_int(right, 4, 2);
    write_int(right, 8, -1);
    write_int(right, 12, 20);
    write_int(right, 16, 30);
    write_int(right, 20, 200);
    write_int(right, 24, 300);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);

    std::remove("mydb.db");
    pass("test_lookup_inner_to_leaf_raw_page");
}

void TestBPlusTree::test_lookup_key_below_minimum() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0);
    write_int(page, 4, 2);
    write_int(page, 8, -1);
    write_int(page, 12, 10);
    write_int(page, 16, 20);
    write_int(page, 20, 100);
    write_int(page, 24, 200);

    assert(tree.lookup(0) == -1);
    assert(tree.lookup(9) == -1);

    std::remove("mydb.db");
    pass("test_lookup_key_below_minimum");
}

void TestBPlusTree::test_lookup_key_above_maximum() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0);
    write_int(page, 4, 2);
    write_int(page, 8, -1);
    write_int(page, 12, 10);
    write_int(page, 16, 20);
    write_int(page, 20, 100);
    write_int(page, 24, 200);

    assert(tree.lookup(21) == -1);
    assert(tree.lookup(INT_MAX) == -1);

    std::remove("mydb.db");
    pass("test_lookup_key_above_maximum");
}

void TestBPlusTree::test_lookup_idempotent() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0);
    write_int(page, 4, 1);
    write_int(page, 8, -1);
    write_int(page, 12, 10);
    write_int(page, 16, 100);

    for (int i = 0; i < 10; i++) {
        assert(tree.lookup(10) == 100);
        assert(tree.lookup(99) == -1);
    }

    std::remove("mydb.db");
    pass("test_lookup_idempotent");
}

// ============================================================================
// NEW ADVANCED + STRESS TESTS
// ============================================================================

void TestBPlusTree::test_deep_routing_multiple_levels() {
    std::remove("mydb.db");
    BufferPool bp(200);
    BPlusTree tree(&bp);

    for (int i = 1; i <= 300; i++)
        tree.insert(i, i * 10);

    for (int i = 1; i <= 300; i++)
        assert(tree.lookup(i) == i * 10);

    std::remove("mydb.db");
    pass("test_deep_routing_multiple_levels");
}

void TestBPlusTree::test_leaf_next_chain_integrity() {
    std::remove("mydb.db");
    BufferPool bp(200);
    BPlusTree tree(&bp);

    for (int i = 1; i <= 50; i++)
        tree.insert(i, i * 10);

    std::vector<int> results;
    tree.range_scan(1, 50, results);

    assert(results.size() == 50);
    for (int i = 1; i <= 50; i++)
        assert(results[i - 1] == i * 10);

    std::remove("mydb.db");
    pass("test_leaf_next_chain_integrity");
}

void TestBPlusTree::test_insert_random_stress() {
    std::remove("mydb.db");
    BufferPool bp(500);
    BPlusTree tree(&bp);

    std::vector<int> keys;
    for (int i = 1; i <= 100; i++)
        keys.push_back(i);
    std::random_shuffle(keys.begin(), keys.end());

    for (int k : keys)
        tree.insert(k, k * 5);
    for (int i = 1; i <= 100; i++)
        assert(tree.lookup(i) == i * 5);

    std::remove("mydb.db");
    pass("test_insert_random_stress");
}

void TestBPlusTree::test_delete_until_single_leaf() {
    std::remove("mydb.db");
    BufferPool bp(200);
    BPlusTree tree(&bp);

    for (int i = 1; i <= 80; i++)
        tree.insert(i, i * 10);
    for (int i = 1; i <= 78; i++)
        tree.remove(i);

    assert(tree.lookup(79) == 790);
    assert(tree.lookup(80) == 800);

    std::remove("mydb.db");
    pass("test_delete_until_single_leaf");
}

void TestBPlusTree::test_delete_random_stress() {
    std::remove("mydb.db");
    BufferPool bp(300);
    BPlusTree tree(&bp);

    const int N = 120;
    for (int i = 1; i <= N; i++)
        tree.insert(i, i * 10);

    std::vector<int> keys;
    for (int i = 1; i <= N; i++)
        keys.push_back(i);
    std::random_shuffle(keys.begin(), keys.end());

    for (int i = 0; i < 60; i++)
        tree.remove(keys[i]);

    for (int i = 0; i < 60; i++)
        assert(tree.lookup(keys[i]) == -1);
    for (int i = 60; i < N; i++)
        assert(tree.lookup(keys[i]) == keys[i] * 10);

    std::remove("mydb.db");
    pass("test_delete_random_stress");
}

void TestBPlusTree::test_large_range_scan_correctness() {
    std::remove("mydb.db");
    BufferPool bp(300);
    BPlusTree tree(&bp);

    for (int i = 1; i <= 200; i++)
        tree.insert(i, i * 2);

    std::vector<int> results;
    tree.range_scan(50, 150, results);

    assert(results.size() == 101);
    for (int i = 0; i < 101; i++)
        assert(results[i] == (50 + i) * 2);

    std::remove("mydb.db");
    pass("test_large_range_scan_correctness");
}

void TestBPlusTree::test_persistence_reload_simulation() {
    std::remove("mydb.db");

    {
        BufferPool bp(50);
        BPlusTree tree(&bp);
        for (int i = 1; i <= 30; i++)
            tree.insert(i, i * 100);
    }

    {
        BufferPool bp2(50);
        BPlusTree tree2(&bp2);
        for (int i = 1; i <= 30; i++)
            assert(tree2.lookup(i) == i * 100);
    }

    std::remove("mydb.db");
    pass("test_persistence_reload_simulation");
}

void TestBPlusTree::test_massive_insert_lookup() {
    std::remove("mydb.db");
    BufferPool bp(800);
    BPlusTree tree(&bp);

    for (int i = 1; i <= 1000; i++)
        tree.insert(i, i);
    for (int i = 1; i <= 1000; i++)
        assert(tree.lookup(i) == i);

    std::remove("mydb.db");
    pass("test_massive_insert_lookup");
}

// ============================================================================
// RUNNER — integrates old + new tests
// ============================================================================

void TestBPlusTree::run_all() {
    std::cout << "===============================" << std::endl;
    std::cout << "       B PLUS TREE TEST        " << std::endl;
    std::cout << "===============================" << std::endl;

    std::cout << "\n--- RAW PAGE TESTS ---\n";
    test_lookup_single_key_raw_page();
    test_lookup_multiple_keys_raw_page();
    test_lookup_inner_to_leaf_raw_page();
    test_lookup_key_below_minimum();
    test_lookup_key_above_maximum();
    test_lookup_idempotent();

    std::cout << "\n--- BASIC INSERT / LOOKUP ---\n";
    test_insert_and_lookup_single_key();
    test_insert_and_lookup_multiple_keys();
    test_lookup_nonexistent_key_returns_sentinel();
    test_insert_reverse_order();
    test_insert_random_order();

    std::cout << "\n--- SPLITS ---\n";
    test_leaf_split_all_keys_accessible();
    test_multiple_splits_all_keys_accessible();
    test_root_split_keys_accessible();

    std::cout << "\n--- DELETE ---\n";
    test_delete_single_key();
    test_delete_nonexistent_key_no_crash();
    test_delete_some_keys_others_remain();
    test_delete_all_keys();

    std::cout << "\n--- RANGE SCAN ---\n";
    test_range_scan_basic();
    test_range_scan_empty_range();
    test_range_scan_full_range();

    std::cout << "\n--- EDGE CASES ---\n";
    test_insert_duplicate_key_overwrites();
    test_lookup_empty_tree();
    test_insert_boundary_keys();

    std::cout << "\n--- STRESS & STRUCTURAL TESTS ---\n";
    test_deep_routing_multiple_levels();
    test_leaf_next_chain_integrity();
    test_insert_random_stress();
    test_delete_until_single_leaf();
    test_delete_random_stress();
    test_large_range_scan_correctness();
    test_persistence_reload_simulation();
    test_massive_insert_lookup();

    std::cout << "\n===== ALL TESTS PASSED! =====\n";
}
