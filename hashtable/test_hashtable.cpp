// Unit tests for the open-addressing hash table in hashtable.cpp.
//
// This file was generated using AI, it helps me to validate my implementation
//
// Build & run (no external dependencies):
//   g++ -std=c++17 -Wall -Wextra -g test_hashtable.cpp -o test_hashtable
//   ./test_hashtable
//
// The template definitions live in hashtable.cpp, so this file includes the
// implementation directly. If you later move the templates into the header,
// you can compile the two files separately instead.
//
// The public API has no lookup method (get/find) yet, so these tests verify
// behaviour observable through size(), capacity(), insert(), erase() and
// hashfunction(). Once you add lookup, add checks for stored values too.

#include "hashtable.cpp"

#include <iostream>
#include <string>

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                                      \
    do {                                                                                 \
        ++g_checks;                                                                      \
        if (!(cond)) {                                                                   \
            ++g_failures;                                                                \
            std::cerr << "    FAIL [line " << __LINE__ << "]: " << #cond << "\n";        \
        }                                                                                \
    } while (0)

static void report(const char *name, int failures_at_start) {
    if (g_failures == failures_at_start) {
        std::cout << "  PASS " << name << "\n";
    } else {
        std::cout << "  FAIL " << name << " (" << (g_failures - failures_at_start)
                  << " check(s) failed)\n";
    }
}

#define RUN_TEST(name)                                                                   \
    do {                                                                                 \
        int start = g_failures;                                                          \
        std::cout << "== " << #name << " ==\n";                                          \
        name();                                                                          \
        report(#name, start);                                                            \
    } while (0)

// A fresh table must report its requested capacity and be empty, and the first
// insert must not crash.
static void test_constructor() {
    HashTable<int, int> t(8);
    CHECK(t.capacity() >= 8);
    CHECK(t.size() == 0);
    CHECK(t.insert(1, 10));
    CHECK(t.size() == 1);
}

// size() must track the number of successfully inserted keys.
static void test_insert_increases_size() {
    HashTable<int, int> t(4);
    for (int i = 0; i < 10; ++i) {
        CHECK(t.insert(i, i * 10));
        CHECK(t.size() == static_cast<size_t>(i + 1));
    }
}

// Crossing the 0.7 load factor must grow the table (capacity increases) while
// keeping every key.
static void test_resize_on_load_factor() {
    HashTable<int, int> t(4);
    size_t initial = t.capacity();
    for (int i = 0; i < 4; ++i) {
        CHECK(t.insert(i, i));
    }
    CHECK(t.size() == 4);
    CHECK(t.capacity() > initial);
}

static void test_erase_existing_key() {
    HashTable<int, int> t(4);
    t.insert(7, 70);
    t.insert(11, 110);
    CHECK(t.erase(7));
    CHECK(t.size() == 1);
    CHECK(!t.erase(7)); // already gone
    CHECK(t.size() == 1);
    CHECK(t.erase(11));
    CHECK(t.size() == 0);
}

static void test_erase_missing_and_empty() {
    HashTable<int, int> t(4);
    CHECK(!t.erase(1)); // empty table
    t.insert(1, 10);
    CHECK(!t.erase(999));
    CHECK(t.size() == 1);
}

// Capacity 8 => hash is key % 8, so 1, 9, 17 and 25 all map to slot 1. All of
// them must be stored despite the collisions, and erased independently.
static void test_colliding_keys() {
    HashTable<int, int> t(8);
    CHECK(t.insert(1, 10));
    CHECK(t.insert(9, 90));
    CHECK(t.insert(17, 170));
    CHECK(t.insert(25, 250));
    CHECK(t.size() == 4);
    CHECK(t.erase(9));
    CHECK(t.size() == 3);
    CHECK(t.erase(25));
    CHECK(t.size() == 2);
}

// hashing must be deterministic and always produce a valid slot index.
static void test_hashfunction_stability() {
    HashTable<int, int> t(16);
    for (int i = 0; i < 100; ++i) {
        CHECK(t.hashfunction(i) == t.hashfunction(i));
        CHECK(t.hashfunction(i) < t.capacity());
    }
}

static void test_string_keys() {
    HashTable<std::string, int> t(8);
    t.insert("apple", 1);
    t.insert("banana", 2);
    t.insert("cherry", 3);
    CHECK(t.size() == 3);
    CHECK(t.erase("banana"));
    CHECK(t.size() == 2);
    CHECK(!t.erase("durian"));
}

// Insert/erase churn across several resizes must not lose or corrupt state.
static void test_many_inserts_and_erases() {
    HashTable<int, int> t(4);
    for (int i = 0; i < 1000; ++i) {
        t.insert(i, i);
    }
    CHECK(t.size() == 1000);
    CHECK(t.capacity() >= t.size());
    for (int i = 0; i < 1000; i += 2) {
        CHECK(t.erase(i));
    }
    CHECK(t.size() == 500);
}

static void test_tiny_table() {
    HashTable<int, int> t(1);
    CHECK(t.capacity() >= 1);
    CHECK(t.insert(1, 10));
    CHECK(t.size() == 1);
    // Second insert crosses the load factor immediately, forcing a resize.
    CHECK(t.insert(2, 20));
    CHECK(t.size() == 2);
    CHECK(t.capacity() > 1);
}

// Decide on your intended duplicate-key semantics. This test assumes the
// second insert of the same key is accepted and the table just grows; adjust
// the expectation (and insert's behaviour) to match your design.
static void test_duplicate_keys() {
    HashTable<int, int> t(8);
    CHECK(t.insert(1, 10));
    CHECK(t.insert(1, 99));
    CHECK(t.size() == 2); // change to 1 if duplicates should be rejected
}

int main() {
    RUN_TEST(test_constructor);
    RUN_TEST(test_insert_increases_size);
    RUN_TEST(test_resize_on_load_factor);
    RUN_TEST(test_erase_existing_key);
    RUN_TEST(test_erase_missing_and_empty);
    RUN_TEST(test_colliding_keys);
    RUN_TEST(test_hashfunction_stability);
    RUN_TEST(test_string_keys);
    RUN_TEST(test_many_inserts_and_erases);
    RUN_TEST(test_tiny_table);
    RUN_TEST(test_duplicate_keys);

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
