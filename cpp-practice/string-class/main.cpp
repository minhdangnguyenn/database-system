#include "mystring.hpp"
#include <cstdio>

int main() {
    const char *test = "abcdxyz";
    MyString *my_str = new MyString(test);

    printf("data from MyString: %s", my_str->get_data());
    printf("length from MyString: %zu", my_str->get_length());

    delete my_str;
}
