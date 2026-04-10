#include "../include/b-plus-tree.h"
#include <iostream>
#include <ostream>

BPlusTree::BPlusTree(BufferPool *bp, int order)
    : buffer_pool(bp), order(order), root_page_id(-1) {
  root_page_id = buffer_pool->create_new_page();
}

int BPlusTree::lookup(int key) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
  return 0;
}

void BPlusTree::insert(int key, int page_id) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}

void BPlusTree::remove(int key) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}
