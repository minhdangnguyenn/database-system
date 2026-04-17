#include "replacer.h"
#include <list>
#include <unordered_map>
#include <unordered_set>

class LFUReplacer : public Replacer {
  public:
    LFUReplacer();
    void pin(Page *page) override;
    void unpin(Page *page) override;
    Page *evict() override;

  private:
    // key -> frequency
    std::unordered_map<int, int> freq_map;

    // key -> page pointer
    std::unordered_map<int, Page *> page_map;

    // frequency -> list of keys (front = most recent at that freq)
    std::unordered_map<int, std::list<int>> freq_list;

    // key -> iterator in its freq_list bucket (for O(1) removal)
    std::unordered_map<int, std::list<int>::iterator> iter_map;

    int min_freq;

    void increment_freq(int key);
};
