#include "../include/b-plus-tree.h"
#include <iostream>
#include <stack>
#include <tuple>
#include <utility>
#include <vector>

BPlusTree::BPlusTree(BufferPool *bp) : buffer_pool(bp) {

    int page_id = bp->create_new_page();
    char *page = bp->fetch_page(page_id);
    this->root_page_id = page_id;
    this->write_int(page, 0, NODETYPE::LEAF);
    this->write_int(page, 4, 0);
    this->write_int(page, 8, -1);

    bp->unpin_page(page_id, false);
}

// these 2 functions only cast integer fron offset
int BPlusTree::read_int(char *page, int offset) {
    return *(int *)(page + offset);
}

void BPlusTree::write_int(char *page, int offset, int value) {
    *(int *)(page + offset) = value;
}

int BPlusTree::binary_search(char *page, int nums_key, int key) {

    int low = 0;
    int high = nums_key - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int k = this->read_int(page, 12 + mid * 4);
        if (k <= key)
            low = mid + 1;
        else
            high = mid - 1;
    }

    int children_start = 12 + nums_key * 4;

    return this->read_int(page, children_start + low * 4);
}

int BPlusTree::lookup(int key) {

    int current_page_id = this->root_page_id;

    while (true) {
        char *page = this->buffer_pool->fetch_page(current_page_id);

        // page layout has 4 bytes for each attribute
        // byte [0-3]   → type       (4 bytes)
        // byte [4-7]   → num_keys   (4 bytes)
        // byte [8-11]  → next_page_id (4 bytes, leaf only)
        // byte [12+]   → keys array (4 bytes each)
        // byte [12 + num_keys*4 +] → values or children (4 bytes each)
        //
        // read page type starts from byte 0
        int type = this->read_int(page, 0);

        // read number of keys starts from byte 4
        int num_keys = this->read_int(page, 4);

        if (type == NODETYPE::LEAF) {
            // scan keys array
            for (int i = 0; i < num_keys; i++) {
                int k = this->read_int(page, 12 + i * 4);
                if (k == key) {
                    // read the corresponding value
                    int val = this->read_int(page, 12 + num_keys * 4 + i * 4);
                    return val;
                }
            }
            return -1;
        }

        if (type == NODETYPE::INNER) {
            // find the correct child page id using binary search on keys
            // then follow the child_page_id
            current_page_id = this->binary_search(page, num_keys, key);
        }
        this->buffer_pool->unpin_page(current_page_id, true);
    }
}

std::tuple<int, char *, std::stack<int>> BPlusTree::find_leaf(int key) {

    int current_page_id = this->root_page_id;
    std::stack<int> parent_stack;

    char *current_page = this->buffer_pool->fetch_page(current_page_id);
    int type = this->read_int(current_page, 0);

    while (type == NODETYPE::INNER) {
        // read number of keys starts from byte 4
        int num_keys = this->read_int(current_page, 4);
        parent_stack.push(current_page_id);

        // find the correspond child page id suitable with the key
        int child_page_id = this->binary_search(current_page, num_keys, key);

        this->buffer_pool->unpin_page(current_page_id, false);

        // traverse down to the child page id
        // traverse until the page is leaf
        current_page_id = child_page_id;
        current_page = this->buffer_pool->fetch_page(current_page_id);
        type = this->read_int(current_page, 0);
    }

    return {current_page_id, current_page, parent_stack};
}

