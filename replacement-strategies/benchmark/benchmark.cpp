#include "../include/buffer-pool.h"
#include "../include/fifo-replacer.h"
#include "../include/lru-cache-naive.h"
#include "../include/lru-replacer.h"

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

struct Operation {
    bool is_get = false;
    int key = 0;
    int value = 0;
};

struct Workload {
    std::string name;
    int operations = 0;
    int key_range = 0;
};

struct Strategy {
    std::string type;
    bool touch_on_get = false;
    std::function<std::unique_ptr<Replacer>()> make_replacer;
};

void init_csv(std::ofstream &csv, const std::string &filename) {
    csv.open(filename);
    csv << "name,type,capacity,operations,key_range,run,time_ms,ns_per_op\n";
}

void write_csv_row(std::ofstream &csv, const std::string &name,
                   const std::string &type, int capacity, int operations,
                   int key_range, int run, long long ms, long long ns_per_op) {
    csv << name << "," << type << "," << capacity << "," << operations << ","
        << key_range << "," << run << "," << ms << "," << ns_per_op << "\n";
}

std::vector<Operation> build_workload(int operations, int key_range, int seed) {
    std::vector<Operation> trace;
    trace.reserve(operations);

    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> key_dist(0, key_range - 1);
    std::uniform_int_distribution<int> op_dist(0, 1);
    std::uniform_int_distribution<int> value_dist(0, 9999);

    for (int i = 0; i < operations; i++) {
        Operation op;
        op.is_get = (op_dist(gen) == 0);
        op.key = key_dist(gen);
        op.value = value_dist(gen);
        trace.push_back(op);
    }

    return trace;
}

void unpin_if_needed(BufferPool &cache, int key) {
    auto it = cache.map.find(key);
    if (it != cache.map.end() && it->second->pinned) {
        cache.unpin(it->second);
    }
}

void run_once(const Workload &workload, int capacity, int run,
              const Strategy &strategy, const std::vector<Operation> &trace,
              std::ofstream &csv) {
    auto replacer = strategy.make_replacer();
    BufferPool cache(capacity, replacer.get());

    auto start = std::chrono::high_resolution_clock::now();

    for (const auto &op : trace) {
        if (op.is_get) {
            int value = cache.get(op.key);
            if (value != -1 && strategy.touch_on_get) {
                // Mark read-hit as recent for LRU-based strategies.
                cache.pin(op.key, value);
                unpin_if_needed(cache, op.key);
            }
            continue;
        }

        cache.pin(op.key, op.value);
        unpin_if_needed(cache, op.key);
    }

    auto end = std::chrono::high_resolution_clock::now();

    const long long elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    const long long elapsed_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count();
    const long long ns_per_op =
        workload.operations > 0 ? elapsed_ns / workload.operations : 0;

    write_csv_row(csv, workload.name, strategy.type, capacity,
                  workload.operations, workload.key_range, run, elapsed_ms,
                  ns_per_op);
}

void run_workload(const Workload &workload, const std::vector<int> &capacities,
                  int runs, int naive_max_capacity,
                  const std::vector<Strategy> &strategies, std::ofstream &csv) {
    std::cout << "\n=== " << workload.name << " ===\n";

    for (int capacity : capacities) {
        std::cout << "capacity = " << capacity << std::endl;

        for (int run = 0; run < runs; run++) {
            const auto trace =
                build_workload(workload.operations, workload.key_range, 42 + run);

            for (const auto &strategy : strategies) {
                if (strategy.type == "NAIVE" && capacity > naive_max_capacity) {
                    std::cout << "  skip NAIVE at cap=" << capacity
                              << " (too expensive)\n";
                    continue;
                }

                run_once(workload, capacity, run, strategy, trace, csv);
            }
        }
    }
}

int main() {
    std::ofstream csv;
    init_csv(csv, "benchmark_results.csv");

    const int runs = 2;
    const int naive_max_capacity = 5000;
    const std::vector<int> capacities = {10, 100, 1000, 5000, 100000};
    const std::vector<Workload> workloads = {
        {"High contention", 120000, 20},
        {"Balanced workload", 90000, 2000},
        {"Low contention", 70000, 100000},
    };
    const std::vector<Strategy> strategies = {
        {"NAIVE", true,
         []() { return std::make_unique<LRUReplacerNaive>(); }},
        {"LRU", true, []() { return std::make_unique<LRUReplacer>(); }},
        {"FIFO", false, []() { return std::make_unique<FIFOReplacer>(); }},
    };

    for (const auto &workload : workloads) {
        run_workload(workload, capacities, runs, naive_max_capacity, strategies,
                     csv);
    }

    csv.close();
    std::cout << "\nDone -> benchmark_results.csv\n";
    return 0;
}
