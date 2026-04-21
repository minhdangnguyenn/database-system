#ifndef B_TREE
#define B_TREE

#include "./buffer-pool.h"
#include "./index-strategy.h"
#include <stack>
#include <utility>
#include <vector>

enum NODETYPE { LEAF, INNER };
static constexpr int TYPE_OFFSET = 0;
static constexpr int NUMKEYS_OFFSET = 4;
static constexpr int NEXTLEAF_OFFSET = 8;
static constexpr int KEY_START = 12;

class BPlusTree : public IndexStrategy {
  public:
    BPlusTree(BufferPool *bp);
    ~BPlusTree() = default;

    int get_root_page_id() { return this->root_page_id; }

    int get_type(char *page) { return read_int(page, TYPE_OFFSET); };
    int get_num_keys(char *page) { return read_int(page, NUMKEYS_OFFSET); }
    void set_num_keys(char *page, int n) { write_int(page, NUMKEYS_OFFSET, n); }
    int get_next_leaf(char *page) { return read_int(page, NEXTLEAF_OFFSET); }
    void set_next_leaf(char *page, int id) {
        write_int(page, NEXTLEAF_OFFSET, id);
    }
    // return the page_id correspond with key value
    int lookup(int key) override;

    void insert(int key, int page_id) override;
    void remove(int key) override;
    void range_scan(int low, int high, std::vector<int> &results) override;

    // helper function to cast from raw char* to int in offset at page
    int read_int(char *page, int offset);

    void write_int(char *page, int offset, int value);

    std::tuple<int, char *, std::stack<int>> find_leaf(int key);
    bool leaf_has_room(char *page);
    void insert_into_leaf(char *page, int key, int value);
    void insert_into_parent(char *page, int key, int value);

    void split_leaf(int leaf_page_id, int key, int value,
                    std::stack<int> parent_stack);

    void split_inner(int page_id, std::pair<int, std::stack<int>>);

    int binary_search(char *page, int nums_keys, int key);

  private:
    BufferPool *buffer_pool;
    int root_page_id;
};

#endif // !B_TREE
