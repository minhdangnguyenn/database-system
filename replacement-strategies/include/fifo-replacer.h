#ifndef FIFO_REPLACER_H
#define FIFO_REPLACER_H

#include "page.h"
#include "replacer.h"
#include <unordered_map>

class FIFOReplacer : public Replacer {
  public:
    FIFOReplacer();
    ~FIFOReplacer();

    void pin(Page *page) override;
    void unpin(Page *page) override;
    Page *evict() override;

  private:
    Page *uhead;
    Page *utail;
    std::unordered_map<int, Page *> unpinned_map;

    void remove_unpin(Page *page);
};

#endif // FIFO_REPLACER_H
