#include "../include/b-plus-tree.h"
#include <iostream>
#include <stack>
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
        std::cout << "SPLIT LEAF NOT IMPLEMENTED YET !" << std::endl;
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

void BPlusTree::split_leaf(int leaf_page_id, std::stack<int> parent_stack) {

    // std::cout << "NOT IMPLEMENTED !" << std::endl;
    char *page = this->buffer_pool->fetch_page(leaf_page_id);
    int num_keys = this->read_int(page, 4);
    int old_next = this->read_int(page, 8);
}

void BPlusTree::remove(int key) { std::cout << "NOT IMPLEMENTED" << std::endl; }

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {

    std::cout << "NOT IMPLEMENTED" << std::endl;
}
