#include "./include/SimpleKeyVStore.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

SimepleKeyVStore::SimepleKeyVStore() {
    // create or read log file
    // use append mode to write not delete the log file
    this->log_file = std::ofstream(this->log_file_name, std::ios::app);
    // else read and run the log file
    this->load_wal();
}

void SimepleKeyVStore::load_wal() {
    std::ifstream input_log(this->log_file_name);
    std::string line;
    std::string optype, key, value;

    while (std::getline(input_log, line)) {
        std::stringstream ss(line);
        ss >> optype >> key >> value;

        if (this->get_operation_type(optype) == OperationType::PUT) {
            this->data[key] = value;
        } else if (this->get_operation_type(optype) == OperationType::DELETE) {
            this->data.erase(key);
        }
    }
}

std::string SimepleKeyVStore::Get(const std::string& key) {
    auto it = this->data.find(key);
    if (it != this->data.end()) {
        return it->second;
    }
    return "";
}

void SimepleKeyVStore::Put(const std::string& key, const std::string value) {
    this->data[key] = value;
    this->write_wal(OperationType::PUT, key, value);
}

void SimepleKeyVStore::Delete(const std::string& key) {
    this->data.erase(key);
    this->write_wal(OperationType::DELETE, key, "");
}

void SimepleKeyVStore::write_wal(const OperationType& op, const std::string& key, const std::string& value) {
    std::string op_str = (op == OperationType::PUT) ? "PUT" : "DELETE";
    this->log_file << op_str << " " << key << " " << value << std::endl;
}
