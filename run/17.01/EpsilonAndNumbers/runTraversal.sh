#!/bin/bash
rm -f ./SJ.csv

for samplesize in {5000..100000..5000}
do
    for epsilon in {5..55..10}
    do
        echo "../../../bin/SpatialJoin -a 4 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -y 1 -g 100 -s 2  -n $samplesize $samplesize"
        ../../../bin/SpatialJoin -a 4 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -y 1 -g 100 -s 0 -n $samplesize $samplesize
        ../../../bin/SpatialJoin -a 2 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -y 0 -g 100 -s 0 -n $samplesize $samplesize
        for algo in {5,6,7,8,9}
        do
          ../../../bin/SpatialJoin -a $algo -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -y 0 -g 10 -s 2 -n $samplesize $samplesize
        done
    done
done
