#include "../include/buffer-pool.h"
#include "../include/disk-manager.h"
#include "../include/lru.h"
#include <iostream>
#include <unordered_map>

BufferPool::BufferPool(const int capacity) : capacity(capacity) {
    this->free_frame_list = {};
    this->replacer = new LRU(capacity);
    const std::string DB_FILENAME = "mydb.db";
    this->disk = new DiskManager(DB_FILENAME);
    this->page_table = {};
    this->frames.resize(capacity);
}

int BufferPool::create_new_page() {
    int page_id = this->disk->allocate_page();
    int frame_id = -1; // set a sentinel value for frame_id
    // frame is full
    if (this->free_frame_list.empty()) {
        bool is_evictable = this->replacer->evict(frame_id);
    if (!is_evictable) {
        std::cout << "The frame id " + std::to_string(frame_id) +
                        " is not evictable !"
                << std::endl;
        return -1;
    }
        // mapping new page_id with a free frame_id
        this->page_table[page_id] = frame_id;
    } else {
        // if there are still free frames in the buffer pool
        frame_id = this->free_frame_list.back();
        this->free_frame_list.pop_back();
        this->page_table.insert({page_id, frame_id});
    }
    this->replacer->pin(frame_id);
    return page_id;
}

// this function is opposite to the create_new_page
int BufferPool::fetch_page(int page_id) {
    // check if cache hit
    if (this->page_table.contains(page_id)) {
        int frame_id = this->page_table[page_id];
        this->replacer->pin(frame_id);
        return frame_id;
    }
    // if the page_id does not exist
    // pick a free frame_id or evict one
    if (!this->free_frame_list.empty()) {
        int frame_id = this->free_frame_list.back();
        Frame* frame = this->frames[frame_id];
        this->free_frame_list.pop_back();
        int page_id = this->page_table[frame_id];

        // write the new page to disk
        this->disk->read_page(page_id, frame->get_page()->get_data());
    } else {
    }
}

BufferPool::~BufferPool() {
  free_frame_list.clear();
  page_table.clear();
}
