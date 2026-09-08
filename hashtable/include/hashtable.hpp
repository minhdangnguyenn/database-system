#include <cstddef>
#include <optional>
#include <vector>

template <typename K, typename V> class HashTable {
private:
    std::vector<std::optional<std::pair<K, V>>> table;
    size_t size_;

public:
    HashTable(size_t capacity);
    bool insert(const K &key, const V &value);
    bool erase(const K &key);
    size_t size() const;
    size_t capacity() const;
    bool empty();
    size_t hashfunction(const K &key);
};
