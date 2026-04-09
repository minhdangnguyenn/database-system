#ifndef FRAME_H
#define FRAME_H

#include "test-data.h"
class Frame {
public:
    Frame() = default;
    ~Frame() = default;
    bool get_dirty() const { return this->is_dirty; }
    void set_dirty(bool dirty) { this->is_dirty = dirty; }
    int get_pin_count() const { return this->pin_count; }
    void set_pin_count(int pin_count) { this->pin_count = pin_count; }
    int get_pid() const { return this->page_id; }
    void set_pid(int page_id) { this->page_id = page_id; }
    char* get_data() { return &data[PAGE_SIZE]; }
private:
    char data[PAGE_SIZE];
    int page_id;
    bool is_dirty;
    int pin_count;
};

#endif //FRAME_H
