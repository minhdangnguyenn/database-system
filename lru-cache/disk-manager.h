#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <fstream>
#include <string>
#include <stack>

class DiskManager {
public:
    explicit DiskManager(const std::string& filename);
    ~DiskManager();

    // Core I/O
    void writePage(int pageId, const char* data);  // write PAGE_SIZE bytes
    void readPage(int pageId, char* data);  // read  PAGE_SIZE bytes

    // Page management
    int  allocatePage();   // returns new pageId, increments counter
    void deallocatePage(int pageId); // marks page as free for reuse

    // Info
    int getNumPages() const;
private:
    std::fstream  file_;       // binary file handle
    std::string   filename_;
    int           numPages_ = 0;   // total pages allocated so far. need to init 0 to avoid garbage memory
    std::stack<int> freeList_;
};

#endif // DISK_MANAGER_H
