#ifndef LRU_REPLACER_NAIVE_H
#define LRU_REPLACER_NAIVE_H

#include "page.h"
#include "replacer.h"
#include <vector>

// Naive LRU replacer — O(n) pin/unpin, for benchmarking against LRUReplacer
class LRUReplacerNaive : public Replacer {
  public:
    LRUReplacerNaive() = default;

    void pin(Page *page) override;
    void unpin(Page *page) override;
    Page *evict() override;

  private:
    // front = MRU, back = LRU — only unpinned pages
    std::vector<Page *> cache;
};

#endif // LRU_REPLACER_NAIVE_H