void BPlusTree::insert(int key, int page_id) {

    // find down to the leaf
    std::tuple<int, char *, std::stack<int>> leaf_tuple = this->find_leaf(key);
    int leaf_page_id = std::get<0>(leaf_tuple);
    char *current_page = std::get<1>(leaf_tuple);
    std::stack<int> parent_stack = std::get<2>(leaf_tuple);
    // overwrite value if key already exist

    // read number of keys starts from byte 4
    int num_keys = this->read_int(current_page, 4);

    // scan keys array
    for (int i = 0; i < num_keys; i++) {
        int k = this->read_int(current_page, 12 + i * 4);
        if (k == key) {
            this->write_int(current_page, 12 + num_keys * 4 + i * 4, page_id);

            // a page should always be unpinned before return
            // mark its dirty as true if it has been modified
            this->buffer_pool->unpin_page(leaf_page_id, true);
            return;
        }
    }

    // check if the leaf still has slot
    // This comes from your page size and your layout. Each key-value pair takes
    // 8 bytes (4 for the key, 4 for the value). Your header takes 12 bytes. So
    // the max keys that fit in one page is: max_keys = (PAGE_SIZE - 12) / 8
    // Where PAGE_SIZE is whatever your buffer pool uses, typically 4096 bytes.

    int max_keys = (PAGE_SIZE - 12) / 8;
    if (num_keys < max_keys) {
        // leaf has room -> insert directly
        this->insert_into_leaf(current_page, key, page_id);
        this->buffer_pool->unpin_page(leaf_page_id, true);
    } else {
        // split leaf here
        // std::cout << "SPLIT LEAF NOT IMPLEMENTED YET !" << std::endl;
        this->split_leaf(leaf_page_id, key, page_id, parent_stack);
        this->buffer_pool->unpin_page(leaf_page_id, true);
    }
}

void BPlusTree::insert_into_leaf(char *page, int key, int value) {

    int num_keys = this->read_int(page, 4);

    // find the insert position
    int insert_pos = 0;
    while (insert_pos < num_keys) {
        int k = this->read_int(page, 12 + insert_pos * 4);

        // find the insert position i
        if (k > key) {
            break;
        }

        insert_pos++;
    }

    // shift key > insert key to the right to insert new key
    // need to loop from right to left and shift from left to right
    int j = num_keys - 1;

    // loop 1: relocate all values
    int old_value_start = 12 + num_keys * 4;
    int new_value_start = 12 + (num_keys + 1) * 4;

    while (j >= 0) {
        int v = this->read_int(page, old_value_start + j * 4);
        if (j >= insert_pos) {
            this->write_int(page, new_value_start + (j + 1) * 4, v);
        } else {
            this->write_int(page, new_value_start + j * 4, v);
        }

        j = j - 1;
    }

    j = num_keys - 1;

    // loop 2: shift key to the right
    while (j >= insert_pos) {
        int k = read_int(page, 12 + j * 4);
        this->write_int(page, 12 + (j + 1) * 4, k);
        j = j - 1;
    }

    this->write_int(page, 12 + insert_pos * 4, key);
    int increment_num_keys = num_keys + 1;

    this->write_int(page, 4, increment_num_keys);

    this->write_int(page, new_value_start + insert_pos * 4, value);
}

