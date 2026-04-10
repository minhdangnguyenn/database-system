#include "include/test-bplus-tree.h"
#include "include/test-buffer-pool.h"
#include "include/test-lru.h"

int main() {
  TEST_LRU lru_tests;
  lru_tests.run_all();

  TestBufferPool bp_tests;
  bp_tests.run_all();

  TestBPlusTree btree_tests;
  btree_tests.run_all();

  return 0;
}
