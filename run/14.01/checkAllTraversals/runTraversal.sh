#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..5..1}
    do
        for secondfile in {1..5..1}
        do
            for algo in {6,7,8}
            do
              ../../../bin/SpatialJoin -a $algo -l 100 -e 50 -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 0 -g 100
              ../../../bin/SpatialJoin -a $algo -l 100 -e 50 -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 1 -g 100
            done
        done
    done
done
