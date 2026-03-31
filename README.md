# Database Management System (DBMS) Techniques

> A from-scratch C++ exploration of how real database engines manage
> memory, storage, and indexes under the hood.

This repository implements core DBMS internals one module at a time,
with benchmarks and visual analysis for each technique.

**Modules**

- **LRU Buffer Pool** — O(1) page eviction using a doubly linked list and hash map. Benchmarked against a naïve O(n) implementation.
- **B+ Tree** *(in development)* — disk-backed index with buffer pool integration, supporting insert, search, and range scan.

---

## Repository Structure

```
.
├── lru-cache/                  # LRU buffer pool — standalone, fully tested
│   ├── includes/               # All header files
│   │   ├── buffer-pool.h
│   │   ├── lru-cache-naive.h
│   │   ├── disk-manager.h
│   │   └── test-data.h
│   ├── benchmark/              # CSV results and PNG charts
│   │   ├── benchmark_results.csv
│   │   ├── benchmark_time_ms.png
│   │   ├── benchmark_ns_per_op.png
│   │   ├── benchmark_time_vs_capacity.png
│   │   ├── benchmark_nsop_vs_capacity.png
│   │   └── plot_benchmark.py
│   ├── scripts/
│   │   ├── compile.sh
│   │   └── run.sh
│   └── benchmark.cpp
└── bplus-tree/                 # B+ tree with disk manager and replacer interface
    ├── includes/               (developing)
    └── scripts/
    └── src/
    └── benchmarks/
    └── tests/
```

---

## How It Works — LRU Buffer Pool

### Core Data Structures

The `BufferPool` uses two doubly linked lists and two hash maps to
achieve O(1) for every operation.

```
Main list (recency order — all pages):
head ── [MRU] ── page ── page ── [LRU] ── tail

Unpinned list (eviction candidates only):
uhead ── [MRU unpinned] ── page ── [LRU unpinned] ── utail
```

| Structure | Purpose |
|---|---|
| Main doubly linked list | Tracks recency order of all pages |
| Unpinned doubly linked list | Tracks only evictable (unpinned) pages |
| `unordered_map` | O(1) key → page pointer lookup |
| `unpinned_map` | O(1) key → unpinned pointer lookup |

### Page Lifecycle

- `pin(key, value)` — insert or update a page, mark it **pinned** (not evictable), move to MRU position.
- `get(key)` — read a page, move to MRU position. Returns `-1` on miss.
- `unpin(page*)` — mark a page as evictable, add to unpinned list.

When the cache is full, the **LRU unpinned page** is evicted.
Pinned pages are never evicted.

### Complexity

| Operation | Time | Space |
|---|---|---|
| `get(key)` | O(1) | O(1) |
| `pin(key, value)` | O(1) | O(n) total |
| `unpin(page*)` | O(1) | O(n) total |

---

## Prerequisites

```bash
cmake     # >= 3.20
g++       # C++20 or higher
python3   # for benchmark visualisation (matplotlib, pandas)
```

---

## Build & Run

```bash
# Compile
bash scripts/compile.sh

# Run tests and benchmarks
bash scripts/run.sh

# Visualise benchmark results
python3 benchmark/plot_benchmark.py
```

---

## Tests

The test suite covers basic correctness, complex edge cases, and stress
scenarios with up to 100,000 entries and 5 million operations.

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

## Benchmarks — O(1) vs Naïve

Both implementations run identical workloads with the same random seed
for a fair comparison.

| Implementation | Data Structure | `get` | `put` |
|---|---|---|---|
| Optimised | Doubly Linked List + HashMap | O(1) | O(1) |
| Naïve | Vector + Linear Scan | O(n) | O(n) |

```
--- Small cache, high contention ---
[O(1)]   cap=10        ops=5M  key_range=20        →    361 ns/op
[NAIVE]  cap=10        ops=5M  key_range=20        →    309 ns/op

--- Medium cache, normal use ---
[O(1)]   cap=1,000     ops=5M  key_range=2,000     →    399 ns/op
[NAIVE]  cap=1,000     ops=5M  key_range=2,000     →  5,106 ns/op  (~13x slower)

--- Large cache, high eviction ---
[O(1)]   cap=100       ops=5M  key_range=100,000   →    416 ns/op
[NAIVE]  cap=100       ops=5M  key_range=100,000   →    846 ns/op  (~2x slower)

--- Large / Massive cache (O(1) only — Naïve skipped, too slow) ---
[O(1)]   cap=100,000   ops=5M  key_range=100,000   →    537 ns/op
[O(1)]   cap=1,000,000 ops=5M  key_range=1,000,000 →    297 ns/op
```

### Summary

| Scenario | Capacity | O(1) | Naïve | Speedup |
|---|---|---|---|---|
| Small, high contention | 10 | 361 ns | 309 ns | ~1x |
| Medium, normal use | 1,000 | 399 ns | 5,106 ns | **~13x** |
| Large, high eviction | 100 | 416 ns | 846 ns | ~2x |
| Large, low eviction | 100,000 | 537 ns | N/A | — |
| Massive cache | 1,000,000 | 297 ns | N/A | — |

The O(1) implementation holds a consistent **~300–540 ns/op** regardless
of cache size. The naïve implementation degrades sharply as capacity
grows because every eviction requires a full linear scan.

> **Note — Small cache anomaly:** The naïve implementation is slightly
> faster at capacity=10 because scanning 10 items costs less than
> the hash map overhead in O(1). This is expected and disappears
> completely once capacity grows.

### Latency per operation (ns/op) per workload

![benchmark ns per op](./lru-cache/benchmark/benchmark_ns_per_op.png)

### Time (ms) vs capacity — log scale

![benchmark time vs capacity](./lru-cache/benchmark/benchmark_time_vs_capacity.png)

[→ Full benchmark results and raw CSV](./lru-cache/benchmark/)

---

## Disk Manager

The `DiskManager` provides page-level file I/O on top of a binary file.

- `allocatePage()` — reserves a new page slot and returns its page ID.
- `writePage(id, buf)` — writes a fixed-size page buffer to disk at the correct offset.
- `readPage(id, buf)` — reads a page back from disk into a buffer.

Pages are fixed-size (`PAGE_SIZE` bytes). The disk file grows as pages
are allocated. This layer simulates how a real storage engine separates
logical page IDs from physical file offsets.

---

## What's Next

- Refactor buffer pool into `BufferPoolManager` + `Replacer` + `Page`
- Add disk-backed page persistence via `DiskManager`
- Implement a B+ Tree index — insert, search, range scan, delete
- Benchmark B+ Tree against `std::map` and `std::unordered_map` across OLTP and range-scan workloads

---

## Learning Goals

This project is built to understand how real database engines work
at the component level — not just to implement algorithms, but to
measure, compare, and reason about tradeoffs.

Each module includes:
- A working implementation with tests
- A benchmark comparing design choices
- Visual output to make the numbers concrete
