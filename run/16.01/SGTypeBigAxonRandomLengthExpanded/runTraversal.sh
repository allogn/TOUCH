#!/bin/bash
rm -f ./SJ.csv

for samplesize in {10000..100000..10000}
do
    for filenum1 in {1..10..1}
    do
        for filenum2 in {1..10..1}
        do
            for algo in {6,7,8}
            do
              ../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesAxExpDifLen/$samplesize/sample$filenum1.bin ../../../data/samplesAxExpDifLen/$samplesize/sample$filenum2.bin -y 0 -g 10 -s 0
            done
        done
    done
done

for samplesize in {10000..100000..10000}
do
    for filenum1 in {1..10..1}
    do
        for filenum2 in {1..10..1}
        do
            for algo in {6,7,8}
            do
              ../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesAxExpDifLen/$samplesize/sample$filenum1.bin ../../../data/samplesAxExpDifLen/$samplesize/sample$filenum2.bin -y 0 -g 10 -s 1
            done
        done
    done
done

for samplesize in {10000..100000..10000}
do
    for filenum1 in {1..10..1}
    do
        for filenum2 in {1..10..1}
        do
            for algo in {6,7,8}
            do
              ../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesAxExpDifLen/$samplesize/sample$filenum1.bin ../../../data/samplesAxExpDifLen/$samplesize/sample$filenum2.bin -y 0 -g 10 -s 2
            done
        done
    done
done
