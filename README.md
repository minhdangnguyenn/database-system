# LRU Replacement Policy

A C++ implementation of an **LRU (Least Recently Used) cache / buffer pool**, built from scratch using a doubly linked list and hash map for true O(1) operations. The project also includes a disk-backed storage layer and a naive O(n) implementation for benchmarking comparison.

---

## Repository Structure

```
.
├── lru-cache/          # Standalone LRU cache (buffer pool only)
└── database-system/    # Extended version with disk manager + replacer interface
```

Both directories share the same core implementation. `database-system` adds:
- `DiskManager` — file-backed page I/O
- `Replacer` — abstract interface for page replacement policies
- `LRU` — concrete replacer implementation (work in progress)

---

## How It Works

### Core Data Structures

The `BufferPool` uses two doubly linked lists and two hash maps:

```
Main list (LRU order):
head -- [MRU] -- page -- page -- [LRU] -- tail

Unpinned list (eviction candidates only):
uhead -- [MRU unpinned] -- page -- [LRU unpinned] -- utail
```

| Structure | Purpose |
|---|---|
| Main doubly linked list | Tracks recency order of all pages |
| Unpinned doubly linked list | Tracks only evictable (unpinned) pages |
| `unordered_map` | O(1) key → page pointer lookup |
| `unpinned_map` | O(1) key → unpinned page pointer lookup |

### Page Lifecycle

- `pin(key, value)` — insert or update a page, mark it as **pinned** (not evictable), move to MRU
- `get(key)` — read a page, move to MRU position, return `-1` on miss
- `unpin(page*)` — mark a page as evictable, add to unpinned list

When the cache is full and a new page is needed, the **LRU unpinned page** is evicted. Pinned pages are never evicted.

### Complexity

| Operation | Time | Space |
|---|---|---|
| `get(key)` | O(1) | O(1) |
| `pin(key, value)` | O(1) | O(n) |
| `unpin(page*)` | O(1) | O(1) |

---

## Components

### `BufferPool`
The main cache. Manages pages in memory using the dual linked list + hash map structure described above.

### `Page`
A cache entry holding a `key`, `value`, `pinned` flag, and four pointers for its position in both linked lists.

### `LRUCacheNaive`
A reference implementation using a `std::vector` with O(n) `get` and `put`. Used only for benchmarking.

### `DiskManager` *(database-system only)*
Handles file-backed page storage. Supports:
- `allocatePage()` — returns a new page ID (reuses freed pages via a free list stack)
- `deallocatePage(pageId)` — marks a page ID as reusable
- `writePage(pageId, data)` / `readPage(pageId, data)` — fixed-size (4 KB) page I/O

### `Replacer` *(database-system only)*
Abstract interface for page replacement policies. `LRU` is the concrete implementation.

---

## Prerequisites

```bash
cmake       # >= 3.20
g++         # C++20 or higher
```

---

## Build & Run

```bash
# Compile
bash scripts/compile.sh

# Run (after compiling)
bash scripts/run.sh
```

Both scripts are in the `scripts/` directory of each project folder.

---

## Tests

The test suite covers basic correctness, complex scenarios, and stress tests.

```
===============================
         BASIC TESTS
===============================
[PASS] Basic
[PASS] Update Existing
[PASS] Capacity One
[PASS] Get Updates Recency
[PASS] Cache Miss

===============================
        COMPLEX TESTS
===============================
[PASS] Many Evictions
[PASS] Get Prevents Eviction
[PASS] Alternating Operations
[PASS] Repeated Updates
[PASS] No Eviction Needed
[PASS] Large Capacity

===============================
         STRESS TESTS
===============================
[PASS] Stress Evictions
[PASS] Stress Repeated Updates
[PASS] Stress Mixed
[PASS] Stress Large Capacity
```

---

## Benchmarks — O(1) vs Naive

Both implementations run identical operations with the same random seed for a fair comparison.

| Implementation | Data Structure | `get` | `put` |
|---|---|---|---|
| O(1) | Doubly Linked List + HashMap | O(1) | O(1) |
| Naive | Vector + Linear Scan | O(n) | O(n) |

```
===============================
   O(1) vs NAIVE BENCHMARKS
===============================

--- Small cache, high contention ---
[O(1)]   cap=10,      ops=5M,  key_range=20       →  361 ns/op
[NAIVE]  cap=10,      ops=5M,  key_range=20       →  309 ns/op

--- Medium cache, normal use ---
[O(1)]   cap=1000,   ops=5M,  key_range=2000      →  399 ns/op
[NAIVE]  cap=1000,   ops=5M,  key_range=2000      → 5106 ns/op  (~13x slower)

--- Large cache, high eviction ---
[O(1)]   cap=100,    ops=5M,  key_range=100000    →  416 ns/op
[NAIVE]  cap=100,    ops=5M,  key_range=100000    →  846 ns/op  (~2x slower)

--- Large cache, low eviction (O(1) only — Naive too slow) ---
[O(1)]   cap=100000, ops=5M,  key_range=100000    →  537 ns/op
[O(1)]   cap=1000000,ops=5M,  key_range=1000000   →  297 ns/op
```

### Summary

| Scenario | Capacity | O(1) | Naive | Speedup |
|---|---|---|---|---|
| Small, high contention | 10 | 361 ns | 309 ns | ~1x |
| Medium, normal use | 1,000 | 399 ns | 5,106 ns | ~13x |
| Large, high eviction | 100 | 416 ns | 846 ns | ~2x |
| Large, low eviction | 100,000 | 537 ns | N/A | — |
| Massive cache | 1,000,000 | 297 ns | N/A | — |

The O(1) implementation maintains consistent ~300–540 ns/op regardless of cache size. The naive implementation degrades sharply with capacity and was skipped entirely for large/massive sizes.

---

## What's Next

The `database-system/idea.md` outlines the planned evolution into a full mini storage engine:

- Refactor buffer pool into `BufferPoolManager` + `Replacer` + `Page`
- Add disk-backed page persistence
- Implement a B+ Tree index (insert, search, range scan, delete)
- Benchmark against `std::map` and `std::unordered_map` across OLTP and range-scan workloads
