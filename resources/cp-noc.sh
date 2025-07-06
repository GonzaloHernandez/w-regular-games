#!/bin/bash

if [ $# -eq 0 ]; then
    ./ex3 --help
    exit 1
fi

base_cmd=("$@")

"${base_cmd[@]}" --noc-even &
PID1=$!

"${base_cmd[@]}" --noc-odd &
PID2=$!

wait -n

kill $PID1 2>/dev/null
kill $PID2 2>/dev/null