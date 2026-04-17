#include "../include/lfu-replacer.h"

LFUReplacer::LFUReplacer() : min_freq(0) {}

void LFUReplacer::increment_freq(int key) {
    int f = freq_map[key];

    // remove from current frequency bucket
    freq_list[f].erase(iter_map[key]);

    // if this bucket is now empty and it was the minimum, bump min_freq
    if (freq_list[f].empty()) {
        freq_list.erase(f);
        if (min_freq == f)
            min_freq = f + 1;
    }

    // insert into next frequency bucket (front = most recent)
    int new_f = f + 1;
    freq_map[key] = new_f;
    freq_list[new_f].push_front(key);
    iter_map[key] = freq_list[new_f].begin();
}

void LFUReplacer::unpin(Page *page) {
    int key = page->getKey();

    // already in unpinned pool — just increment frequency
    if (page_map.count(key)) {
        increment_freq(key);
        return;
    }

    // new entry — frequency starts at 1
    page_map[key] = page;
    freq_map[key] = 1;
    freq_list[1].push_front(key);
    iter_map[key] = freq_list[1].begin();

    // new entry always has the lowest possible freq
    min_freq = 1;
}

void LFUReplacer::pin(Page *page) {
    int key = page->getKey();

    if (!page_map.count(key))
        return;

    int f = freq_map[key];

    // remove from frequency bucket
    freq_list[f].erase(iter_map[key]);

    if (freq_list[f].empty()) {
        freq_list.erase(f);
        if (min_freq == f)
            min_freq = f + 1;
    }

    // remove from all maps
    page_map.erase(key);
    freq_map.erase(key);
    iter_map.erase(key);
}

Page *LFUReplacer::evict() {
    if (page_map.empty())
        return nullptr;

    // evict from the least frequent bucket
    // if tie -> evict the oldest (back of the list)
    auto &bucket = freq_list[min_freq];
    int victim_key = bucket.back();
    bucket.pop_back();

    if (bucket.empty())
        freq_list.erase(min_freq);

    Page *victim = page_map[victim_key];

    page_map.erase(victim_key);
    freq_map.erase(victim_key);
    iter_map.erase(victim_key);

    return victim;
}
