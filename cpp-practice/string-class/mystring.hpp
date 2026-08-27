#include <cstddef>
#include <cstdio>
#ifndef STRING

class MyString {
private:
    std::size_t length;
    const char *data;

public:
    MyString(const char *data) {
        this->length = 0;
        this->data = data;

        auto current_char = data;

        while (*current_char != '\0') {
            current_char = current_char + 1; // pointer moves forward
            this->length++;
        }
    }
    size_t get_length() {
        return this->length;
    }
    const char *get_data() {
        return this->data;
    }
};

#endif // STRING
