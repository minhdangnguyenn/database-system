#include <cstddef>

template <typename T> class LinkedList {
private:
    struct Node {
        T data;
    };

    Node *head;
    Node *tail;

public:
    LinkedList();
    ~LinkedList();

    void push_front(const T &value);
    void push_back(const T &value);

    void pop_front();
    void pop_back();

    T &front();
    T &back();
    bool contains(const T &value) const;

    Node *find(const T &value);

    bool remove(const T &value);

    bool empty() const;
    std::size_t get_size() const;

    void clear();
};
