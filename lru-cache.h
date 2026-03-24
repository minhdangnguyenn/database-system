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
    std::unordered_map<int, Node*> um;
    void put(int key, int value);
    int get(int key);
    void removeNode(Node* node);
    void push_head(Node* node);
    ~LRUCache();

private:
    int capacity;
    Node* head;
    Node* tail;
};



#endif //LRU_CACHE_H
