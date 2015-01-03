#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..5..1}
    do
        for secondfile in {1..5..1}
        do
                ../../../bin/SpatialJoin -a 8 -l 100 -e 1 -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 1 -g 0
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..5..1}
    do
        for secondfile in {1..5..1}
        do
                ../../../bin/SpatialJoin -a 8 -l 100 -e 1 -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 0 -g 0
            
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..5..1}
    do
        for secondfile in {1..5..1}
        do
                ../../../bin/SpatialJoin -a 8 -l 100 -e 1 -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 2 -g 0
            
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..5..1}
    do
        for secondfile in {1..5..1}
        do
                ../../../bin/SpatialJoin -a 8 -l 100 -e 1 -b 3 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 3 -g 0
            
        done
    done
done
