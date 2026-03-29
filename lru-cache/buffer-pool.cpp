#include "buffer-pool.h"
#include "page.h"
#include <unordered_map>
#include <iostream>

BufferPool::BufferPool(const int capacity) {
    this->capacity = capacity;
    this->head = new Page();
    this->tail = new Page();
    this->utail = new Page();
    this->uhead = new Page();

    // link tail and head
    this->tail->prev = this->head;
    this->head->next = this->tail;
    this->utail->uprev = this->uhead;
    this->uhead->unext = this->utail;

    this->map = std::unordered_map<int, Page*>();
    this->unpinned_map = std::unordered_map<int, Page*>();
}
// when get a key, the key becomes the most recently used -> need to move it to the front
int BufferPool::get(int key) {
    if (this->map.find(key) == this->map.end()) { // go to the end
        return -1;
    }

    // move the get node to the front MRU
    Page * page = this->map[key];
    this->remove_page(page);
    this->push_head(page);
    return page->getValue();
}

void BufferPool::pin(int key, int value) {
    // 3 cases to handle:
    // case 1: key already exist -> move to MRU
    if (this->map.find(key) != this->map.end()) {
        Page* p = this->map[key];
        p->setValue(value);
        this->remove_page(this->map[key]);
        this->push_head(this->map[key]);

        if (!p->pinned) {
            this->remove_unpin(p);
            this->unpinned_map.erase(key);
        }
    }
    // case 2: new key, cache not full -> add
    else if (this->map.size() < this->capacity) {
        Page* p = new Page(key, value);
        this->push_head(p);
        this->map[key] = p;
        p->pinned = true;
    }
    // case 3: new key, cache full -> evict LRU and add
    else {
        // evict lru
        Page* p = this->tail->prev;
        if (p == this->head) {
            std::cout << "ALL PAGES ARE PINNED" << std::endl;
            return;
        }
        if (!p->pinned) {
            this->remove_unpin(p);
            this->unpinned_map.erase(p->getKey());
        }
        this->remove_page(p);
        this->map.erase(p->getKey());
        delete p;

        // add new node
        Page* np = new Page(key, value);
        np->pinned = true;
        this->map.insert({key, np});
        this->push_head(np);
    }
}

void BufferPool::unpin(Page* p) {
    p->pinned = false;
    this->unpinned_map.insert({p->getKey(), p});

    p->unext = uhead->unext;
    p->uprev = uhead;

    uhead->unext->uprev = p;
    uhead->unext = p;
}

void BufferPool::remove_page(Page* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void BufferPool::remove_unpin(Page * p) {
    p->uprev->unext = p->unext;
    p->unext->uprev = p->uprev;
}

// push to head
void BufferPool::push_head(Page* page) {
    page->next = head->next; // point to the old first node
    page->prev = head;
    head->next->prev = page;
    head->next = page;
}

BufferPool::~BufferPool() {
    unpinned_map.clear();
    map.clear();
}
