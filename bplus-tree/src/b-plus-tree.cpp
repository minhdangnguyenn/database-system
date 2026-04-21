#include "../include/b-plus-tree.h"
#include <cstdio>
#include <stack>
#include <tuple>
#include <utility>
#include <vector>

static bool DEBUG_ENABLED = false;

#define DEBUG_LOG(fmt, ...)                                                    \
    do {                                                                       \
        if (DEBUG_ENABLED) {                                                   \
            fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__);               \
        }                                                                      \
    } while (0)

namespace {
constexpr int LEAF_MAX_KEYS = (PAGE_SIZE - KEY_START) / 8;
constexpr int INNER_MAX_KEYS = (PAGE_SIZE - 16) / 8;
} // namespace

BPlusTree::BPlusTree(BufferPool *bp) : buffer_pool(bp), root_page_id(0) {
    char *page = bp->fetch_page(root_page_id);

    // Fresh files contain an all-zero page 0. We keep page 0 as the root so
    // tests that write raw pages directly still interact with the real root.
    if (read_int(page, NEXTLEAF_OFFSET) == 0) {
        write_int(page, TYPE_OFFSET, NODETYPE::LEAF);
        write_int(page, NUMKEYS_OFFSET, 0);
        write_int(page, NEXTLEAF_OFFSET, -1);
        bp->unpin_page(root_page_id, true);
    } else {
        bp->unpin_page(root_page_id, false);
    }
}

int BPlusTree::read_int(char *page, int offset) {
    return *reinterpret_cast<int *>(page + offset);
}

void BPlusTree::write_int(char *page, int offset, int value) {
    *reinterpret_cast<int *>(page + offset) = value;
}

bool BPlusTree::leaf_has_room(char *page) {
    return get_num_keys(page) < LEAF_MAX_KEYS;
}

int BPlusTree::binary_search(char *page, int num_keys, int key) {
    int low = 0;
    int high = num_keys - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int mid_key = read_int(page, KEY_START + mid * 4);

        if (key < mid_key) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    int children_start = KEY_START + num_keys * 4;
    return read_int(page, children_start + low * 4);
}

int BPlusTree::lookup(int key) {
    int current_pid = root_page_id;

    while (true) {
        char *page = buffer_pool->fetch_page(current_pid);
        int type = get_type(page);
        int num_keys = get_num_keys(page);

        if (type == NODETYPE::LEAF) {
            for (int i = 0; i < num_keys; i++) {
                if (read_int(page, KEY_START + i * 4) == key) {
                    int value = read_int(page, KEY_START + num_keys * 4 + i * 4);
                    buffer_pool->unpin_page(current_pid, false);
                    return value;
                }
            }

            buffer_pool->unpin_page(current_pid, false);
            return -1;
        }

        int next_pid = binary_search(page, num_keys, key);
        buffer_pool->unpin_page(current_pid, false);
        current_pid = next_pid;
    }
}

std::tuple<int, char *, std::stack<int>> BPlusTree::find_leaf(int key) {
    int current_pid = root_page_id;
    std::stack<int> parents;
    char *page = buffer_pool->fetch_page(current_pid);

    while (get_type(page) == NODETYPE::INNER) {
        int next_pid = binary_search(page, get_num_keys(page), key);
        parents.push(current_pid);
        buffer_pool->unpin_page(current_pid, false);
        current_pid = next_pid;
        page = buffer_pool->fetch_page(current_pid);
    }

    return {current_pid, page, parents};
}

void BPlusTree::insert_into_leaf(char *page, int key, int value) {
    int num_keys = get_num_keys(page);
    int insert_pos = 0;

    while (insert_pos < num_keys &&
           read_int(page, KEY_START + insert_pos * 4) < key) {
        insert_pos++;
    }

    std::vector<int> keys(num_keys);
    std::vector<int> values(num_keys);

    for (int i = 0; i < num_keys; i++) {
        keys[i] = read_int(page, KEY_START + i * 4);
        values[i] = read_int(page, KEY_START + num_keys * 4 + i * 4);
    }

    keys.insert(keys.begin() + insert_pos, key);
    values.insert(values.begin() + insert_pos, value);

    int new_num_keys = num_keys + 1;
    for (int i = 0; i < new_num_keys; i++) {
        write_int(page, KEY_START + i * 4, keys[i]);
        write_int(page, KEY_START + new_num_keys * 4 + i * 4, values[i]);
    }
    set_num_keys(page, new_num_keys);
}

