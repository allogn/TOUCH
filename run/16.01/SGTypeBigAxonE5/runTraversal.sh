#!/bin/bash
rm -f ./SJ.csv

for samplesize in {10000..200000..20000}
do
    for algo in {6,7,8}
    do
      ../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -y 0 -g 10 -s 1 -n $samplesize $samplesize
    done
done

for samplesize in {10000..200000..20000}
do
    for algo in {6,7,8}
    do
      ../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -y 0 -g 10 -s 2 -n $samplesize $samplesize
    done
done
