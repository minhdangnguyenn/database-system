#ifndef B_TREE
#define B_TREE

#include "./buffer-pool.h"
#include "./index-strategy.h"
#include <vector>

enum class NodeType { LEAF, INNER };

struct Node {
  NodeType type;
  std::vector<Node *> children_nodes;
  std::vector<int> keys;
  std::vector<int>
      values; // the vector holds corresponded values with the key in node
  Node *next_node;
  Node(NodeType type = NodeType::LEAF) : type(type) {
    this->next_node = nullptr;
  }
  // return the index of child in this->children_nodes
  int traverse(int key) {
    // check edge case
    if (this->keys.empty())
      return 0;
    if (key < this->keys.front()) {
      return 0; // first child
    }
    if (key > this->keys.back()) {
      return this->keys.size(); // for n keys, there are n+1 children
    } else {
      return this->binary_search(key);
    }
  };

  int binary_search(int key) {
    int low = 0;
    int high = this->keys.size() - 1;
    while (low <= high) {
      int mid = low + (high - low) / 2;

      // skip the left side
      if (this->keys[mid] <= key)
        low = mid + 1;
      // skip the right side
      else
        high = mid - 1;
    }
    return low;
  };
};

class BPlusTree : public IndexStrategy {
public:
  BPlusTree(BufferPool *bp, int node_capacity);
  ~BPlusTree();
  int lookup(int key) override;
  void insert(int key, int page_id) override;
  void remove(int key) override;
  void range_scan(int low, int high, std::vector<int> &results) override;

private:
  BufferPool *buffer_pool;
  int node_cap;
  std::vector<Node *> nodes;
  Node *root_node;
};

#endif // !B_TREE
