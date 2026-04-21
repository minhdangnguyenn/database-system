#include "../include/page.h"

void Page::set_value(int val) { this->value = val; }
void Page::set_key(int key) { this->key = key; }
char *Page::get_data() { return this->data; }
int Page::get_key() const { return this->key; }
