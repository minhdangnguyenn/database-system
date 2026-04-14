#include "include/test-bplus-tree.h"
#include "../include/b-plus-tree.h"
#include <cassert>
#include <climits>
#include <iostream>
#include <vector>

// ─────────────────────────────────────────
//  helper — mirrors read_int
//  writes 4 bytes into a page at offset
// ─────────────────────────────────────────

static void write_int(char *page, int offset, int value) {
    *(int *)(page + offset) = value;
}

void TestBPlusTree::pass(const std::string &name) {
    std::cout << "[PASS] " << name << std::endl;
}

// ─────────────────────────────────────────
//  Basic insert / lookup — order 63
// ─────────────────────────────────────────

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

// ─────────────────────────────────────────
//  Splits — order 3
// ─────────────────────────────────────────

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
    for (int i = 1; i <= N; i++) {
        tree.insert(i * 10, i * 100);
    }

    for (int i = 1; i <= N; i++) {
        assert(tree.lookup(i * 10) == i * 100);
    }

    std::remove("mydb.db");
    pass("test_multiple_splits_all_keys_accessible");
}

void TestBPlusTree::test_root_split_keys_accessible() {
    std::remove("mydb.db");
    BufferPool bp(50);
    BPlusTree tree(&bp);

    const int N = 30;
    for (int i = 1; i <= N; i++) {
        tree.insert(i, i * 5);
    }

    for (int i = 1; i <= N; i++) {
        assert(tree.lookup(i) == i * 5);
    }

    std::remove("mydb.db");
    pass("test_root_split_keys_accessible");
}

// ─────────────────────────────────────────
//  Delete — order 3
// ─────────────────────────────────────────

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

    for (int i = 1; i <= 10; i++) {
        tree.insert(i * 10, i * 100);
    }

    for (int i = 1; i <= 10; i += 2) {
        tree.remove(i * 10);
    }

    for (int i = 1; i <= 10; i += 2) {
        assert(tree.lookup(i * 10) == -1);
    }

    for (int i = 2; i <= 10; i += 2) {
        assert(tree.lookup(i * 10) == i * 100);
    }

    std::remove("mydb.db");
    pass("test_delete_some_keys_others_remain");
}

void TestBPlusTree::test_delete_all_keys() {
    std::remove("mydb.db");
    BufferPool bp(20);
    BPlusTree tree(&bp);

    const int N = 10;
    for (int i = 1; i <= N; i++) {
        tree.insert(i, i * 10);
    }

    for (int i = 1; i <= N; i++) {
        tree.remove(i);
    }

    for (int i = 1; i <= N; i++) {
        assert(tree.lookup(i) == -1);
    }

    std::remove("mydb.db");
    pass("test_delete_all_keys");
}

// ─────────────────────────────────────────
//  Range scan — order 63
// ─────────────────────────────────────────

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
    for (int i = 1; i <= N; i++) {
        tree.insert(i * 10, i * 100);
    }

    std::vector<int> results;
    tree.range_scan(10, 50, results);

    assert(results.size() == 5);

    std::remove("mydb.db");
    pass("test_range_scan_full_range");
}

// ─────────────────────────────────────────
//  Edge cases — order 63
// ─────────────────────────────────────────

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

// ─────────────────────────────────────────
//  Lookup only — raw page writes
//  since insert is not yet implemented
//  these tests write raw bytes directly
//  into buffer pool pages and validate
//  that lookup reads them correctly
//
//  page layout:
//  byte [0-3]   → type         (0=LEAF, 1=INNER)
//  byte [4-7]   → num_keys
//  byte [8-11]  → next_page_id (leaf only, -1 if none)
//  byte [12+]   → keys         (4 bytes each)
//  byte [12 + num_keys*4 +] → values or children
// ─────────────────────────────────────────

// empty root leaf page — all lookups must return -1
void TestBPlusTree::test_lookup_single_key_raw_page() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0);   // type = LEAF
    write_int(page, 4, 1);   // num_keys = 1
    write_int(page, 8, -1);  // next_page_id = none
    write_int(page, 12, 42); // keys[0] = 42
    write_int(page, 16, 99); // values[0] = 99

    assert(tree.lookup(42) == 99);
    assert(tree.lookup(41) == -1);
    assert(tree.lookup(43) == -1);

    std::remove("mydb.db");
    pass("test_lookup_single_key_raw_page");
}

