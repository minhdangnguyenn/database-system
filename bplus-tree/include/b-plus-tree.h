#ifndef B_TREE
#define B_TREE

#include "./index-strategy.h"
#include "./buffer-pool.h"

class BPlusTree: public IndexStrategy {
    public:
        BPlusTree(BufferPool* bp, int order);
        int lookup(int key) override;
        void insert(int key, int page_id) override;
        void remove(int key) override;
        void range_scan(int low, int high, std::vector<int>& results) override;
    private:
        BufferPool* buffer_pool;
        int root_page_id;
        int order;
};

#endif // !B_TREE
