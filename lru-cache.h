#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <unordered_map>
#include "page.h"

struct Node {
    int key;
    int value;
    Node* next;
    Node* prev;
};

class BufferPool {
public:
    BufferPool(const int capacity);
    std::list<std::pair<int, int>> orders;
    std::unordered_map<int, Page*> map;
    void pin(int key, int value);
    int get(int key);
    void remove_page(Page* node);
    void push_head(Page* node);
    Page * findLRU
    ~BufferPool();

private:
    int capacity;
    Page* head;
    Page* tail;
};



#endif //LRU_CACHE_H
