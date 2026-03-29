# Mini Storage Engine — Disk-Backed B+ Tree Index with Buffer Pool

> Build a disk-backed B+ Tree Index with Buffer Pool, then benchmark it against in-memory baselines.

> Pseudocode for buffer manager connect with disk manager
```bash
BufferPool(capacity, disk_manager):
    capacity = capacity
    disk = disk_manager
    page_table = {}      // page_id → Page*
    free_frames = []     // list of empty frame slots
    lru_list = []        // ordered by recency

Page:
    page_id  = -1
    is_dirty = false
    pin_count = 0
    data[PAGE_SIZE] = empty

// ─────────────────────────────────────────
newPage():
    frame = get_frame()
    if frame == null → return null   // all pinned

    page_id = disk.allocatePage()
    frame.page_id  = page_id
    frame.is_dirty = false
    frame.pin_count = 1
    frame.data = zeros

    page_table[page_id] = frame
    push_to_lru_front(frame)
    return frame

// ─────────────────────────────────────────
fetchPage(page_id):
    // cache hit
    if page_id in page_table:
        page = page_table[page_id]
        page.pin_count++
        move_to_front(page)
        return page

    // cache miss
    frame = get_frame()
    if frame == null → return null   // all pinned

    disk.readPage(page_id, frame.data)
    frame.page_id   = page_id
    frame.is_dirty  = false
    frame.pin_count = 1

    page_table[page_id] = frame
    push_to_lru_front(frame)
    return frame

// ─────────────────────────────────────────
unpinPage(page_id, is_dirty):
    if page_id not in page_table → return
    page = page_table[page_id]
    if page.pin_count == 0 → return

    page.pin_count--
    if is_dirty:
        page.is_dirty = true

// ─────────────────────────────────────────
flushPage(page_id):
    if page_id not in page_table → return
    page = page_table[page_id]

    disk.writePage(page_id, page.data)
    page.is_dirty = false

// ─────────────────────────────────────────
get_frame():
    // use free frame if available
    if free_frames not empty:
        return free_frames.pop()

    // evict LRU unpinned page
    victim = lru_tail
    while victim != null:
        if victim.pin_count == 0:
            break
        victim = victim.prev

    if victim == null → return null  // all pinned

    if victim.is_dirty:
        disk.writePage(victim.page_id, victim.data)

    page_table.erase(victim.page_id)
    remove_from_lru(victim)
    victim.reset()
    return victim
```

---

## Core Classes

### `DiskManager`
- Allocate page IDs
- Read/write fixed-size pages to a file
- Track basic I/O stats (read count, write count)

### `Page`
- `page_id`, `pin_count`, `is_dirty`
- Raw byte buffer (fixed page size, e.g. 4 KB)
- Latch/mutex if you later add concurrency

### `BufferPoolManager`
- Fetch / unpin / flush / new page
- Integrates replacer + disk manager
- Maintains page table: `page_id → frame_id`

### `Replacer` *(interface)*
- `LRUReplacer` (you already have most logic)
- Optional: `ClockReplacer` for comparison

### `BPlusTree`
- Public APIs: `insert`, `get`, `remove`, `scan(start, end)`
- Handles root changes, split / merge / redistribution

### `BPlusTreeNode` hierarchy

| Class | Responsibility |
|---|---|
| `BPlusTreePage` | Base metadata: `is_leaf`, `size`, `max_size`, `parent_id` |
| `LeafPage` | Sorted `(key, value)` pairs + `next_leaf` pointer |
| `InternalPage` | `(key, child_page_id)` routing entries |

### `IndexIterator`
- In-order leaf traversal for range scans

### `Serializer / Comparator`
- Key encoding and key compare abstraction
- Start with `int64_t` keys

---

## Implementation Milestones

1. Refactor current buffer pool into `BufferPoolManager` + `Replacer` + `Page`
2. Add disk-backed pages and persistence tests
3. Implement leaf-only insert / search
4. Add internal nodes and recursive split propagation
5. Add range scan iterator
6. Add delete *(optional but great for resume)*
7. Add benchmark harness and result plots / tables

---

## Benchmark Plan

### Baselines
- `std::map`
- `std::unordered_map`
- Optional: in-memory B+ tree mode *(no disk / buffer pool)*

### Workloads

| Workload | Description |
|---|---|
| Read-heavy | 95% get, 5% insert |
| Write-heavy | 50% get, 50% insert |
| Range scan | Scan over key ranges |
| Uniform keys | Evenly distributed keys |
| Zipfian keys | Hotspot / skewed access pattern |
| Cold vs warm cache | First run vs repeated run |

### Metrics

| Category | Metrics |
|---|---|
| **Performance** | Throughput (ops/sec), Latency (p50 / p95 / p99) |
| **Buffer Pool** | Hit rate, disk I/O count |
| **Tree** | Height, split / merge counts |

### Fairness Rules
- Same dataset size, key distribution, and random seed across all runs
- Report exact hardware, compiler flags, page size, and buffer pool size

---

## Resume Positioning

> *"Implemented a disk-backed B+ tree index in C++ with buffer pool, LRU replacement, page-level persistence, and benchmark suite showing performance across mixed OLTP and range-scan workloads."*