void BPlusTree::split_leaf(int leaf_page_id, int key, int value,
                           std::stack<int> parent_stack) {

    int max_keys = (PAGE_SIZE - 12) / 8;
    char *page = this->buffer_pool->fetch_page(leaf_page_id);
    int num_keys = this->read_int(page, 4);
    int old_next = this->read_int(page, 8);

    int temp_keys[max_keys + 1];
    int temp_values[max_keys + 1];

    // read existing keys and values into temp arrays
    // leave a gap at the correct position for the new key

    int insert_pos = 0;
    while (insert_pos < num_keys) {
        int k = this->read_int(page, 12 + insert_pos * 4);

        // find the insert position i
        if (k > key) {
            break;
        }

        insert_pos++;
    }

    int j = 0;
    for (int i = 0; i <= max_keys; i++) {
        if (i == insert_pos) {
            temp_keys[i] = key;
            temp_values[i] = value;
        } else {
            temp_keys[i] = this->read_int(page, 12 + j * 4);
            temp_values[i] = this->read_int(page, 12 + num_keys * 4 + j * 4);
            j++;
        }
    }

    // step 3: split point
    int mid = (max_keys + 1) / 2;
    // step 4: create and fill the right leaf

    // step 5: rewrite the left leaf
    for (int i = 0; i < mid; i++) {
        this->write_int(page, 12 + i * 4, temp_keys[i]);
        this->write_int(page, 12 + mid * 4 + i * 4, temp_values[i]);
    }

    // update the nums_key of the old page (now it is the left page)
    this->write_int(page, 4, mid);

    int new_right_page_id = this->buffer_pool->create_new_page();
    char *right_leaf_data = this->buffer_pool->fetch_page(new_right_page_id);

    int right_num_keys = max_keys - mid + 1;

    this->write_int(right_leaf_data, 0, 1);
    this->write_int(right_leaf_data, 4,
                    right_num_keys); // node type, 1 for leaf, 0 for inner
    this->write_int(right_leaf_data, 8, old_next);

    // assign keys for new right leaf
    // assign values for new right leaf
    for (int i = mid; i <= max_keys; i++) {
        this->write_int(right_leaf_data, 12 + (i - mid) * 4, temp_keys[i]);
        this->write_int(right_leaf_data,
                        12 + right_num_keys * 4 + (i - mid) * 4,
                        temp_values[i]);
    }
    this->write_int(page, 8, new_right_page_id);

    // step 6: push mid key point to inner
    int promote_key = temp_keys[mid];
    // if empty -> create new root
    if (parent_stack.empty()) {
        int new_root_pid = this->buffer_pool->create_new_page();
        char *root_data = this->buffer_pool->fetch_page(new_root_pid);

        this->write_int(root_data, 0, 0); // write type inner for inner node
        this->write_int(root_data, 4, 1); // write nums_key = 1
        this->write_int(root_data, 8, leaf_page_id); // write left child pointer
        this->write_int(root_data, 12, promote_key);
        this->write_int(root_data, 16, new_right_page_id);

        this->root_page_id = new_root_pid;
        this->buffer_pool->unpin_page(new_root_pid, true);
    }
    // if the parent is not empty
    else {
        int parent_page_id = parent_stack.top();
        parent_stack.pop();
        char *parent_data = this->buffer_pool->fetch_page(parent_page_id);
        int num_keys = this->read_int(parent_data, 4);

        // find the suitable position to insert new key
        int insert_pos = -1;
        for (int i = 0; i < num_keys; i++) {
            int k = this->read_int(parent_data, 12 + i * 8);
            if (k > promote_key) {
                insert_pos = i;
                break;
            }
        }
        if (insert_pos == -1) {
            insert_pos = num_keys;
        }

        // shift existing key and children
        for (int i = num_keys; i > insert_pos; i--) {
            int copy_key = this->read_int(parent_data, 12 + (i - 1) * 8);
            int copy_child = this->read_int(parent_data, 8 + i * 8);

            this->write_int(parent_data, 12 + i * 8, copy_key);
            this->write_int(parent_data, 8 + (i + 1) * 8, copy_child);
        }

        // insert into parent data
        this->write_int(parent_data, 12 + insert_pos * 8, promote_key);
        this->write_int(parent_data, 4, num_keys + 1);
        this->write_int(parent_data, 8 + (insert_pos + 1) * 8,
                        new_right_page_id);

        // check if parent is overflow
        int inner_max_keys = ((PAGE_SIZE - 8) / 4 - 1) / 2;
        if (num_keys + 1 > inner_max_keys) {
            this->split_inner(parent_page_id,
                              std::pair(parent_page_id, parent_stack));
        } else {
            this->buffer_pool->unpin_page(parent_page_id, true);
        }
    }

    // step 7: unpin, call parent
    this->buffer_pool->unpin_page(leaf_page_id, true);
    this->buffer_pool->unpin_page(new_right_page_id, true);
}

