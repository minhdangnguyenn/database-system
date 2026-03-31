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
    void pin(int) override;
    void unpin(int);
    size_t Size() override;
private:
    // orders store a list of frame_id, which can be removed
    std::list<int> orders; // e.g. LRU - frameid1 - frameid2 - MRU

    // map frame_id -> pointer to its address in orders
    std::unordered_map<int, std::list<int>::iterator> candidates; 
    size_t num_pages;
};

#endif //LRU_H
