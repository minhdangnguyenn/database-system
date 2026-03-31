#include "page.h"

int Page::getKey() const { return this->key; }
int Page::getValue() const { return this->value; }
void Page::setValue(int val) {this->value = val;}
void Page::setKey(int key) {this->key = key;}
