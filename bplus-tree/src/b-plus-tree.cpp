#include "../include/b-plus-tree.h"
#include <iostream>
#include <vector>

BPlusTree::BPlusTree(BufferPool *bp) : buffer_pool(bp) {

    int page_id = bp->create_new_page();
    this->root_page_id = page_id;

}

int BPlusTree::read_int(char* page, int offset) {
    return *(int*)(page + offset);
}

void BPlusTree::write_int(char* page, int offset, NODETYPE type) {
    std::cout << "NOT IMPLEMENTED YET" << std::endl;
}

int BPlusTree::binary_search(char * page, int nums_key, int key) {

    int low = 0;
    int high = nums_key - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int k = this->read_int(page, 12 + mid * 4);
        if ( k <= key) low = mid + 1;
        else high = mid - 1;
    }

    int children_start = 12 + nums_key * 4;

    return this->read_int(page, children_start + low * 4);
}

int BPlusTree::lookup(int key) {

    int current_page_id =this->root_page_id;

    while (true) {
        char* page = this->buffer_pool->fetch_page(current_page_id);

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
                int k = this->read_int(page, 12 + i*4);
                if (k == key) {
                    // read the corresponding value
                    int val = this->read_int(page, 12 + num_keys * 4 + i*4);
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
    }
}

void BPlusTree::insert(int key, int page_id) {

    std::cout << "NOT IMPLEMENTED" << std::endl;
}

void BPlusTree::remove(int key) {

     std::cout << "NOT IMPLEMENTED" << std::endl;
}

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {

     std::cout << "NOT IMPLEMENTED" << std::endl;
}
