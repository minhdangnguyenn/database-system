#include "../include/lru-replacer.h"

LRUReplacer::LRUReplacer() {
    uhead = new Page();
    utail = new Page();
    uhead->unext = utail;
    utail->uprev = uhead;
}

void LRUReplacer::unpin(Page *p) {
    if (unpinned_map.count(p->getKey()))
        return;

    unpinned_map[p->getKey()] = p;

    p->unext = uhead->unext;
    p->uprev = uhead;
    uhead->unext->uprev = p;
    uhead->unext = p;
}

void LRUReplacer::pin(Page *p) {
    if (unpinned_map.find(p->getKey()) == unpinned_map.end())
        return;

    remove_unpin(p);
    unpinned_map.erase(p->getKey());
}

Page *LRUReplacer::evict() {
    Page *victim = utail->uprev;
    if (victim == uhead)
        return nullptr; // all pinned

    remove_unpin(victim);
    unpinned_map.erase(victim->getKey());
    return victim;
}

void LRUReplacer::remove_unpin(Page *p) {
    p->uprev->unext = p->unext;
    p->unext->uprev = p->uprev;
}

LRUReplacer::~LRUReplacer() {
    delete uhead;
    delete utail;
}
