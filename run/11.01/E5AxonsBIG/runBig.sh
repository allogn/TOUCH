#!/bin/bash
rm -f ./SJ.csv

for samplesize in {100000..100000..1}
do
    for alg in {2,4,5,6,7,8,9}
    do
        echo "../../../bin/SpatialJoin -a $alg -l 100 -e 5 -b 3 -g 100 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -n 100000 100000"
        ../../../bin/SpatialJoin -a $alg -l 100 -e 5 -b 3 -g 100 -J 2 -i ../../../data/RawData-segment-object-1M-Axons.bin ../../../data/RawData-segment-object-1M-Dendrites.bin -n 100000 100000
    done
done
