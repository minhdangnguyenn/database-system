#include "../include/buffer-pool.h"
#include "../include/lru.h"
#include "../include/disk-manager.h"
#include "../include/page.h"
#include <map>
#include <unordered_map>
#include <iostream>

BufferPool::BufferPool(const int capacity):capacity(capacity) {
    this->free_list = {};
    this->replacer = new LRU(capacity);
    const std::string DB_FILENAME = "mydb.db";
    this->disk = new DiskManager(DB_FILENAME);
    this->page_table = {};
}

void BufferPool::create_new_page() {
    int page_id = this->disk->allocate_page();
    // get the correspond frame_id with new page_id
    // return the memory of the frame_id to the bplustree
    // b tree writes node into its memory
    // b tree calls unpin the page_id
    this->disk->write_page(page_id, char *data);
}

BufferPool::~BufferPool() {
    free_list.clear();
    page_table.clear();
}
