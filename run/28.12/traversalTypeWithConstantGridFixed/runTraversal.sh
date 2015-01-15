#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..5..1}
    do
        for secondfile in {1..5..1}
        do
                echo "../../../bin/SpatialJoin1 -a $alg -l 100 -e 5 -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin"
                ../../../bin/SpatialJoin -a 8 -l 100 -e 10 -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 2
                ../../../bin/SpatialJoin -a 8 -l 100 -e 10 -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 1
                ../../../bin/SpatialJoin -a 8 -l 100 -e 10 -b 3 -g 100 -J 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin -y 0
        done
    done
done
