#include "../include/buffer-pool.h"
#include <iostream>

BufferPool::BufferPool(int capacity, Replacer *replacer)
    : capacity(capacity), replacer(replacer) {
    head = new Page();
    tail = new Page();
    tail->prev = head;
    head->next = tail;
    map = std::unordered_map<int, Page *>();
}

int BufferPool::get(int key) {
    if (map.find(key) == map.end())
        return -1;

    Page *page = map[key];
    remove_page(page);
    push_head(page);
    return page->getValue();
}

void BufferPool::pin(int key, int value) {
    // Case 1: key exists -> update and move to MRU
    if (map.find(key) != map.end()) {
        Page *p = map[key];
        p->setValue(value);
        remove_page(p);
        push_head(p);

        if (!p->pinned) {
            replacer->pin(p);
            p->pinned = true;
        }
    }
    // Case 2: new key, cache not full
    else if ((int)map.size() < capacity) {
        Page *p = new Page(key, value);
        p->pinned = true;
        push_head(p);
        map[key] = p;
    }
    // Case 3: new key, cache full -> evict via replacer, fallback to main list
    // LRU
    else {
        Page *victim = replacer->evict();

        // fallback: replacer has no candidates (all pinned), evict main list
        // LRU
        if (!victim) {
            victim = tail->prev;
            if (victim == head) {
                std::cout << "ALL PAGES ARE PINNED" << std::endl;
                return;
            }
        }

        remove_page(victim);
        map.erase(victim->getKey());
        delete victim;

        Page *np = new Page(key, value);
        np->pinned = true;
        map[key] = np;
        push_head(np);
    }
}

void BufferPool::unpin(Page *page) {
    page->pinned = false;
    replacer->unpin(page);
}

void BufferPool::remove_page(Page *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void BufferPool::push_head(Page *page) {
    page->next = head->next;
    page->prev = head;
    head->next->prev = page;
    head->next = page;
}

BufferPool::~BufferPool() {
    map.clear();
    delete head;
    delete tail;
}
