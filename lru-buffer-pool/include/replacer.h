#ifndef REPLACER_H
#define REPLACER_H

#include "page.h"

class Replacer {
  public:
    virtual ~Replacer() = default;
    virtual void pin(Page *page) = 0;
    virtual void unpin(Page *page) = 0;
    virtual Page *evict() = 0;
};

#endif // REPLACER_H
