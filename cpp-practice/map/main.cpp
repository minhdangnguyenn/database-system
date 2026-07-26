#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

int main() {
    std::map<std::string, int> ordered_dict;
    std::unordered_map<std::string, int> unordered_dict;

    // Inserting elements
    ordered_dict["apple"] = 5;
    unordered_dict["banana"] = 3;

    // Accessing elements
    std::cout << "Ordered: " << ordered_dict["apple"] << std::endl;
    std::cout << "Unordered: " << unordered_dict["banana"] << std::endl;

    return 0;
}
