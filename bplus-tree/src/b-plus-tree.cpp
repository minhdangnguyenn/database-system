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

    // Initialize a leaf
    write_int(page, TYPE_OFFSET, NODETYPE::LEAF);
    write_int(page, NUMKEYS_OFFSET, 0);
    write_int(page, NEXTLEAF_OFFSET, -1);

    bp->unpin_page(page_id, true);
}

// these 2 functions only cast integer fron offset
int BPlusTree::read_int(char *page, int offset) {
    return *reinterpret_cast<int *>(page + offset);
}

void BPlusTree::write_int(char *page, int offset, int value) {
    *reinterpret_cast<int *>(page + offset) = value;
}

int BPlusTree::binary_search(char *page, int num_keys, int key) {

    int low = 0;
    int high = num_keys - 1;

    // Keys are sorted ascending
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int mid_key = read_int(page, 12 + mid * 4);

        if (key < mid_key)
            high = mid - 1;
        else
            low = mid + 1;
    }

    // Children start immediately after keys
    int children_start = 12 + num_keys * 4;

    // child index = low
    return read_int(page, children_start + low * 4);
}

int BPlusTree::lookup(int key) {

    int current_pid = this->root_page_id;

    while (true) {

        char *page = buffer_pool->fetch_page(current_pid);

        int type = read_int(page, TYPE_OFFSET);
        int num_keys = read_int(page, NUMKEYS_OFFSET);

        if (type == NODETYPE::LEAF) {

            for (int i = 0; i < num_keys; i++) {
                int k = read_int(page, 12 + i * 4);
                if (k == key) {

                    int value = read_int(page, 12 + num_keys * 4 + i * 4);

                    buffer_pool->unpin_page(current_pid, false);
                    return value;
                }
            }

            buffer_pool->unpin_page(current_pid, false);
            return -1;
        }

        if (type == NODETYPE::INNER) {

            int next_pid = binary_search(page, num_keys, key);

            // Unpin current BEFORE descending
            buffer_pool->unpin_page(current_pid, false);
            current_pid = next_pid;
            continue;
        }

        // Should not happen, but safe cleanup
        buffer_pool->unpin_page(current_pid, false);
        return -1;
    }
}

std::tuple<int, char *, std::stack<int>> BPlusTree::find_leaf(int key) {

    int current_pid = this->root_page_id;
    std::stack<int> parents;

    char *page = buffer_pool->fetch_page(current_pid);
    int type = read_int(page, TYPE_OFFSET);

    while (type == NODETYPE::INNER) {

        int num_keys = read_int(page, NUMKEYS_OFFSET);

        // remember this inner node for potential splits later
        parents.push(current_pid);

        // find correct child using binary search
        int child_pid = binary_search(page, num_keys, key);

        // unpin current inner node before descending
        buffer_pool->unpin_page(current_pid, false);

        // move down to child
        current_pid = child_pid;
        page = buffer_pool->fetch_page(current_pid);
        type = read_int(page, TYPE_OFFSET);
    }

    // page is now a LEAF and still pinned
    // caller is responsible for unpinning it
    return {current_pid, page, parents};
}

void BPlusTree::insert(int key, int value) {

    auto [leaf_pid, leaf_page, parents] = find_leaf(key);

    int num_keys = get_num_keys(leaf_page);

    // check if key already exists
    for (int i = 0; i < num_keys; i++) {

        int k = read_int(leaf_page, 12 + i * 4);

        if (k == key) {
            // overwrite the value
            write_int(leaf_page, 12 + num_keys * 4 + i * 4, value);
            buffer_pool->unpin_page(leaf_pid, true);
            return;
        }
    }

    // check if leaf has room
    int max_keys = (PAGE_SIZE - 12) / 8;

    if (num_keys < max_keys) {
        // insert directly, then unpin
        insert_into_leaf(leaf_page, key, value);
        buffer_pool->unpin_page(leaf_pid, true);
        return;
    }

    // leaf is full, must split
    // split_leaf will unpin leaf_page internally
    // so we unpin here BEFORE calling split_leaf
    buffer_pool->unpin_page(leaf_pid, false);
    split_leaf(leaf_pid, key, value, parents);
}

