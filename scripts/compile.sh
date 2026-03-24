#!/bin/bash

echo "==============================="
echo "     Building LRU Cache..."
echo "==============================="

# Create build folder if it doesn't exist
echo "Creating build folder..."
rm -rf build/
mkdir build
cd build
cmake ..


# Build
make

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo ""
echo "==============================="
echo "         Running Tests..."
echo "==============================="
echo ""

# Run the executable
./LRUCache

# Check if tests passed
if [ $? -ne 0 ]; then
    echo "❌ Tests failed!"
    exit 1
fi
