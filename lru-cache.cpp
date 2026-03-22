#include "lru-cache.h"

LRUCache::LRUCache(const int capacity) {
    this->capacity = capacity;
    this->dummy_head->key = -999;
    this->dummy_head->value = 0;
    this->dummy_head->prev = nullptr;
    this->dummy_head->next = nullptr;

    this->dummy_tail->key = 999;
    this->dummy_tail->value = 0;
    this->dummy_tail->prev = nullptr;

    // link tail and head
    this->dummy_tail->prev = this->dummy_head;
    this->dummy_head->next = this->dummy_tail;

    this->map = std::unordered_map<int, Node*>();
    // this->map.insert({this->dummy_head->key, this->dummy_head});
    // this->map.insert({this->dummy_tail->key, this->dummy_tail});
}

int LRUCache::get(int key) {
    if (this->map.find(key) != this->map.end()) {

    }
    return this->map[key]->value;
}

void LRUCache::put(int key, int value) {

}

void LRUCache::removeNode(Node* node) {
    
}

LRUCache::~LRUCache() {
    orders.clear();
    map.clear();
}