void BPlusTree::insert_into_leaf(char *page, int key, int value) {

    int num = get_num_keys(page);

    // step 1: find insert position
    int pos = 0;
    while (pos < num && read_int(page, 12 + pos * 4) < key)
        pos++;

    // step 2: read all old values into temp
    int old_vals[num];
    for (int i = 0; i < num; i++)
        old_vals[i] = read_int(page, 12 + num * 4 + i * 4);

    // step 3: shift keys right
    for (int j = num - 1; j >= pos; j--) {
        int k = read_int(page, 12 + j * 4);
        write_int(page, 12 + (j + 1) * 4, k);
    }

    // step 4: write the new key
    write_int(page, 12 + pos * 4, key);

    // step 5: write all values into new positions
    // new value base = 12 + (num+1)*4
    int new_num = num + 1;

    int j = 0;
    for (int i = 0; i < new_num; i++) {
        if (i == pos) {
            // write the new value
            write_int(page, 12 + new_num * 4 + i * 4, value);
        } else {
            // write old value
            write_int(page, 12 + new_num * 4 + i * 4, old_vals[j]);
            j++;
        }
    }

    // step 6: update num_keys ---
    set_num_keys(page, new_num);
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
        write_int(root_data, 12, promote_key);          // key[0]
        write_int(root_data, 12 + 1 * 4, leaf_page_id); // child[0] = 16
        write_int(root_data, 12 + 1 * 4 + 4,
                  new_right_page_id); // child[1] = 20
        write_int(root_data, NEXTLEAF_OFFSET, -1);

        this->root_page_id = new_root_pid;
        this->buffer_pool->unpin_page(new_root_pid, true);
    }
    // if the parent is not empty
    else {
        int parent_pid = parent_stack.top();
        parent_stack.pop();

        char *pp = buffer_pool->fetch_page(parent_pid);
        int pnum = get_num_keys(pp);

        // read all old keys and children into temp
        int old_keys[pnum];
        int old_children[pnum + 1];

        for (int i = 0; i < pnum; i++)
            old_keys[i] = read_int(pp, 12 + i * 4);
        for (int i = 0; i <= pnum; i++)
            old_children[i] = read_int(pp, 12 + pnum * 4 + i * 4);

        // find insert position
        int ipos = 0;
        while (ipos < pnum && old_keys[ipos] < promote_key)
            ipos++;

        int new_pnum = pnum + 1;

        // write new keys (insert promote_key at ipos)
        int ki = 0;
        for (int i = 0; i < new_pnum; i++) {
            if (i == ipos)
                write_int(pp, 12 + i * 4, promote_key);
            else {
                write_int(pp, 12 + i * 4, old_keys[ki]);
                ki++;
            }
        }

        // write new children (insert new_right_page_id at ipos+1)
        int ci = 0;
        for (int i = 0; i <= new_pnum; i++) {
            if (i == ipos + 1)
                write_int(pp, 12 + new_pnum * 4 + i * 4, new_right_page_id);
            else {
                write_int(pp, 12 + new_pnum * 4 + i * 4, old_children[ci]);
                ci++;
            }
        }

        set_num_keys(pp, new_pnum);

        // check overflow
        int inner_max = (PAGE_SIZE - 16) / 8;

        buffer_pool->unpin_page(parent_pid, true);
        if (new_pnum > inner_max) {
            split_inner(parent_pid, std::pair(parent_pid, parent_stack));
        }
    }

    // step 7: unpin, call parent
    this->buffer_pool->unpin_page(leaf_page_id, true);
    this->buffer_pool->unpin_page(new_right_page_id, true);
}

void BPlusTree::split_inner(int pid,
                            std::pair<int, std::stack<int>> parent_info) {

    char *page = buffer_pool->fetch_page(pid);
    int num = get_num_keys(page);

    int keys[num];
    int children[num + 1];

    for (int i = 0; i < num; i++)
        keys[i] = read_int(page, 12 + i * 4);
    for (int i = 0; i <= num; i++)
        children[i] = read_int(page, 12 + num * 4 + i * 4);

    int mid = num / 2;
    int promote = keys[mid];
    int left_n = mid;
    int right_n = num - mid - 1;

    // rewrite left node
    for (int i = 0; i < left_n; i++)
        write_int(page, 12 + i * 4, keys[i]);
    for (int i = 0; i <= left_n; i++)
        write_int(page, 12 + left_n * 4 + i * 4, children[i]);
    set_num_keys(page, left_n);

    // create right node
    int right_pid = buffer_pool->create_new_page();
    char *rp = buffer_pool->fetch_page(right_pid);

    write_int(rp, TYPE_OFFSET, NODETYPE::INNER);
    write_int(rp, NEXTLEAF_OFFSET, -1);
    set_num_keys(rp, right_n);

    for (int i = 0; i < right_n; i++)
        write_int(rp, 12 + i * 4, keys[mid + 1 + i]);
    for (int i = 0; i <= right_n; i++)
        write_int(rp, 12 + right_n * 4 + i * 4, children[mid + 1 + i]);

    // push promote up
    if (parent_info.second.empty()) {
        int root_pid = buffer_pool->create_new_page();
        char *root = buffer_pool->fetch_page(root_pid);

        write_int(root, TYPE_OFFSET, NODETYPE::INNER);
        set_num_keys(root, 1);
        write_int(root, NEXTLEAF_OFFSET, -1);

        write_int(root, 12, promote);   // key[0]
        write_int(root, 16, pid);       // child[0]
        write_int(root, 20, right_pid); // child[1]

        root_page_id = root_pid;
        buffer_pool->unpin_page(root_pid, true);
    } else {
        int gp_pid = parent_info.second.top();
        parent_info.second.pop();

        char *gp = buffer_pool->fetch_page(gp_pid);
        int gp_num = get_num_keys(gp);

        int old_keys[gp_num];
        int old_children[gp_num + 1];

        for (int i = 0; i < gp_num; i++)
            old_keys[i] = read_int(gp, 12 + i * 4);
        for (int i = 0; i <= gp_num; i++)
            old_children[i] = read_int(gp, 12 + gp_num * 4 + i * 4);

        int ipos = 0;
        while (ipos < gp_num && old_keys[ipos] < promote)
            ipos++;

        int new_gp = gp_num + 1;

        int ki = 0;
        for (int i = 0; i < new_gp; i++) {
            if (i == ipos)
                write_int(gp, 12 + i * 4, promote);
            else {
                write_int(gp, 12 + i * 4, old_keys[ki]);
                ki++;
            }
        }

        int ci = 0;
        for (int i = 0; i <= new_gp; i++) {
            if (i == ipos + 1)
                write_int(gp, 12 + new_gp * 4 + i * 4, right_pid);
            else {
                write_int(gp, 12 + new_gp * 4 + i * 4, old_children[ci]);
                ci++;
            }
        }

        set_num_keys(gp, new_gp);

        int inner_max = (PAGE_SIZE - 16) / 8;

        buffer_pool->unpin_page(gp_pid, true);

        if (new_gp > inner_max) {
            split_inner(gp_pid, std::pair(gp_pid, parent_info.second));
        }
    }

    buffer_pool->unpin_page(pid, true);
    buffer_pool->unpin_page(right_pid, true);
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
