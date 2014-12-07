#!/bin/bash
data1="../../../data/RawData-segment-object-1M-Axons.bin"
data2="../../../data/RawData-segment-object-1M-Dendrites.bin"
rm ./SJ.csv

for objnum in {2..30..1}
do
    for alg in {4,6,7,8,9}
    do
        echo "../../../bin/SpatialJoin -a $alg -p 100 -e 5 -b $objnum -t 1 -J 0 -i $data1 $data2 -n 4000 4000"
        ../../../bin/SpatialJoin -a $alg -p 100 -e 5 -b $objnum -t 1 -J 0 -i $data1 $data2 -n 4000 4000
    done
done
