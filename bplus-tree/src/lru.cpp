#include "../include/lru.h"
#include <cstddef>

LRU::LRU(size_t num_pages) : num_pages(num_pages) {}
bool LRU::evict(int& frame_id) {
    if (orders.empty()) {
        return false;
    }
    // get the id of the leasr recently used frame
    frame_id = orders.front();
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

void LRU::pin(int frame_id) {
    if (candidates.count(frame_id)) {
        // this frame is now in use — remove it from the LRU list so it cannot be evicted
        orders.erase(candidates[frame_id]);
        // erase from the candidate map
        candidates.erase(frame_id);
    }
}

// unpin a frame id means that it can be a candidate to be evicted
void LRU::unpin(int frame_id) {
    if (!candidates.count(frame_id)) {
        orders.push_back(frame_id);
        // end() does not point to the last element
        // it points to invalid (at the end)
        // so need to use prev(end())
        candidates.insert({frame_id, std::prev(orders.end())});
    }
}

size_t LRU::Size() { return orders.size(); }
