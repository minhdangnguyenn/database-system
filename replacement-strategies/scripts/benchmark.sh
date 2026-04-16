#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "==============================="
echo "         BUILDING              "
echo "==============================="
cmake -S . -B build
cmake --build build

echo ""
echo "==============================="
echo "         RUNNING BENCHMARK     "
echo "==============================="
./build/lrucache

echo ""
echo "==============================="
echo "         VISUALIZING           "
echo "==============================="
if command -v python3 >/dev/null 2>&1; then
  PYTHON_BIN=python3
elif command -v python >/dev/null 2>&1; then
  PYTHON_BIN=python
else
  echo "Python not found in this bash environment; skipping visualization."
  echo "Run this from PowerShell instead:"
  echo "  python benchmark/plot_benchmark.py benchmark_results.csv"
  PYTHON_BIN=""
fi
if [ -n "$PYTHON_BIN" ]; then
  "$PYTHON_BIN" benchmark/plot_benchmark.py benchmark_results.csv
fi

echo ""
echo "Done! Charts saved in the current directory."
