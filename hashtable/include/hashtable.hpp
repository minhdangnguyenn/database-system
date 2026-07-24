#include <cstdint>
#ifndef HASHTABLE

struct tuple_t {
    uint32_t key;
    uint32_t rid;
    tuple_t() {
        key = 0;
        rid = 0;
    }
};

struct relation_t {
    tuple_t *data = nullptr;
    uint32_t number_tuples = 0;
    ~relation_t() {
        delete[] data;
    }
};

class HashTable {
private:
    relation_t R_;
    relation_t S_;
    void probe();
    void build();

public:
    HashTable(relation_t R_, relation_t S_) : R_(R_), S_(S_) {};
    ~HashTable() {};
};

#endif // HASHTABLE
