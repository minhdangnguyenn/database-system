#ifndef TEST_BPLUS_TREE_H
#define TEST_BPLUS_TREE_H

#include <cassert>
#include <climits>
#include <string>

class TestBPlusTree {
public:
    void pass(const std::string &name);

    void test_lookup_single_key_raw_page();
    void test_lookup_multiple_keys_raw_page();
    void test_lookup_inner_to_leaf_raw_page();
    void test_lookup_key_below_minimum();
    void test_lookup_key_above_maximum();
    void test_lookup_idempotent();
    void run_lookup_tests();

    // basic insert / lookup
    void test_insert_and_lookup_single_key();
    void test_insert_and_lookup_multiple_keys();
    void test_lookup_nonexistent_key_returns_sentinel();
    void test_lookup_empty_tree();
    void test_insert_reverse_order();
    void test_insert_random_order();

    // page serialization / deserialization
    // validates data survives raw byte round trip through buffer pool
    void test_page_roundtrip_single_key();
    void test_page_roundtrip_many_keys();

    // splits
    void test_leaf_split_all_keys_accessible();
    void test_leaf_split_keys_below_split_point();
    void test_leaf_split_keys_above_split_point();
    void test_multiple_splits_all_keys_accessible();
    void test_root_split_keys_accessible();
    void test_split_nonexistent_keys_still_return_sentinel();
    void test_split_reverse_order_insert();

    // inner node page ids
    // validates child page id traversal across tree levels
    void test_inner_node_child_page_ids_correct();
    void test_three_level_tree_lookup();

    // delete
    void test_delete_single_key();
    void test_delete_nonexistent_key_no_crash();
    void test_delete_some_keys_others_remain();
    void test_delete_all_keys();
    void test_delete_then_insert_same_key();

    // range scan
    void test_range_scan_basic();
    void test_range_scan_empty_range();
    void test_range_scan_full_range();
    void test_range_scan_across_multiple_leaf_pages();
    void test_range_scan_single_key_range();
    void test_range_scan_results_are_ordered();

    // edge cases
    void test_insert_duplicate_key_overwrites();
    void test_insert_boundary_keys();
    void test_large_number_of_inserts();
    void test_interleaved_insert_and_lookup();

    void run_all();
};

#endif // TEST_BPLUS_TREE_H
