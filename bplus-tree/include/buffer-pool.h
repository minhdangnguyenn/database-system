#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <unordered_map>
#include "disk-manager.h"
#include "lru.h"
#include "page.h"

class BufferPool {
    public:
        BufferPool(const int capacity);
        void create_new_page();
        void fetch_page();
        void unpin_page(int page_id, bool is_dirty);
        void flush_page(int page_id);
        int get_frame();
        // std::unordered_map<int, Page*> map;
        // std::unordered_map<int, Page*> unpinned_map;
        // void pin(int key, int value);
        // void unpin(Page*);
        // int get(int key);
        // void remove_page(Page*);
        // void push_head(Page*);
        // void remove_unpin(Page*);
        ~BufferPool();

    private:
        int capacity;
        std::unordered_map<int, int> page_table; // Map page_id -> frame_id
        std::list<int> free_list; // list of free frame_ids
        Replacer * replacer; // eviction policy e.g. LRU
        DiskManager* disk;
        // Page* head;
        // Page* tail;

        // Page* utail;
        // Page* uhead;
};

#endif //LRU_CACHE_H
