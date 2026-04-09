#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include "disk-manager.h"
#include "frame.h"
#include "lru.h"
#include <list>
#include <unordered_map>
#include <vector>

class BufferPool {
public:
    BufferPool(const int capacity);
    int create_new_page();
    char* fetch_page(int page_id);
    void unpin_page(int page_id, bool is_dirty);
    void flush_page(int page_id);
    int get_frame();
    ~BufferPool();

private:
    int capacity = 0;
    std::vector<Frame> frames;
    std::unordered_map<int, int> page_table; // Map page_id -> frame_id
    std::list<int> free_frame_list;          // list of free frame_ids
    Replacer *replacer;                    // eviction policy e.g. LRU
    DiskManager *disk;
};

#endif // LRU_CACHE_H
