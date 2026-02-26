#!/bin/bash
set -e  # Exit immediately if a command fails

# Configuration
WORK_DIR="/home/chalo/Software/w-regular-games"
TARGET_DIR="$WORK_DIR/thirdparty"
mkdir -p "$TARGET_DIR"

# 1. Chuffed
cd "$TARGET_DIR"
REPO_URL="https://github.com/chuffed/chuffed.git"
REPO_NAME="chuffed"
PATCH_FILE="$WORK_DIR/chuffed-patch/flatzinc-NOC.patch"

# 1.1 Cloning repository
if [ ! -d "$REPO_NAME" ]; then
    echo "Cloning $REPO_NAME..."
    git clone "$REPO_URL" "$REPO_NAME"
else
    echo "Repository $REPO_NAME already exists. Skipping clone."
fi

# 1.2. Enter the repository directory
cd "$REPO_NAME" 

# 1.3. Apply Patch
if [ -f "$PATCH_FILE" ]; then
    echo "Applying patches... to $REPO_NAME"
    patch -p1 -N < "$PATCH_FILE" || echo "Patch already applied or failed."
else
    echo "Warning: No patch file found at $PATCH_FILE, skipping."
fi

# 1.4. Compile
echo "Starting compilation of $REPO_NAME"
cmake -B build -S .
cmake --build build 

echo "Setup for $REPO_NAME complete."

# ----------------------------------------
# 2. Chuffed
cd "$TARGET_DIR"
REPO_URL="https://github.com/arminbiere/cadical.git"
REPO_NAME="cadical"

# 2.1 Cloning repository
if [ ! -d "$REPO_NAME" ]; then
    echo "Cloning $REPO_NAME..."
    git clone "$REPO_URL" "$REPO_NAME"
else
    echo "Repository $REPO_NAME already exists. Skipping clone."
fi

# 2.2. Enter the repository directory
cd "$REPO_NAME" 

# 1.3. Compile
echo "Starting compilation of $REPO_NAME"
./configure
make

echo "Setup for $REPO_NAME complete."
