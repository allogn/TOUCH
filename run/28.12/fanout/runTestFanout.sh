#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
                for eps in {2..10..2}
                do
                    for alg in {4,6,7,8,9}
                    do
                        ../../../bin/SpatialJoin -a $alg -l 100 -e 5 -b $eps -g 0 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
                    done
                done
        done
    done
done
