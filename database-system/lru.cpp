#include "lru.h"
#include <cstddef>

LRU::LRU(size_t num_pages) : num_pages(num_pages) {}
bool LRU::evict(int& frame_id) {
    if (orders.empty()) {
        return false;
    }
    // get the id of the leasr recentlz used frame
    frame_id = orders.back();
    if (candidates.find(frame_id) == candidates.end()) {
        return false;
    }

    auto frame = candidates[frame_id];

    // remove frame from orders
    orders.erase(frame);

    // remove frame from hash table
    candidates.erase(frame_id);

    return true;
}

void LRU::pin(int a, int b) {}
void LRU::unpin(int) {}
size_t LRU::Size() { return 0; }

// TODO: implement this class
