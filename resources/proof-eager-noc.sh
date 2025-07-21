#!/bin/bash

nv=15
np=10
e1=1
e2=10

while true; do
    all_dots=true

    ./noc --rand $nv $np $e1 $e2 --export-gm temp.gm

    for ((i=0; i<nv; i++)); do
        output=$(./noc --gm temp.gm --min --noc-odd --filter-eager --proof --start "$i")
        
        if [ "$output" = "." ]; then
            printf "."
        else
            all_dots=false
        fi
    done
    
    if [ "$all_dots" = false ]; then
        break
    fi

    printf "\n"
done