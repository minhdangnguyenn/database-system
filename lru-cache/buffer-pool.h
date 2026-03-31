#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <unordered_map>
#include "page.h"

class BufferPool {
    public:
        BufferPool(const int capacity);
        std::unordered_map<int, Page*> map;
        std::unordered_map<int, Page*> unpinned_map;
        void pin(int key, int value);
        void unpin(Page*);
        int get(int key);
        void remove_page(Page*);
        void push_head(Page*);
        void remove_unpin(Page*);
        ~BufferPool();

    private:
        int capacity;
        Page* head;
        Page* tail;

        Page* utail;
        Page* uhead;
};

#endif //LRU_CACHE_H
