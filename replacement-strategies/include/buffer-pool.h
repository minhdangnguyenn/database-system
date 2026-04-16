#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include "page.h"
#include "replacer.h"
#include <unordered_map>

class BufferPool {
  public:
    BufferPool(int capacity, Replacer *replacer);

    std::unordered_map<int, Page *> map;

    void pin(int key, int value);
    void unpin(Page *page);
    int get(int key);

    ~BufferPool();

  private:
    int capacity;
    Page *head;
    Page *tail;
    Replacer *replacer;

    void push_head(Page *page);
    void remove_page(Page *page);
};

#endif // BUFFER_POOL_H
