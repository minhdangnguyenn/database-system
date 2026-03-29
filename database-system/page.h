#ifndef PAGE_H
#define PAGE_H

class Page {
private:
    int key;
    int value;

public:
    Page* prev = nullptr;
    Page* next = nullptr;
    Page* uprev = nullptr;
    Page* unext = nullptr;

    int getKey() const;
    int getValue() const;
    void setKey(int key);
    void setValue(int value);

    bool pinned = false;

    Page() : key(0), value(0), pinned(false) {}
    Page(int key, int value) :  
        key(key),
        value(value),
        pinned(false),
        prev(nullptr),
        next(nullptr),
        uprev(nullptr),
        unext(nullptr) {}
    ~Page() = default;
};

#endif
