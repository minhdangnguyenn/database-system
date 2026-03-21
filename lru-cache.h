#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <unordered_map>

struct Node {
    int key;
    int value;
    Node* next;
    Node* prev;
};

class LRUCache {
public:
    LRUCache(const int capacity);
    std::list<std::pair<int, int>> orders;
    std::unordered_map<int, Node*> map;
    void put(int key, int value);
    int get(int key);
    ~LRUCache();

private:
    int capacity;
    Node* dummy_head;
    Node* dummy_tail;
};



#endif //LRU_CACHE_H