void BPlusTree::split_inner(int page_id,
                            std::pair<int, std::stack<int>> parent_stack) {
    char *inner_page = this->buffer_pool->fetch_page(page_id);
    int num_keys = this->read_int(inner_page, 4);
    int original_num_keys = num_keys;

    int original_keys[num_keys];
    int original_children[num_keys + 1];

    for (int i = 0; i < num_keys; i++) {
        original_keys[i] = this->read_int(inner_page, 12 + i * 8);
        original_children[i] = this->read_int(inner_page, 8 + i * 8);
    }

    // last member of children
    original_children[num_keys] = this->read_int(inner_page, 8 + num_keys * 8);

    // find split point
    int mid = num_keys / 2;
    int promote_key = original_keys[mid];

    // update into the left child node
    // 0 -> mid - 1 go to left child
    // mid + 1 -> nums_key - 1 go to right child
    // temp_keys[mid] goes up
    int left_num_keys = mid;
    for (int i = 0; i < mid; i++) {
        this->write_int(inner_page, 8 + i * 8, original_children[i]);
        this->write_int(inner_page, 12 + i * 8, original_keys[i]);
    }
    this->write_int(inner_page, 8 + mid * 8, original_children[mid]);
    num_keys = mid;
    this->write_int(inner_page, 4, mid);

    // create right inner node
    int new_right_inner_id = this->buffer_pool->create_new_page();
    char *new_right_inner_data =
        this->buffer_pool->fetch_page(new_right_inner_id);

    this->write_int(new_right_inner_data, 0, 0);
    int right_num_key = original_num_keys - mid - 1;
    for (int i = 0; i < right_num_key; i++) {
        this->write_int(new_right_inner_data, 8 + i * 8,
                        original_children[mid + 1 + i]);
        this->write_int(new_right_inner_data, 12 + i * 8,
                        original_keys[mid + 1 + i]);
    }

    // write the last key
    this->write_int(new_right_inner_data, 8 + right_num_key * 8,
                    original_children[original_num_keys]);
    this->write_int(new_right_inner_data, 4, right_num_key);

    // check if parent is overflow
    if (parent_stack.second.empty()) {
        // create new root with onn key promote key
        int new_root_id = this->buffer_pool->create_new_page();
        // left child is page_id, right child is new_right_inner_id
        char *root_data = this->buffer_pool->fetch_page(new_root_id);

        this->write_int(root_data, 0, 0); // write type inner for inner node
        this->write_int(root_data, 4, 1); // write nums_key = 1
        this->write_int(root_data, 8, page_id);
        this->write_int(root_data, 12, promote_key);
        this->write_int(root_data, 16, new_right_inner_id);

        this->root_page_id = new_root_id;
        this->buffer_pool->unpin_page(new_root_id, true);
    } else {
        // pop up the grandparent from parent stack
        int grandparent_id = parent_stack.second.top();
        parent_stack.second.pop();
        // insert promote_key and new_right_inner_id into it
        char *grandparent_data = this->buffer_pool->fetch_page(grandparent_id);
        int grandparent_num_keys = this->read_int(grandparent_data, 4);

        // find the suitable position to insert new key
        int insert_pos = -1;
        for (int i = 0; i < grandparent_num_keys; i++) {
            int k = this->read_int(grandparent_data, 12 + i * 8);
            if (k > promote_key) {
                insert_pos = i;
                break;
            }
        }
        if (insert_pos == -1) {
            insert_pos = grandparent_num_keys;
        }
        // check if grandparent overflow, if yes, call recursive
        for (int i = grandparent_num_keys; i > insert_pos; i--) {
            int copy_key = this->read_int(grandparent_data, 12 + (i - 1) * 8);
            int copy_child = this->read_int(grandparent_data, 8 + i * 8);

            this->write_int(grandparent_data, 12 + i * 8, copy_key);
            this->write_int(grandparent_data, 8 + (i + 1) * 8, copy_child);
        }

        // insert into parent data
        this->write_int(grandparent_data, 12 + insert_pos * 8, promote_key);
        this->write_int(grandparent_data, 4, grandparent_num_keys + 1);
        this->write_int(grandparent_data, 8 + (insert_pos + 1) * 8,
                        new_right_inner_id);

        // check if parent is overflow
        int inner_max_keys = ((PAGE_SIZE - 8) / 4 - 1) / 2;
        if (grandparent_num_keys + 1 > inner_max_keys) {
            this->split_inner(grandparent_id,
                              std::pair(grandparent_id, parent_stack.second));
        } else {
            this->buffer_pool->unpin_page(grandparent_id, true);
        }
    }
    this->buffer_pool->unpin_page(page_id, true);
    this->buffer_pool->unpin_page(new_right_inner_id, true);
}

