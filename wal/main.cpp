#include <iostream>
#include <string>
#include <random> // Essential for random operations
#include <chrono>
#include "./include/SimpleKeyVStore.hpp"

int main() {
    const int NUM_OPERATIONS = 1000000; // 1 000 000

    {
        auto start_time = std::chrono::high_resolution_clock::now();
        SimepleKeyVStore db;
        std::cout << "[Test] Injecting " << NUM_OPERATIONS << " random operations..." << std::endl;

        // Setup random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> op_dist(0, 2);
        std::uniform_int_distribution<> key_dist(1, 1000000); // Using 1-10 000 000 for key collisions

        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            std::string key = "key_" + std::to_string(key_dist(gen));
            int op = op_dist(gen);

            if (op == 0) {
                db.Put(key, "val_" + std::to_string(i));
            } else if (op == 1) { // DELETE
                db.Delete(key);
            } else {
                db.Put(key, "updated_val_" + std::to_string(i));
            }
        }
        auto end_time = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> diff = end_time - start_time;
        std::cout << "[Test] Stress test ingestion complete in " << diff.count() << "seconds" << std::endl;
    }

    {
        std::cout << "[Test] Restarting to verify data integrity after stress test..." << std::endl;
        SimepleKeyVStore db;

        // Perform a quick verification check
        int check_key = 500; // Checking a random key within our range
        std::string key_str = "key_" + std::to_string(check_key);
        std::string val = db.Get(key_str);

        std::cout << "[Check] Value for " << key_str << " is: " << (val.empty() ? "[DELETED/EMPTY]" : val) << std::endl;
        std::cout << "[SUCCESS] System survived " << NUM_OPERATIONS << " operations without crashing." << std::endl;
    }

    return 0;
}
