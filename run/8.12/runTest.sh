#!/bin/bash
data1="../../data/RawData-segment-object-1M-Axons.bin"
data2="../../data/RawData-segment-object-1M-Dendrites.bin"
rm ./SJ.csv

for objnum in {500..15000..1000}
do
    for alg in {4,6,7,8,9}
    do
        echo "../../bin/SpatialJoin -a $alg -p 100 -e 25 -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum"
        ../../bin/SpatialJoin -a $alg -p 100 -e 25 -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum
    done
done
