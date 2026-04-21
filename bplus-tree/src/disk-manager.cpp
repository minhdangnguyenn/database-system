#include "../include/disk-manager.h"
#include "../include/test-data.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

DiskManager::DiskManager(const std::string &filename) : filename_(filename) {
    file_.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    // if file does not exist -> create file
    if (!file_.is_open()) {
        file_.open(filename, std::ios::out | std::ios::binary);
        char zeros[PAGE_SIZE] = {};
        file_.write(
            zeros,
            PAGE_SIZE); // write to create page 0 instead of leaving it empty
        file_.close();
        file_.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    file_.clear();
    file_.seekg(0, std::ios::end);
    std::streamoff file_size = file_.tellg();
    this->num_pages_ = static_cast<int>(file_size / PAGE_SIZE);
    file_.seekg(0, std::ios::beg);
}

int DiskManager::allocate_page() {
    int new_page_id = this->num_pages_;
    this->num_pages_ += 1;
    // create a page at empty byte in the file
    // wirte empty byte into the file
    char zero_buffer[PAGE_SIZE] = {};

    this->write_page(new_page_id, zero_buffer);
    // return the new page_id correspond
    return new_page_id;
};

bool DiskManager::is_valid_page_id(int page_id) const {
    return page_id >= 0 && page_id < this->num_pages_;
}

void DiskManager::ensure_page_exists(int page_id) {
    while (page_id >= this->num_pages_) {
        allocate_page();
    }
}

void DiskManager::write_page(int page_id, const char *data) {
    if (page_id >= this->num_pages_ || page_id < 0) {
        throw std::out_of_range("PAGE_ID out of range in write_page !");
    }
    file_.clear();
    file_.seekp(page_id * PAGE_SIZE);
    file_.write(data, PAGE_SIZE);
    file_.flush();
}

void DiskManager::read_page(int page_id, char *data) {
    file_.clear();
    file_.seekg(page_id * PAGE_SIZE);
    file_.read(data, PAGE_SIZE);
}
