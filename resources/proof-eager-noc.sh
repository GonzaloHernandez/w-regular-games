#!/bin/bash

nv=3
np=3
e1=1
e2=2

./ex3 --rand $nv $np $e1 $e2 --export-gm temp.gm

for ((i=0; i<nv; i++)); do
    echo -n "$i "
    ./ex3 --gm temp.gm --min --cp --proof --start "$i"
done