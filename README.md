# LRU Cache — C++ Implementation

A **Least Recently Used (LRU) Cache** implemented in C++ using a **Doubly Linked List** + **Hash Map** for O(1) `get` and `put` operations.

---

## How It Works

### Data Structures
- **Doubly Linked List** — tracks order of recency
- **`unordered_map`** — maps key to node pointer for O(1) lookup

### Visualize

```
head -- [MRU] -- node -- node -- [LRU] -- tail
  ↑                                         ↑
always here                           always here
```

- Every `get` or `put` → node moves to **front** (MRU)
- When cache is full → node at **back** is evicted (LRU)

### Complexity

| Operation | Time | Space |
|---|---|---|
| `get(key)` | O(1) | O(1) |
| `put(key, value)` | O(1) | O(n) |

---

## Prerequisites

```bash
cmake
g++ (C++20 or higher)
```

---

## How to Run

```bash
bash run.sh
```

---

## Benchmark Results

Machine: `Ubuntu WSL2`

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
          BENCHMARKS
===============================
[BENCH] Small cache, high contention
  Operations : 1000000
  Capacity   : 10
  Key range  : 20
  Time       : 358 ms
  Per op     : 358 ns

[BENCH] Medium cache, normal use
  Operations : 1000000
  Capacity   : 1000
  Key range  : 2000
  Time       : 363 ms
  Per op     : 363 ns

[BENCH] Large cache, low eviction
  Operations : 1000000
  Capacity   : 100000
  Key range  : 100000
  Time       : 451 ms
  Per op     : 451 ns

[BENCH] Large cache, high eviction
  Operations : 1000000
  Capacity   : 100
  Key range  : 100000
  Time       : 414 ms
  Per op     : 414 ns

===============================
     All tests passed! ✅
===============================
```

### Observations
- ~350-450 ns per operation regardless of cache size confirms **O(1)** behaviour
- High eviction rate has minimal impact on performance
- Consistent results across small and large capacities
