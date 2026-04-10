#ifndef TEST_BPLUS_TREE_H
#define TEST_BPLUS_TREE_H

#include "../../include/buffer-pool.h"
#include <cassert>
#include <climits>
#include <iostream>
#include <string>
#include <vector>

class TestBPlusTree {
public:
  void pass(const std::string &name);

  // basic insert / lookup
  void test_insert_and_lookup_single_key();
  void test_insert_and_lookup_multiple_keys();
  void test_lookup_nonexistent_key_returns_sentinel();
  void test_insert_reverse_order();
  void test_insert_random_order();

  // splits
  void test_leaf_split_all_keys_accessible();
  void test_multiple_splits_all_keys_accessible();
  void test_root_split_keys_accessible();

  // delete
  void test_delete_single_key();
  void test_delete_nonexistent_key_no_crash();
  void test_delete_some_keys_others_remain();
  void test_delete_all_keys();

  // range scan
  void test_range_scan_basic();
  void test_range_scan_empty_range();
  void test_range_scan_full_range();

  // edge cases
  void test_insert_duplicate_key_overwrites();
  void test_lookup_empty_tree();
  void test_insert_boundary_keys();

  void run_all();
};

#endif // TEST_BPLUS_TREE_H
