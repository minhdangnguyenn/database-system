#include "../include/buffer-pool.h"
#include "../include/lru.h"
#include "../include/disk-manager.h"
#include <unordered_map>
#include <iostream>
#include <string>
#include <unordered_map>

BufferPool::BufferPool(const int capacity) : capacity(capacity) {
    for (int i = 0; i < capacity; i++) {
       this->free_frame_list.push_back(i);
    }
    this->replacer = new LRU(capacity);
    const std::string DB_FILENAME = "mydb.db";
    this->disk = new DiskManager(DB_FILENAME);
    this->page_table = {};
    this->frames.resize(capacity);
}

int BufferPool::create_new_page() {
    int page_id = this->disk->allocate_page();
    int frame_id = -1; // set a sentinel value for frame_id
    // frames is full
    if (this->free_frame_list.empty()) {
        bool is_evictable = this->replacer->evict(frame_id); // frame_id will be init here
        if (!is_evictable) {
            std::cout << "The frame id " + std::to_string(frame_id) +
                            " is not evictable !"
                    << std::endl;
            return -1;
        }

        // update the page id correspond to the old frame
        int old_page_id = this->frames[frame_id].get_pid();

        // check if the frame correspond with the old page in buffer is dirty first
        if (this->frames[frame_id].get_dirty()) {
            this->flush_page(old_page_id);
        }

        this->page_table.erase(old_page_id);

        // mapping new page_id with a free frame_id
        this->page_table[page_id] = frame_id;
        this->frames[frame_id].set_pid(page_id);
    } else {
        // if there are still free frames in the buffer pool
        frame_id = this->free_frame_list.back();
        this->free_frame_list.pop_back();

        // match the frame in the free_frame with the new created page
        this->page_table.insert({page_id, frame_id});
        this->frames[frame_id].set_dirty(false);
        this->frames[frame_id].set_pid(page_id);
    }
    this->replacer->pin(frame_id);
    return page_id;
}

// this function is opposite to the create_new_page
char* BufferPool::fetch_page(int page_id) {
    // check if cache hit
    if (this->page_table.count(page_id)) {
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
        frame->set_dirty(false);
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
        int current_page_id = this->frames[frame_id].get_pid();
        if (this->frames[frame_id].get_dirty()) {
            // get the currect page_id, not the page_id in param
            // page_id in param is a new page_id
            this->flush_page(current_page_id);
        }
        this->page_table.erase(current_page_id);

        Frame* new_frame = &this->frames[frame_id];
        new_frame->set_pid(page_id);
        new_frame->set_dirty(false);

        std::cout << std::to_string(frame_id) + " has been successfully evicted !" << std::endl;
        this->disk->read_page(page_id, this->frames[frame_id].get_data());
        this->replacer->pin(frame_id);
        this->page_table[page_id] = frame_id;
        return this->frames[frame_id].get_data();
    }
}

/*
 * Flush a frame in buffer pool into disk
 */
void BufferPool::flush_page(int page_id) {
    auto iter = this->page_table.find(page_id);
    if (iter == this->page_table.end()) {
        std::cout << "PAGE ID" + std::to_string(page_id) + "CANNOT FOUND WHILE FLUSHING PAGE" << std::endl;
    } else {
        int frame_id = iter->second;
        Frame* target_frame = &this->frames[frame_id];

        // write of the page_id into disk
        this->disk->write_page(page_id, target_frame->get_data());
        target_frame->set_dirty(false);
    }
}

void BufferPool::unpin_page(int page_id, bool is_dirty) {
    if (!this->page_table.count(page_id)) return; // do nothing
    else {
        int frame_id = this->page_table.find(page_id)->second;
        Frame* target_frame = &this->frames[frame_id];

        if (target_frame->get_pin_count() == 0) return;
        target_frame->set_pin_count(this->frames[frame_id].get_pin_count()-1);

        if (is_dirty) {
            target_frame->set_dirty(true);
        }

        if (target_frame->get_pin_count() == 0) {
            this->replacer->unpin(frame_id);
        }
    }
}

BufferPool::~BufferPool() {
    this->free_frame_list.clear();
    this->page_table.clear();
    delete this->disk;
    delete this->replacer;
}
