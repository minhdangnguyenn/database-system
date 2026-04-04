#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include "frame.h"
#include <fstream>
#include <list>
#include <string>
#include <stack>

class DiskManager {
public:
    explicit DiskManager(const std::string& filename);
    ~DiskManager();

    // Core I/O
    int allocate_page();

    // // Page management
    // int  allocatePage();   // returns new pageId, increments counter
    // void deallocatePage(int pageId); // marks page as free for reuse

    // // Info
    // int getNumPages() const;
private:
    std::fstream  file_;       // binary file handle
    std::string   filename_;
    int           num_pages_ = 0;   // total pages allocated so far. need to init 0 to avoid garbage memory
    std::stack<int> freelist_;
    void write_page(int page_id, const char* data);  // write PAGE_SIZE bytes
    void read_page(int page_id, char* data);  // read  PAGE_SIZE bytes
};

#endif // DISK_MANAGER_H
