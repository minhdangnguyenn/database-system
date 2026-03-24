#include "lru-cache.h"

LRUCache::LRUCache(const int capacity) {
    this->capacity = capacity;
    this->head = new Node();
    this->tail = new Node();

    // link tail and head
    this->tail->prev = this->head;
    this->head->next = this->tail;

    this->um = std::unordered_map<int, Node*>();
}
// when get a key, the key becomes the most recently used -> need to move it to the front
int LRUCache::get(int key) {
    if (this->um.find(key) == this->um.end()) { // go to the end
        return -1;
    }

    // move the get node to the front MRU
    Node * node = this->um[key];
    this->removeNode(node);
    this->push_head(node);
    return node->value;
}

void LRUCache::put(int key, int value) {
    // 3 cases to handle:
    // case 1: key already exist -> move to MRU
    if (this->um.find(key) != this->um.end()) {
        this->um[key]->value = value;
        this->removeNode(this->um[key]);
        this->push_head(this->um[key]);
    }
    // case 2: new key, cache not full -> add
    else if (this->um.size() < this->capacity) {
        Node* n = new Node {key, value};
        this->push_head(n);
        this->um[key] = n;
    }
    // case 3: new key, cache full -> evict LRU and add
    else {
        // evict lru
        Node * n = this->tail->prev;
        this->removeNode(n);
        this->um.erase(n->key);
        delete n;

        // add new node to MRU
        Node * nn = new Node{key, value};
        this->um.insert({key, nn});
        this->push_head(nn);
    }
}

void LRUCache::removeNode(Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

// push to head
void LRUCache::push_head(Node* node) {
    node->next = head->next; // point to the old first node
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}

LRUCache::~LRUCache() {
    orders.clear();
    um.clear();
}
