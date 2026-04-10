#include "include/test-bplus-tree.h"
#include "../include/b-plus-tree.h"

void TestBPlusTree::pass(const std::string &name) {
  std::cout << "[PASS] " << name << std::endl;
}

// ─────────────────────────────────────────
//  Basic insert / lookup — order 63
// ─────────────────────────────────────────

void TestBPlusTree::test_insert_and_lookup_single_key() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 63);

  tree.insert(10, 100);

  assert(tree.lookup(10) == 100);

  std::remove("mydb.db");
  pass("test_insert_and_lookup_single_key");
}

void TestBPlusTree::test_insert_and_lookup_multiple_keys() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 63);

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
  BPlusTree tree(&bp, 63);

  tree.insert(10, 100);

  assert(tree.lookup(999) == -1);

  std::remove("mydb.db");
  pass("test_lookup_nonexistent_key_returns_sentinel");
}

void TestBPlusTree::test_insert_reverse_order() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 63);

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
  BPlusTree tree(&bp, 63);

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
//  order 3 means max 2 keys per node
//  splits trigger after 2 keys
// ─────────────────────────────────────────

void TestBPlusTree::test_leaf_split_all_keys_accessible() {
  std::remove("mydb.db");
  BufferPool bp(20);
  BPlusTree tree(&bp, 3); // splits after 2 keys

  tree.insert(10, 100);
  tree.insert(20, 200);
  tree.insert(30, 300); // triggers first leaf split
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
  BPlusTree tree(&bp, 3); // frequent splits with order 3

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
  BPlusTree tree(&bp, 3); // root split happens quickly with order 3

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
//  small order forces merge and borrow
//  to happen with few keys
// ─────────────────────────────────────────

void TestBPlusTree::test_delete_single_key() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 3);

  tree.insert(10, 100);
  tree.remove(10);

  assert(tree.lookup(10) == -1);

  std::remove("mydb.db");
  pass("test_delete_single_key");
}

void TestBPlusTree::test_delete_nonexistent_key_no_crash() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 3);

  tree.insert(10, 100);
  tree.remove(999); // must not crash

  assert(tree.lookup(10) == 100);

  std::remove("mydb.db");
  pass("test_delete_nonexistent_key_no_crash");
}

void TestBPlusTree::test_delete_some_keys_others_remain() {
  std::remove("mydb.db");
  BufferPool bp(20);
  BPlusTree tree(&bp, 3);

  for (int i = 1; i <= 10; i++) {
    tree.insert(i * 10, i * 100);
  }

  // delete odd indexed keys
  for (int i = 1; i <= 10; i += 2) {
    tree.remove(i * 10);
  }

  // odd indexed keys must be gone
  for (int i = 1; i <= 10; i += 2) {
    assert(tree.lookup(i * 10) == -1);
  }

  // even indexed keys must remain
  for (int i = 2; i <= 10; i += 2) {
    assert(tree.lookup(i * 10) == i * 100);
  }

  std::remove("mydb.db");
  pass("test_delete_some_keys_others_remain");
}

void TestBPlusTree::test_delete_all_keys() {
  std::remove("mydb.db");
  BufferPool bp(20);
  BPlusTree tree(&bp, 3);

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
//  range scan only needs leaf chain
//  no splits needed to test it
// ─────────────────────────────────────────

void TestBPlusTree::test_range_scan_basic() {
  std::remove("mydb.db");
  BufferPool bp(20);
  BPlusTree tree(&bp, 63);

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
  BPlusTree tree(&bp, 63);

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
  BPlusTree tree(&bp, 63);

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
  BPlusTree tree(&bp, 63);

  tree.insert(10, 100);
  tree.insert(10, 999); // same key, new value

  assert(tree.lookup(10) == 999);

  std::remove("mydb.db");
  pass("test_insert_duplicate_key_overwrites");
}

void TestBPlusTree::test_lookup_empty_tree() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 63);

  assert(tree.lookup(10) == -1);

  std::remove("mydb.db");
  pass("test_lookup_empty_tree");
}

void TestBPlusTree::test_insert_boundary_keys() {
  std::remove("mydb.db");
  BufferPool bp(10);
  BPlusTree tree(&bp, 63);

  tree.insert(0, 1);
  tree.insert(INT_MAX, 2);

  assert(tree.lookup(0) == 1);
  assert(tree.lookup(INT_MAX) == 2);

  std::remove("mydb.db");
  pass("test_insert_boundary_keys");
}

// ─────────────────────────────────────────
//  Runner
// ─────────────────────────────────────────

void TestBPlusTree::run_all() {
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
