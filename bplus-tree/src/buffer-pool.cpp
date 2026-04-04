#include "../include/buffer-pool.h"
#include "../include/disk-manager.h"
#include "../include/lru.h"
#include <cstdio>
#include <iostream>
#include <string>
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
char* BufferPool::fetch_page(int page_id) {
    // check if cache hit
    if (this->page_table.contains(page_id)) {
        int frame_id = this->page_table[page_id];
        this->replacer->pin(frame_id);
        return this->frames[frame_id].get_data();
    }
    // if the page_id does not exist
    // pick a free frame_id or evict one
    if (!this->free_frame_list.empty()) {
        int frame_id = this->free_frame_list.back();
        Frame* frame = &this->frames[frame_id];
        frame->set_pid(page_id);
        this->free_frame_list.pop_back();

        // write the new page to disk
        this->disk->read_page(page_id, frame->get_data());
        this->replacer->pin(frame_id);
        this->page_table[page_id] = frame_id;
        return frame->get_data();
    }
    // else if buffer pool has no free frame left -> evict one using replacer
    else {
        int frame_id = 0; // init with a sentinal value
        // if the frame is already dirty, write the old data into the disk first

        bool can_evict = this->replacer->evict(frame_id);
        if (!can_evict) {
            std::cout << "Replacer can not evict frame " +  std::to_string(frame_id) << std::endl;
            return nullptr;
        }
        // after evict, then the frame_id has read value
        // if the frame is dirty, it need to flush to disk first
        if (this->frames[frame_id].get_dirty()) {
            // get the currect page_id, not the page_id in param
            // page_id in param is a new page_id
            int current_page_id = this->frames[frame_id].get_pid();
            this->flush_page(current_page_id);
            this->page_table.erase(current_page_id);
        }

        Frame* current_frame = &this->frames[frame_id];
        current_frame->set_pid(page_id);
        std::cout << std::to_string(frame_id) + " has been successfully evicted !" << std::endl;
        this->disk->read_page(page_id, this->frames[frame_id].get_data());
        this->replacer->pin(frame_id);
        this->page_table[page_id] = frame_id;
        return this->frames[frame_id].get_data();
    }
}

BufferPool::~BufferPool() {
  free_frame_list.clear();
  page_table.clear();
}
