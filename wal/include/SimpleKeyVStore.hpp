#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#ifndef SIMPLE_KEY_VSTORE_HPP
#define SIMPLE_KEY_VSTORE_HPP

enum OperationType {
    GET,
    PUT,
    DELETE
};

class SimepleKeyVStore {
    private:
        std::string log_file_name = "log.txt";
        std::unordered_map<std::string, std::string> data;
        std::ofstream log_file;
        OperationType get_operation_type(const std::string& op_str) {
            if (op_str == "GET") return OperationType::GET;
            if (op_str == "PUT") return OperationType::PUT;
            if (op_str == "DELETE") return OperationType::DELETE;
            throw std::invalid_argument("Invalid operation type");
        }

    public:
        SimepleKeyVStore();
        void Put(const std::string& key, const std::string value);
        std::string Get(const std::string& key);
        void Delete(const std::string& key);
        void write_wal(const OperationType&, const std::string&, const std::string&);
        void load_wal();


};

#endif
