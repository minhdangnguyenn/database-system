#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <fstream>
#include <stack>
#include <string>

class DiskManager {
public:
  explicit DiskManager(const std::string &filename);
  ~DiskManager() = default;

  // Core I/O
  int allocate_page();

  void write_page(int page_id, const char *data); // write PAGE_SIZE bytes
  void read_page(int page_id, char *data);

private:
  std::fstream file_; // binary file handle
  std::string filename_;
  int num_pages_ = 0; // total pages allocated so far. need to init 0 to avoid
                      // garbage memory
  std::stack<int> freelist_;
};

#endif // DISK_MANAGER_H
