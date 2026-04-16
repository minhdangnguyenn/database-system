#include "../include/fifo-replacer.h"

FIFOReplacer::FIFOReplacer() {
    uhead = new Page();
    utail = new Page();
    uhead->unext = utail;
    utail->uprev = uhead;
}

void FIFOReplacer::pin(Page *page) {
    if (unpinned_map.find(page->getKey()) == unpinned_map.end())
        return;

    remove_unpin(page);
    unpinned_map.erase(page->getKey());
}

void FIFOReplacer::unpin(Page *page) {
    if (unpinned_map.count(page->getKey()))
        return;

    page->unext = uhead->unext;
    page->uprev = uhead;
    uhead->unext->uprev = page;
    uhead->unext = page;

    unpinned_map[page->getKey()] = page;
}

Page *FIFOReplacer::evict() {
    Page *victim = utail->uprev;
    if (victim == uhead)
        return nullptr;

    remove_unpin(victim);
    unpinned_map.erase(victim->getKey());
    return victim;
}

void FIFOReplacer::remove_unpin(Page *page) {
    page->uprev->unext = page->unext;
    page->unext->uprev = page->uprev;
}

FIFOReplacer::~FIFOReplacer() {
    delete uhead;
    delete utail;
}
