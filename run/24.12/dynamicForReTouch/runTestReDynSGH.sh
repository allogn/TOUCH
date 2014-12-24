#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            for eps in {1..15..2}
            do
                ../../../bin/SpatialJoin -a 8 -l 100 -e $eps -b 3 -g 0 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
            done
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            for eps in {1..15..2}
            do
                ../../../bin/SpatialJoin -a 8 -l 100 -e $eps -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
            done
        done
    done
done
