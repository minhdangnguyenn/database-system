#include <iostream>
#include "./include/SimpleKeyVStore.hpp"

int main() {
    {
        SimepleKeyVStore db;
        std::cout << "[Test] Injecting batch data into the store..." << std::endl;

        db.Put("project_name", "WalStorageEngine");
        db.Put("language", "CPP");
        db.Put("status", "Development");
        db.Put("version", "1.0");

        db.Put("status", "Stable");
        db.Delete("temporary_file");

        std::cout << "[Test] Data ingestion complete. Closing store." << std::endl;
    } // The destructor runs here, ensuring logs are flushed

    {
        std::cout << "[Test] Restarting the system to trigger WAL recovery..." << std::endl;
        SimepleKeyVStore db;

        std::string expected_data[][2] = {
            {"project_name", "WalStorageEngine"},
            {"language", "CPP"},
            {"status", "Stable"}, // Check if the update persisted
            {"version", "1.0"}
        };

        bool all_passed = true;
        for (const auto& pair : expected_data) {
            std::string actual = db.Get(pair[0]);
            if (actual == pair[1]) {
                std::cout << "[OK] Key: " << pair[0] << " matches expected value: " << actual << std::endl;
            } else {
                std::cout << "[FAIL] Key: " << pair[0] << " expected " << pair[1] << " but got " << actual << std::endl;
                all_passed = false;
            }
        }

        if (db.Get("temporary_file") == "") {
            std::cout << "[OK] Deleted key 'temporary_file' successfully removed." << std::endl;
        } else {
            std::cout << "[FAIL] Deleted key 'temporary_file' still exists." << std::endl;
            all_passed = false;
        }

        if (all_passed) {
            std::cout << "\n[SUCCESS] WAL recovery validated. System integrity maintained." << std::endl;
        } else {
            std::cout << "\n[FAILURE] System integrity check failed." << std::endl;
        }
    }

    return 0;
}