void BPlusTree::remove(int key) {
    // std::cout << "NOT IMPLEMENTED" << std::endl;
    std::tuple<int, char *, std::stack<int>> leaf = this->find_leaf(key);
    int leaf_pid = std::get<0>(leaf);
    char *leaf_data = std::get<1>(leaf);
    int leaf_num_keys = this->read_int(leaf_data, 4);

    int delete_pos = -1;
    for (int i = 0; i < leaf_num_keys; i++) {
        int check_key = this->read_int(leaf_data, 12 + i * 4);
        if (check_key == key) {
            delete_pos = i;
            break;
        }
    }
    // if not found -> unpin, return
    if (delete_pos == -1) {
        std::cout << "DELETE KEY NOT FOUND !" << std::endl;
        this->buffer_pool->unpin_page(leaf_pid, false);
        return;
    }

    int original_leaf_keys[leaf_num_keys];
    int original_leaf_values[leaf_num_keys];

    for (int i = 0; i < leaf_num_keys; i++) {
        original_leaf_keys[i] = this->read_int(leaf_data, 12 + i * 4);
        original_leaf_values[i] =
            this->read_int(leaf_data, 12 + leaf_num_keys * 4 + i * 4);
    }

    // write everything back, skipping delete pos
    int new_num_keys = leaf_num_keys - 1;
    int j = 0;
    for (int i = 0; i < leaf_num_keys; i++) {
        if (i == delete_pos) {
            continue;
        }
        this->write_int(leaf_data, 12 + j * 4, original_leaf_keys[i]);
        this->write_int(leaf_data, 12 + new_num_keys * 4 + j * 4,
                        original_leaf_values[i]);

        j++;
    }

    // update num keys in the leaf page
    this->write_int(leaf_data, 4, new_num_keys);

    this->buffer_pool->unpin_page(leaf_pid, true);
}

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {

    std::tuple<int, char *, std::stack<int>> low_leaf = this->find_leaf(low);
    int low_leaf_pid = std::get<0>(low_leaf);

    char *low_leaf_data = std::get<1>(low_leaf);
    int low_leaf_num_keys = this->read_int(low_leaf_data, 4);
    int current_pid = low_leaf_pid;
    char *current_data = low_leaf_data;

    while (true) {
        int num_keys = this->read_int(current_data, 4);
        for (int i = 0; i < num_keys; i++) {
            int key = this->read_int(current_data, 12 + i * 4);

            if (low <= key && key <= high) {
                int value =
                    this->read_int(current_data, 12 + num_keys * 4 + i * 4);

                results.push_back(
                    this->read_int(current_data, 12 + num_keys * 4 + i * 4));
            }
            if (key > high) {
                this->buffer_pool->unpin_page(current_pid, false);
                return;
            }
        }

        int next_pid = this->read_int(current_data, 8);

        this->buffer_pool->unpin_page(current_pid, false);

        if (next_pid == -1) {
            return;
        }

        current_pid = next_pid;
        current_data = this->buffer_pool->fetch_page(current_pid);
    }
}
