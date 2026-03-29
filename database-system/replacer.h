#include <cstddef>
#ifndef REPLACER_H
#define REPLACER_H

class Replacer {
public:
    virtual bool evict(int& frame_id) = 0;
    virtual void pin(int, int) = 0;
    virtual void unpin (int, int) = 0;
    virtual size_t Size() = 0;
    virtual ~Replacer() = default;
};

#endif // REPLACER_H
