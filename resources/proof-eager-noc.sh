#!/bin/bash

nv=3
np=3
e1=1
e2=2

while true; do
    all_dots=true

    ./ex3 --rand $nv $np $e1 $e2 --export-gm temp.gm

    for ((i=0; i<nv; i++)); do
        output=$(./ex3 --gm temp.gm --min --cp --proof --start "$i" --filter 3)
        
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