void BPlusTree::insert_into_parent(int left_page_id, int key, int right_page_id,
                                   std::stack<int> parent_stack) {
    if (parent_stack.empty()) {
        int new_root_pid = buffer_pool->create_new_page();
        char *root = buffer_pool->fetch_page(new_root_pid);

        write_int(root, TYPE_OFFSET, NODETYPE::INNER);
        set_num_keys(root, 1);
        write_int(root, NEXTLEAF_OFFSET, -1);
        write_int(root, KEY_START, key);
        write_int(root, KEY_START + 4, left_page_id);
        write_int(root, KEY_START + 8, right_page_id);

        root_page_id = new_root_pid;
        buffer_pool->unpin_page(new_root_pid, true);
        return;
    }

    int parent_pid = parent_stack.top();
    parent_stack.pop();

    char *parent = buffer_pool->fetch_page(parent_pid);
    int num_keys = get_num_keys(parent);
    std::vector<int> keys(num_keys);
    std::vector<int> children(num_keys + 1);

    for (int i = 0; i < num_keys; i++) {
        keys[i] = read_int(parent, KEY_START + i * 4);
    }
    for (int i = 0; i <= num_keys; i++) {
        children[i] = read_int(parent, KEY_START + num_keys * 4 + i * 4);
    }

    int insert_pos = 0;
    while (insert_pos < num_keys && keys[insert_pos] < key) {
        insert_pos++;
    }

    keys.insert(keys.begin() + insert_pos, key);
    children.insert(children.begin() + insert_pos + 1, right_page_id);

    int new_num_keys = num_keys + 1;
    for (int i = 0; i < new_num_keys; i++) {
        write_int(parent, KEY_START + i * 4, keys[i]);
    }
    for (int i = 0; i <= new_num_keys; i++) {
        write_int(parent, KEY_START + new_num_keys * 4 + i * 4, children[i]);
    }
    set_num_keys(parent, new_num_keys);

    buffer_pool->unpin_page(parent_pid, true);
    if (new_num_keys > INNER_MAX_KEYS) {
        split_inner(parent_pid, {parent_pid, parent_stack});
    }
}

void BPlusTree::split_leaf(int leaf_page_id, int key, int value,
                           std::stack<int> parent_stack) {
    char *leaf = buffer_pool->fetch_page(leaf_page_id);
    int num_keys = get_num_keys(leaf);
    int old_next = get_next_leaf(leaf);

    std::vector<int> keys(num_keys);
    std::vector<int> values(num_keys);
    for (int i = 0; i < num_keys; i++) {
        keys[i] = read_int(leaf, KEY_START + i * 4);
        values[i] = read_int(leaf, KEY_START + num_keys * 4 + i * 4);
    }

    int insert_pos = 0;
    while (insert_pos < num_keys && keys[insert_pos] < key) {
        insert_pos++;
    }

    keys.insert(keys.begin() + insert_pos, key);
    values.insert(values.begin() + insert_pos, value);

    int total_keys = num_keys + 1;
    int left_size = total_keys / 2;
    int right_size = total_keys - left_size;

    for (int i = 0; i < left_size; i++) {
        write_int(leaf, KEY_START + i * 4, keys[i]);
        write_int(leaf, KEY_START + left_size * 4 + i * 4, values[i]);
    }
    set_num_keys(leaf, left_size);

    int right_page_id = buffer_pool->create_new_page();
    char *right_leaf = buffer_pool->fetch_page(right_page_id);
    write_int(right_leaf, TYPE_OFFSET, NODETYPE::LEAF);
    set_num_keys(right_leaf, right_size);
    write_int(right_leaf, NEXTLEAF_OFFSET, old_next);

    for (int i = 0; i < right_size; i++) {
        write_int(right_leaf, KEY_START + i * 4, keys[left_size + i]);
        write_int(right_leaf,
                  KEY_START + right_size * 4 + i * 4,
                  values[left_size + i]);
    }

    set_next_leaf(leaf, right_page_id);
    int promote_key = keys[left_size];

    buffer_pool->unpin_page(leaf_page_id, true);
    buffer_pool->unpin_page(right_page_id, true);
    insert_into_parent(leaf_page_id, promote_key, right_page_id, parent_stack);
}

