#!/bin/bash

# List of required files
required_files=("assign.c" "output.txt" "alias_lib.cpp" "CMakeLists.txt")

# Check if all required files exist
missing_files=()
for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        missing_files+=("$file")
    fi
done

# Report missing files
if [ ${#missing_files[@]} -ne 0 ]; then
    echo "❌ Missing files: ${missing_files[*]}"
    exit 1
fi

echo "✅ All required files are present."

# Extract pass registration name from alias_lib.cpp
cpp_pass_name=$(grep -oP 'RegisterPass<\w+>\s+\w+\("\K[^"]+' alias_lib.cpp)

# Extract library name from CMakeLists.txt
cmake_library_name=$(grep -oP 'add_llvm_library\(\K\w+' CMakeLists.txt | head -1)

# Expected names
expected_cpp_name="alias_lib_given"
expected_cmake_name="aliasCustom"

# Check if alias_lib.cpp has the correct pass name
if [ "$cpp_pass_name" == "$expected_cpp_name" ]; then
    echo "✅ Correct pass registration name in alias_lib.cpp: $cpp_pass_name"
else
    echo "❌ Incorrect pass registration name in alias_lib.cpp. Expected '$expected_cpp_name', found '$cpp_pass_name'"
    exit 1
fi

# Check if CMakeLists.txt has the correct library name
if [ "$cmake_library_name" == "$expected_cmake_name" ]; then
    echo "✅ Correct library name in CMakeLists.txt: $cmake_library_name"
else
    echo "❌ Incorrect library name in CMakeLists.txt. Expected '$expected_cmake_name', found '$cmake_library_name'"
    exit 1
fi

# Check if private_test/test.c exists
test_file="private_test/test.c"
if [ ! -f "$test_file" ]; then
    echo "❌ File '$test_file' does not exist!"
    exit 1
else
    echo "✅ File '$test_file' found."
fi


