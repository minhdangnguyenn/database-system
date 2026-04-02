#! /bin/bash

echo ""
echo "==============================="
echo "         Running Tests..."
echo "==============================="
echo ""

# Run the executable
./build/tests
./build/dbms # temp comment, still implementing
# Check if tests passed
if [ $? -ne 0 ]; then
    echo "❌ Tests failed!"
    exit 1
fi
