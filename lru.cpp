#include "lru.h"

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


// TODO: implement this class
