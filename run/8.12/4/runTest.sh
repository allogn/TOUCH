#!/bin/bash
data1="../../../data/RawData-segment-object-1M-Axons.bin"
data2="../../../data/RawData-segment-object-1M-Dendrites.bin"
rm ./SJ.csv

for maxlevel in {1..10000..50}
do
    echo "../../../bin/SpatialJoin -a 7 -p 100 -e 15 -b 3 -t 1 -J 2 -i $data1 $data2 -n 5000 5000 -m $maxlevel"
    ../../../bin/SpatialJoin -a 7 -p 100 -e 15 -b 3 -t 1 -J 2 -i $data1 $data2 -n 5000 5000 -m $maxlevel
done
