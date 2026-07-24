#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef DICTIONARY

class Dictionary {
private:
    std::vector<std::string> keys;
    std::vector<std::string> values;

public:
    Dictionary() {};
    std::string get(std::string key) {
        for (size_t i = 0; i < keys.size(); i++) {
            if (this->keys[i] == key) {
                return this->values[i];
            }
        }
        return nullptr;
    }

    void insert(std::string key, std::string value) {
        for (size_t i = 0; i < keys.size(); i++) {
            // if key already exist. update value
            if (this->keys[i] == key) {
                this->values[i] = value;
                break;
            }
        }

        // if key not exist yet
        this->keys.push_back(key);
        this->values.push_back(value);
    }
};

#endif // DICTIONARY
