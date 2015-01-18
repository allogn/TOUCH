#!/bin/bash
rm -f ./SJ.csv

epsilon=10 

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            echo "../../../bin/SpatialJoin -a 4 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 1 -g 100 -s 2"
            ../../../bin/SpatialJoin -a 4 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 1 -g 100 -s 0
            ../../../bin/SpatialJoin -a 2 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 0 -g 100 -s 0
            for algo in {5,6,7,8,9}
            do
              ../../../bin/SpatialJoin -a $algo -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 0 -g 10 -s 2
            done
        done
    done
done
