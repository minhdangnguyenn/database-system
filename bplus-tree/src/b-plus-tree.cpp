#include "../include/b-plus-tree.h"
#include <algorithm>
#include <iostream>
#include <vector>

BPlusTree::BPlusTree(BufferPool *bp, int node_cap)

    : buffer_pool(bp), node_cap(node_cap), root_node(nullptr) {
  this->root_node = new Node(NodeType::LEAF);
  this->nodes.push_back(this->root_node);
}

int BPlusTree::lookup(int key) {

  Node *current_node = this->root_node;
  while (current_node->type == NodeType::INNER) {
    // find child idx of the current node
    // check comment in bplus-tree.h
    int child_idx = current_node->traverse(key);
    // after traverse -> get the idx of child node (leaf node)
    // traverse down, assign current node as the child node
    current_node = current_node->children_nodes[child_idx];
  }

  // after traverse, current is a leaf node
  for (int i = 0; i < current_node->keys.size(); i++) {
    if (current_node->keys[i] == key) {
      return current_node->values[i];
    }
  }

  // if not found match key -> return -1
  return -1;
}

void BPlusTree::insert(int key, int page_id) {

  Node *current_node = this->root_node;

  // Stage 1: walk down to correct leaf
  std::stack<std::pair<Node *, int>> path = {};
  while (current_node->type == NodeType::INNER) {
    int child_idx = current_node->traverse(key);

    path.push(std::pair(current_node, child_idx));
    current_node = current_node->children_nodes[child_idx];
  }

  // traverse until the node is not inner anymore -> leaf node
  // Stage 2: insert into leaf
  // insert at matching position i
  // lower boud find highest key that < int key params
  auto pos = std::lower_bound(current_node->keys.begin(),
                              current_node->keys.end(), key);

  int idx = pos - current_node->keys.begin();

  // insert keys into leaf node keys at position pos
  current_node->keys.insert(pos, key);

  // insert page_id into correspond idx in current_node values
  current_node->values.insert(current_node->values.begin() + idx, page_id);

  // Stage 3: check if node overfull
  // if overfull -> split the leaf
  if (current_node->keys.size() > this->node_cap) {
    int mid = current_node->keys.size() / 2;

    // current key becomes left node
    // copy values and keys for left node
    std::vector<int> left_keys(current_node->keys.begin(),
                               current_node->keys.begin() + mid);

    std::vector<int> right_keys(current_node->keys.begin() + mid,
                                current_node->keys.end());

    Node *right_node = new Node(NodeType::LEAF);
    this->nodes.push_back(right_node);

    right_node->keys = right_keys;
    right_node->values = std::vector<int>(current_node->values.begin() + mid,
                                          current_node->values.end());
    // overwrite keys for current node, current node -> leaf node
    current_node->keys = left_keys;

    current_node->values = std::vector<int>(current_node->values.begin(),
                                            current_node->values.begin() + mid);

    current_node->next_node = right_node;

    // save old root
    Node *old_root = this->root_node;
    int divider_key = right_node->keys.front();
    // put the right key.front() up to parent to become divider key
    while (!path.empty()) {
      Node *parent_node = path.top().first;
      int child_idx = path.top().second;
      path.pop();

      divider_key = right_node->keys.front();
      parent_node->keys.insert(parent_node->keys.begin() + child_idx,
                               divider_key);
      parent_node->children_nodes.insert(
          parent_node->children_nodes.begin() + child_idx + 1, right_node);

      // break when there isnt overflow
      if (parent_node->keys.size() <= this->node_cap) {
        break;
      }

      // else->parent overflow -> recursive split
      int mid = parent_node->keys.size() / 2;
      divider_key = parent_node->keys[mid];

      // split from bottom to top -> create a new inner -> no values
      right_node = new Node(NodeType::INNER);
      this->nodes.push_back(right_node);
      std::vector<int> right_keys(parent_node->keys.begin() + mid + 1,
                                  parent_node->keys.end());

      right_node->keys = right_keys;
      right_node->children_nodes =
          std::vector<Node *>(parent_node->children_nodes.begin() + mid + 1,
                              parent_node->children_nodes.end());

      parent_node->keys.erase(parent_node->keys.begin() + mid,
                              parent_node->keys.end());

      parent_node->children_nodes.erase(parent_node->children_nodes.begin() +
                                            mid + 1,
                                        parent_node->children_nodes.end());
    }

    // now path is empty after while loop
    Node *new_root = new Node(NodeType::INNER);
    this->root_node = new_root;
    new_root->children_nodes.push_back(old_root);
    new_root->children_nodes.push_back(right_node);
    new_root->keys.push_back(divider_key);
    this->nodes.push_back(new_root);
  }
}

void BPlusTree::remove(int key) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}
