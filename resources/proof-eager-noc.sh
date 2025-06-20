#!/bin/bash

nv=5
np=5
e1=1
e2=3

while true; do
    all_dots=true

    ./ex3 --rand $nv $np $e1 $e2 --export-gm temp.gm

    for ((i=0; i<nv; i++)); do
        output=$(./ex3 --gm temp.gm --min --cp --proof --start "$i" --filter-reload)
        
        if [ "$output" = "." ]; then
            echo -n "."
        else
            all_dots=false
        fi
    done
    
    if [ "$all_dots" = false ]; then
        break
    fi
done