#!/bin/bash

# Configuration
WORK_DIR="/home/chalo/Software/w-regular-games"
REPO_URL="https://github.com/chuffed/chuffed.git"
REPO_NAME="chuffed"
PATCH_FILE="flatzinc-NOC.patch"
TARGET_DIR="$WORK_DIR/thirdparty"

# 1. Create directory and Clone
# mkdir -p "$TARGET_DIR"
cd "$TARGET_DIR" || exit

if [ ! -d "$REPO_NAME" ]; then
    echo "Cloning $REPO_NAME..."
    git clone "$REPO_URL" "$REPO_NAME"
else
    echo "Repository $REPO_NAME already exists. Skipping clone."
fi

# 3. Apply Patch
# The -N flag ignores patches that are already applied
if [ -f "$PATCH_FILE" ]; then
    echo "Applying patches..."
    patch -p1 -N < "$PATCH_FILE"
else
    echo "No patch file found at $PATCH_FILE, skipping."
fi

# 4. Compile (assuming CMake)
echo "Starting compilation..."
cmake -B build -S .
cmake --build build 

echo "Setup for $REPO_NAME complete."