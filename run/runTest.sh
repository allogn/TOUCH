#!/bin/bash
data1="../data/RawData-segment-object-1M-Axons.bin"
data2="../data/RawData-segment-object-1M-Dendrites.bin"
rm ./SJ.csv

for objnum in {500..15000..500}
do
    for eps in {5..15..5}
    do
        for alg in {2,3,4,5,6}
        do
            echo "../bin/SpatialJoin -a $alg -p 100 -e $eps -b 3 -t 1 -J 1 -i $data1 $data2 -n $objnum $objnum"
            ../bin/SpatialJoin -a $alg -p 100 -e $eps -b 3 -t 1 -J 1 -i $data1 $data2 -n $objnum $objnum
        done
    done
done
