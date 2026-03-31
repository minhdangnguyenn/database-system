echo ""
echo "==============================="
echo "         Running Tests..."
echo "==============================="
echo ""

# Run the executable
./build/lrucache

# Check if tests passed
if [ $? -ne 0 ]; then
    echo "❌ Tests failed!"
    exit 1
fi
