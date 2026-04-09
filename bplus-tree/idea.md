## ✅ Completed So Far

- [x] DiskManager — read, write, allocate, deallocate pages
- [x] Page — page_id, pin_count, is_dirty, data buffer
- [x] LRU Replacer — evict, pin, unpin, size

---

## 🔜 Step 1 — BufferPoolManager

This is the core component that connects everything together.

### What it owns internally

| Field | Type | Purpose |
|-------|------|---------|
| `frames` | `vector<Page>` | Fixed-size memory pool |
| `page_table` | `unordered_map<int, int>` | Maps page_id → frame_id |
| `free_list` | `list<int>` | Available empty frames |
| `replacer` | `LRU*` | Eviction policy |
| `disk` | `DiskManager*` | Disk I/O |

### Functions to implement

#### `NewPage(page_id)`
```
get free frame (free_list or evict)
if no frame available → return nullptr
allocate page_id from disk
reset frame data
update page_table
pin the frame
call replacer.pin(frame_id)
return page
```

#### `FetchPage(page_id)`
```
check page_table (cache hit?)
if hit:
    increment pin_count
    call replacer.pin(frame_id)
    return page

if miss:
    get free frame (free_list or evict)
    if no frame → return nullptr
    read page from disk
    update page_table
    pin the frame
    call replacer.pin(frame_id)
    return page
```

#### `UnpinPage(page_id, is_dirty)`
```
if page_id not in page_table → return
decrement pin_count
if is_dirty → mark page dirty
if pin_count == 0:
    call replacer.unpin(frame_id)
```

#### `FlushPage(page_id)`
```
if page_id not in page_table → return
write page data to disk
clear dirty flag
```

#### `get_frame()` (internal helper)
```
if free_list not empty:
    return free_list.pop()

if replacer.evict(frame_id):
    if frame is dirty:
        disk.writePage(page_id, data)
    page_table.erase(old_page_id)
    return frame_id

return -1  // all frames pinned
```

### Milestones

- [ ] Define class header with all fields
- [ ] Implement constructor (init frames, free_list, replacer)
- [ ] Implement `get_frame()` helper
- [ ] Implement `NewPage()`
- [ ] Implement `FetchPage()`
- [ ] Implement `UnpinPage()`
- [ ] Implement `FlushPage()`
- [ ] Write unit tests for each function

---

## 🔜 Step 2 — B+ Tree Leaf Nodes

After BufferPoolManager is tested and working.

### What to implement

- `LeafPage` class with:
  - sorted `(key, value)` pairs
  - `next_leaf` pointer (page_id of next leaf)
  - `insert(key, value)`
  - `search(key)`
  - `is_full()` check
  - `split()` → returns new sibling page

### Milestones

- [ ] Define `BPlusTreePage` base class
- [ ] Define `LeafPage` extending base
- [ ] Implement `insert` into leaf
- [ ] Implement `search` in leaf
- [ ] Implement leaf `split`
- [ ] Test leaf insert and search with BufferPoolManager

---

## 🔜 Step 3 — B+ Tree Internal Nodes

### What to implement

- `InternalPage` class with:
  - `(key, child_page_id)` routing entries
  - `insert_after_split(key, left_page_id, right_page_id)`
  - `find_child(key)` → returns child page_id
  - `is_full()` check
  - `split()` → push middle key up

### Milestones

- [ ] Define `InternalPage` extending base
- [ ] Implement `find_child(key)`
- [ ] Implement `insert_after_split()`
- [ ] Implement internal `split`
- [ ] Test root splits and tree growth

---

## 🔜 Step 4 — B+ Tree Public API

### Functions to implement

```
insert(key, value)
search(key) → value
remove(key)        ← optional but recommended
scan(start, end)   → list of values
```

### Milestones

- [ ] Implement `insert` with recursive split propagation
- [ ] Implement `search`
- [ ] Implement `scan` (range query)
- [ ] Handle root changes (root split)
- [ ] Test with 1000+ random keys

---

## 🔜 Step 5 — Range Scan Iterator

### What to implement

- `IndexIterator` class
  - traverses leaf pages in order
  - uses `next_leaf` pointer to move between pages
  - supports `begin()`, `end()`, `++`, `*`

### Milestones

- [ ] Define `IndexIterator`
- [ ] Implement `operator++`
- [ ] Implement `operator*` to get `(key, value)`
- [ ] Test range scan across multiple leaf pages

---

## 🔜 Step 6 — Benchmark Harness

### Baselines to compare

| Baseline | Type |
|----------|------|
| `std::map` | In-memory ordered tree |
| `std::unordered_map` | In-memory hash map |
| Your B+ Tree | Disk-backed with buffer pool |

### Workloads to test

| Workload | Description |
|----------|-------------|
| Read-heavy | 95% search, 5% insert |
| Write-heavy | 50% search, 50% insert |
| Range scan | Scan over key ranges |
| Cold cache | First run — empty buffer pool |
| Warm cache | Repeated run — pages cached |

### Metrics to collect

| Metric | How |
|--------|-----|
| Throughput (ops/sec) | Count ops / elapsed time |
| Latency p50/p95/p99 | Record per-op duration |
| Buffer pool hit rate | hits / (hits + misses) |
| Disk I/O count | DiskManager read/write counters |
| Tree height | Count levels during traversal |

### Milestones

- [ ] Add read/write counters to DiskManager
- [ ] Add hit/miss counters to BufferPoolManager
- [ ] Write benchmark runner
- [ ] Run all workloads
- [ ] Plot results

---

## 🔜 Step 7 — Optional Stretch Goals

- [ ] Clock Replacer (compare with LRU)
- [ ] Concurrency (page-level latches)
- [ ] Write-Ahead Logging (WAL) for crash recovery
- [ ] Compression for leaf pages
- [ ] Variable-length keys
- [ ] Bulk-loading for fast initial inserts

---

## 📁 Recommended Project Structure

```
database-system/
  bplus-tree/
    include/
      disk_manager.h
      page.h
      replacer.h
      lru.h
      buffer_pool_manager.h
      bplus_tree.h
      leaf_page.h
      internal_page.h
      index_iterator.h
    src/
      disk_manager.cpp
      lru.cpp
      buffer_pool_manager.cpp
      bplus_tree.cpp
      leaf_page.cpp
      internal_page.cpp
    benchmark/
      benchmark.cpp
    tests/
      test_lru.cpp
      test_buffer_pool.cpp
      test_bplus_tree.cpp
    scripts/
      compile.sh
      run.sh
    CMakeLists.txt
```

---

## 🎯 Priority Order

```
1. BufferPoolManager     ← start here
2. LeafPage + insert/search
3. InternalPage + splits
4. Full B+ Tree API
5. Range scan iterator
6. Benchmark harness
7. Stretch goals
```
