#ifndef B_TREE
#define B_TREE

#include "./index-strategy.h"

class BPlusTree: public IndexStrategy {
    public:
        int lookup(int key) override;
        void insert(int key, int page_id) override;
        void remove(int key) override;
        void range_scan(int low, int high, std::vector<int>& results) override;
    private:

};

#endif // !B_TREE
