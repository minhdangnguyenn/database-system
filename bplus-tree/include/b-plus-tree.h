#ifndef B_TREE
#define B_TREE

#include "./buffer-pool.h"
#include "./index-strategy.h"
#include <vector>

enum NODETYPE {
    LEAF,
    INNER
};

class BPlusTree : public IndexStrategy {
public:
  BPlusTree(BufferPool *bp);
  ~BPlusTree() = default;

  int get_root_page_id() { return this->root_page_id; }

  // return the page_id correspond with key value
  int lookup(int key) override;

  void insert(int key, int page_id) override;
  void remove(int key) override;
  void range_scan(int low, int high, std::vector<int> &results) override;

  // helper function to cast from raw char* to int in offset at page
  int read_int(char* page, int offset);

  void write_int(char* page, int offset, NODETYPE type);

  int binary_search(char * page, int nums_keys, int key);

private:
  BufferPool *buffer_pool;
  int root_page_id;
};

#endif // !B_TREE
