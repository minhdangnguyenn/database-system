#ifndef INDEX_H
#define INDEX_H

#include <vector>

class IndexStrategy {
public:
    virtual int lookup(int key) = 0;
    virtual void insert(int key, int page_id) = 0;
    virtual void remove(int key) = 0;
    virtual void range_scan(int low, int high, std::vector<int>& results) = 0;
    virtual ~IndexStrategy() = default;

private:

};

#endif //INDEX_H
