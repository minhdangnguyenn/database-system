#! /bin/bash

echo "======= START BUILDING TEST ======"
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

./tests
