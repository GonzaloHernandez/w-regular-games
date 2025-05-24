#!/bin/bash

# Check if a command was provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <command>"
    exit 1
fi

# Run the command in a loop until the output is different from "."
while true; do
    output=$("$@")

    if [[ "$output" != "." ]]; then
        echo "$output"
        break
    fi

    sleep 0.1
done
