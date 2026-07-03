#include <cstddef>
#include <cstring>
#ifndef DYNAMICARRAY

class DynamicArray {
  private:
    int *data;
    size_t size;
    size_t capacity;

  public:
    DynamicArray() {
        this->size = 0;
        this->capacity = 4;
        data = new int[capacity];
    };

    ~DynamicArray() { delete[] this->data; }
    void push_back(int value) {
        if (this->size == this->capacity) {
            // resize
            this->capacity *= 2;
            size_t newSize = this->capacity;
            int *newData = new int[newSize];
            std::memcpy(newData, this->data, this->size * sizeof(int));
            delete[] this->data;
            this->data = newData;
        }

        this->data[size] = value;
        this->size += 1;
    };
};

#endif // ! DYNAMICARRAY
