#include "page.h"
#ifndef LRU_H
#define LRU_H
#include "replacer.h"
#include <unordered_map>
#include <list>

class LRU : public Replacer {
public:
    LRU(size_t num_pages);
    ~LRU() override = default;
    bool evict(int& frame_id) override;
    void pin(int, int) override;
    void unpin(int);
    size_t Size() override;
private:
    std::list<int> orders; // e.g. LRU - node1 - node2 - MRU
    std::unordered_map<int, std::list<int>::iterator> candidates; // map frame_id -> orders
    size_t num_pages;
};

#endif //LRU_H
