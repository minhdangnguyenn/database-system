#ifndef PAGE_H
#define PAGE_H

#include "test-data.h"
class Page {
private:
    int key;
    int value;
    char data[PAGE_SIZE];

public:
    // Page* prev = nullptr;
    // Page* next = nullptr;
    // Page* uprev = nullptr;
    // Page* unext = nullptr;
    char* get_data();

    int get_key() const;
    int get_value() const;
    void set_key(int key);
    void set_value(int value);

    bool pinned = false;

    Page() : key(0), value(0), pinned(false) {}
    Page(int key, int value) :
        key(key),
        value(value),
        pinned(false) {}
        // prev(nullptr),
        // next(nullptr),
        // uprev(nullptr),
        // unext(nullptr) {}
    ~Page() = default;
};

#endif