void BPlusTree::split_inner(int page_id,
                            std::pair<int, std::stack<int>> parent_info) {
    (void)parent_info.first;

    char *page = buffer_pool->fetch_page(page_id);
    int num_keys = get_num_keys(page);
    std::vector<int> keys(num_keys);
    std::vector<int> children(num_keys + 1);

    for (int i = 0; i < num_keys; i++) {
        keys[i] = read_int(page, KEY_START + i * 4);
    }
    for (int i = 0; i <= num_keys; i++) {
        children[i] = read_int(page, KEY_START + num_keys * 4 + i * 4);
    }

    int mid = num_keys / 2;
    int promote_key = keys[mid];
    int left_size = mid;
    int right_size = num_keys - mid - 1;

    for (int i = 0; i < left_size; i++) {
        write_int(page, KEY_START + i * 4, keys[i]);
    }
    for (int i = 0; i <= left_size; i++) {
        write_int(page, KEY_START + left_size * 4 + i * 4, children[i]);
    }
    set_num_keys(page, left_size);

    int right_page_id = buffer_pool->create_new_page();
    char *right_page = buffer_pool->fetch_page(right_page_id);
    write_int(right_page, TYPE_OFFSET, NODETYPE::INNER);
    set_num_keys(right_page, right_size);
    write_int(right_page, NEXTLEAF_OFFSET, -1);

    for (int i = 0; i < right_size; i++) {
        write_int(right_page, KEY_START + i * 4, keys[mid + 1 + i]);
    }
    for (int i = 0; i <= right_size; i++) {
        write_int(right_page,
                  KEY_START + right_size * 4 + i * 4,
                  children[mid + 1 + i]);
    }

    buffer_pool->unpin_page(page_id, true);
    buffer_pool->unpin_page(right_page_id, true);
    insert_into_parent(page_id, promote_key, right_page_id, parent_info.second);
}

void BPlusTree::insert(int key, int value) {
    auto [leaf_pid, leaf_page, parents] = find_leaf(key);
    int num_keys = get_num_keys(leaf_page);

    for (int i = 0; i < num_keys; i++) {
        if (read_int(leaf_page, KEY_START + i * 4) == key) {
            write_int(leaf_page, KEY_START + num_keys * 4 + i * 4, value);
            buffer_pool->unpin_page(leaf_pid, true);
            return;
        }
    }

    if (leaf_has_room(leaf_page)) {
        insert_into_leaf(leaf_page, key, value);
        buffer_pool->unpin_page(leaf_pid, true);
        return;
    }

    buffer_pool->unpin_page(leaf_pid, false);
    split_leaf(leaf_pid, key, value, parents);
}

void BPlusTree::remove(int key) {
    auto [leaf_pid, leaf_page, parents] = find_leaf(key);
    (void)parents;

    int num_keys = get_num_keys(leaf_page);
    int delete_pos = -1;
    for (int i = 0; i < num_keys; i++) {
        if (read_int(leaf_page, KEY_START + i * 4) == key) {
            delete_pos = i;
            break;
        }
    }

    if (delete_pos == -1) {
        buffer_pool->unpin_page(leaf_pid, false);
        return;
    }

    std::vector<int> keys;
    std::vector<int> values;
    keys.reserve(num_keys - 1);
    values.reserve(num_keys - 1);

    for (int i = 0; i < num_keys; i++) {
        if (i == delete_pos) {
            continue;
        }
        keys.push_back(read_int(leaf_page, KEY_START + i * 4));
        values.push_back(read_int(leaf_page, KEY_START + num_keys * 4 + i * 4));
    }

    int new_num_keys = num_keys - 1;
    for (int i = 0; i < new_num_keys; i++) {
        write_int(leaf_page, KEY_START + i * 4, keys[i]);
        write_int(leaf_page, KEY_START + new_num_keys * 4 + i * 4, values[i]);
    }
    set_num_keys(leaf_page, new_num_keys);

    buffer_pool->unpin_page(leaf_pid, true);
}

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {
    auto [leaf_pid, leaf_page, parents] = find_leaf(low);
    (void)parents;

    int current_pid = leaf_pid;
    char *current_page = leaf_page;

    while (true) {
        int num_keys = get_num_keys(current_page);
        for (int i = 0; i < num_keys; i++) {
            int key = read_int(current_page, KEY_START + i * 4);
            if (key >= low && key <= high) {
                results.push_back(
                    read_int(current_page, KEY_START + num_keys * 4 + i * 4));
            }
            if (key > high) {
                buffer_pool->unpin_page(current_pid, false);
                return;
            }
        }

        int next_pid = get_next_leaf(current_page);
        buffer_pool->unpin_page(current_pid, false);

        if (next_pid == -1) {
            return;
        }

        current_pid = next_pid;
        current_page = buffer_pool->fetch_page(current_pid);
    }
}
