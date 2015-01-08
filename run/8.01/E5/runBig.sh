#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..1..1}
    do
        for secondfile in {1..1..1}
        do
            for alg in {1..9..1}
            do
                echo "../../../bin/SpatialJoin -a $alg -l 100 -e 5 -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin"
                ../../../bin/SpatialJoin -a $alg -l 100 -e 5 -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
            done
        done
    done
done