// 3 keys written into root leaf page
// validates: keys at 12+i*4, values at 12+num_keys*4+i*4
void TestBPlusTree::test_lookup_multiple_keys_raw_page() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    char *page = bp.fetch_page(tree.get_root_page_id());
    write_int(page, 0, 0); // type = LEAF
    write_int(page, 4, 3); // num_keys = 3
    write_int(page, 8, -1);
    // keys at [12], [16], [20]
    write_int(page, 12, 10);
    write_int(page, 16, 20);
    write_int(page, 20, 30);
    // values at [24], [28], [32]  (12 + 3*4 = 24)
    write_int(page, 24, 100);
    write_int(page, 28, 200);
    write_int(page, 32, 300);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);
    assert(tree.lookup(15) == -1);
    assert(tree.lookup(99) == -1);

    std::remove("mydb.db");
    pass("test_lookup_multiple_keys_raw_page");
}

// inner node page 0, left leaf page 1, right leaf page 2
// validates that binary_search follows correct child_page_id
void TestBPlusTree::test_lookup_inner_to_leaf_raw_page() {
    std::remove("mydb.db");
    BufferPool bp(10);
    BPlusTree tree(&bp);

    // inner root: keys=[20], children=[1, 2]
    char *root = bp.fetch_page(0);
    write_int(root, 0, 1); // type = INNER
    write_int(root, 4, 1); // num_keys = 1
    write_int(root, 8, -1);
    write_int(root, 12, 20); // keys[0] = 20
    // children start at 12 + 1*4 = 16
    write_int(root, 16, 1); // children[0] = page 1
    write_int(root, 20, 2); // children[1] = page 2

    // left leaf: keys=[10], values=[100]
    char *left = bp.fetch_page(1);
    write_int(left, 0, 0);
    write_int(left, 4, 1);
    write_int(left, 8, 2); // next_page_id = 2
    write_int(left, 12, 10);
    write_int(left, 16, 100);

    // right leaf: keys=[20, 30], values=[200, 300]
    char *right = bp.fetch_page(2);
    write_int(right, 0, 0);
    write_int(right, 4, 2);
    write_int(right, 8, -1);
    write_int(right, 12, 20);
    write_int(right, 16, 30);
    // values at 12 + 2*4 = 20
    write_int(right, 20, 200);
    write_int(right, 24, 300);

    assert(tree.lookup(10) == 100);
    assert(tree.lookup(20) == 200);
    assert(tree.lookup(30) == 300);
    assert(tree.lookup(15) == -1);
    assert(tree.lookup(99) == -1);

    std::remove("mydb.db");
    pass("test_lookup_inner_to_leaf_raw_page");
}

// keys below smallest must return -1
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

// keys above largest must return -1
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

// repeated calls must not corrupt page bytes
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

// ─────────────────────────────────────────
//  Runner
// ─────────────────────────────────────────

void TestBPlusTree::run_all() {
    std::cout << "===============================" << std::endl;
    std::cout << "       B PLUS TREE TEST        " << std::endl;
    std::cout << "===============================" << std::endl;

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   LOOKUP ONLY — RAW PAGES     " << std::endl;
    std::cout << "===============================" << std::endl;
    test_lookup_single_key_raw_page();
    test_lookup_multiple_keys_raw_page();
    test_lookup_inner_to_leaf_raw_page();
    test_lookup_key_below_minimum();
    test_lookup_key_above_maximum();
    test_lookup_idempotent();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   BASIC INSERT / LOOKUP       " << std::endl;
    std::cout << "===============================" << std::endl;
    test_insert_and_lookup_single_key();
    test_insert_and_lookup_multiple_keys();
    test_lookup_nonexistent_key_returns_sentinel();
    test_insert_reverse_order();
    test_insert_random_order();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   SPLITS                      " << std::endl;
    std::cout << "===============================" << std::endl;
    test_leaf_split_all_keys_accessible();
    test_multiple_splits_all_keys_accessible();
    test_root_split_keys_accessible();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   DELETE                      " << std::endl;
    std::cout << "===============================" << std::endl;
    test_delete_single_key();
    test_delete_nonexistent_key_no_crash();
    test_delete_some_keys_others_remain();
    test_delete_all_keys();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   RANGE SCAN                  " << std::endl;
    std::cout << "===============================" << std::endl;
    test_range_scan_basic();
    test_range_scan_empty_range();
    test_range_scan_full_range();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   EDGE CASES                  " << std::endl;
    std::cout << "===============================" << std::endl;
    test_insert_duplicate_key_overwrites();
    test_lookup_empty_tree();
    test_insert_boundary_keys();

    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "   All tests passed! ✅        " << std::endl;
    std::cout << "===============================" << std::endl;
}
