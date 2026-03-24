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

## How to Compile and Run

```bash
bash scripts/compile.sh
```

## How to Run Only

```bash
bash scripts/run.sh
```

---

## Test Results

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
```

---

## Benchmark — O(1) vs Naive

Both implementations run the **exact same operations** (same random seed) for a fair comparison.

| Implementation | Data Structure | `get` | `put` |
|---|---|---|---|
| **O(1)** | Doubly Linked List + HashMap | O(1) | O(1) |
| **Naive** | Vector + Loop | O(n) | O(n) |

```
===============================
   O(1) vs NAIVE BENCHMARKS
===============================

--- Small cache, high contention ---
[O(1)]  Small cache, high contention
  Operations : 1000000
  Capacity   : 10
  Key range  : 20
  Time       : 353 ms
  Per op     : 353 ns

[NAIVE] Small cache, high contention
  Operations : 1000000
  Capacity   : 10
  Key range  : 20
  Time       : 350 ms
  Per op     : 350 ns

--- Medium cache, normal use ---
[O(1)]  Medium cache, normal use
  Operations : 1000000
  Capacity   : 1000
  Key range  : 2000
  Time       : 368 ms
  Per op     : 368 ns

[NAIVE] Medium cache, normal use
  Operations : 1000000
  Capacity   : 1000
  Key range  : 2000
  Time       : 11496 ms
  Per op     : 11496 ns

--- Large cache, high eviction ---
[O(1)]  Large cache, high eviction
  Operations : 1000000
  Capacity   : 100
  Key range  : 100000
  Time       : 433 ms
  Per op     : 433 ns

[NAIVE] Large cache, high eviction
  Operations : 1000000
  Capacity   : 100
  Key range  : 100000
  Time       : 963 ms
  Per op     : 963 ns

--- Large cache, low eviction (O(1) only — Naive too slow!) ---
[O(1)]  Large cache, low eviction
  Operations : 1000000
  Capacity   : 100000
  Key range  : 100000
  Time       : 413 ms
  Per op     : 413 ns

===============================
     All tests passed! ✅
===============================
```

---

## Observations

### O(1) Implementation
- Consistent **~350-450 ns per operation** regardless of cache size
- Confirms true **O(1)** behaviour across all scenarios
- High eviction rate has minimal impact on performance

### Naive Implementation
- Similar speed for **small capacity** (capacity=10) — loop is short
- **31x slower** on medium cache (11496 ns vs 368 ns at capacity=1000)
- **2x slower** on high eviction (963 ns vs 433 ns at capacity=100)
- Skipped for large capacity (100000) — would be too slow to run

### Summary Table

| Scenario | Capacity | O(1) per op | Naive per op | Speedup |
|---|---|---|---|---|
| Small, high contention | 10 | 353 ns | 350 ns | ~1x |
| Medium, normal use | 1000 | 368 ns | 11496 ns | **31x** |
| Large, high eviction | 100 | 433 ns | 963 ns | **2x** |
| Large, low eviction | 100000 | 413 ns | N/A (too slow) | — |
