#!/bin/bash

echo "==============================="
echo "         BUILDING              "
echo "==============================="
cmake -B build && cmake --build build

echo ""
echo "==============================="
echo "         RUNNING BENCHMARK     "
echo "==============================="
./build/lrucache

echo ""
echo "==============================="
echo "         VISUALIZING           "
echo "==============================="
python3 benchmark/plot_benchmark.py ./build/benchmark_results.csv

echo ""
echo "Done! Charts saved in the current directory."
