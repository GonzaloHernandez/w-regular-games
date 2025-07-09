#!/bin/bash

if [ $# -eq 0 ]; then
    ./ex3 --help
    exit 1
fi

user_args=("$@")

./ex3 --noc-even "${user_args[@]}" &
PID1=$!

./ex3 --noc-odd "${user_args[@]}" &
PID2=$!

wait -n

kill $PID1 2>/dev/null
kill $PID2 2>/dev/null