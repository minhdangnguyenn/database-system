#ifndef TEST_BPLUS_TREE_H
#define TEST_BPLUS_TREE_H

#include <string>

class TestBPlusTree {
  public:
    // provided tests (already in your code)
    void run_all();
    void pass(const std::string &name);

    // ---------------------------
    // Existing tests
    // ---------------------------
    void test_insert_and_lookup_single_key();
    void test_insert_and_lookup_multiple_keys();
    void test_lookup_nonexistent_key_returns_sentinel();
    void test_insert_reverse_order();
    void test_insert_random_order();
    void test_leaf_split_all_keys_accessible();
    void test_multiple_splits_all_keys_accessible();
    void test_root_split_keys_accessible();
    void test_delete_single_key();
    void test_delete_nonexistent_key_no_crash();
    void test_delete_some_keys_others_remain();
    void test_delete_all_keys();
    void test_range_scan_basic();
    void test_range_scan_empty_range();
    void test_range_scan_full_range();
    void test_insert_duplicate_key_overwrites();
    void test_lookup_empty_tree();
    void test_insert_boundary_keys();
    void test_lookup_single_key_raw_page();
    void test_lookup_multiple_keys_raw_page();
    void test_lookup_inner_to_leaf_raw_page();
    void test_lookup_key_below_minimum();
    void test_lookup_key_above_maximum();
    void test_lookup_idempotent();

    // ---------------------------
    // NEW extended tests
    // ---------------------------
    void test_deep_routing_multiple_levels();
    void test_leaf_next_chain_integrity();
    void test_insert_random_stress();
    void test_delete_until_single_leaf();
    void test_delete_random_stress();
    void test_large_range_scan_correctness();
    void test_persistence_reload_simulation();
    void test_massive_insert_lookup();
};

#endif // TEST_BPLUS_TREE_H
