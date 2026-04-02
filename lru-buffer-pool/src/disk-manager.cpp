#include "disk-manager.h"
#include <iostream>
#include <fstream>
#include "test-data.h"

DiskManager::DiskManager(const std::string& filename) : filename_(filename) {
    file_.open(filename, std::ios::in|std::ios::out|std::ios::binary);
    // if file does not exist -> create file
    if (!file_.is_open()) {
        file_.open(filename, std::ios::out | std::ios::binary);
        char zeros[PAGE_SIZE] ={};
        file_.write(zeros, PAGE_SIZE); //write to create page 0 instead of leaving it empty
        file_.close();
        file_.open(filename, std::ios::in|std::ios::out|std::ios::binary);
    }
}

void DiskManager::writePage(int pageId, const char* data) {
    file_.clear();
    file_.seekp(pageId * PAGE_SIZE);
    file_.write(data, PAGE_SIZE);
    file_.flush();
}

void DiskManager::readPage(int pageId, char* data) {
    file_.clear();
    file_.seekg(pageId * PAGE_SIZE);
    file_.read(data, PAGE_SIZE);
}

int DiskManager::allocatePage() {
    // need to focus on free list
    // get the free page id
    if (!freeList_.empty()) {
        int id = freeList_.top(); //LIFO stack -- pop the last element
        freeList_.pop();
        return id;
    }
    return numPages_++;
}

void DiskManager::deallocatePage(int pageId) {
    freeList_.push(pageId);
}

int DiskManager::getNumPages() const {
    return numPages_;
}

DiskManager::~DiskManager() {
    if (file_.is_open()) {
        file_.close();
    }
}
