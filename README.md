# Database Management System (DBMS) Techniques

> Implement fundamental techniques of DBMS
> memory, storage, and indexes under the hood.

This repository implements core DBMS internals one module at a time,
with benchmarks and visual analysis for each technique.

**Modules**

- **Replacement Strategy Buffer Pool** — O(1) page eviction using a doubly linked list and hash map. Benchmarked against a naïve O(n) implementation.
- **B+ Tree** _(in development)_ — disk-backed index with buffer pool integration, supporting insert, search, and range scan.

---

## Learning Goals

This project is built to understand how real database engines work
at the component level — not just to implement algorithms, but to
measure, compare, and reason about tradeoffs.

Each module includes:

- A working implementation with tests
- A benchmark comparing design choices
- Visual output to make the numbers concrete
