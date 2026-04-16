#!/usr/bin/env bash
set -euo pipefail

echo "==============================="
echo "     Building LRU Cache..."
echo "==============================="

echo "Creating build folder..."
rm -rf build
mkdir -p build
cd build
cmake ..
make

echo ""
echo "==============================="
echo "         Running Tests..."
echo "==============================="
echo ""

./lrucache
