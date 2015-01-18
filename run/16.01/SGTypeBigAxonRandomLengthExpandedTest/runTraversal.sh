#!/bin/bash
rm -f ./SJ.csv

for samplesize in {5000..30000..5000}
do
    for filenum1 in {1..10..1}
    do
        for filenum2 in {1..10..1}
        do
            for algo in {6,7,8}
            do
                echo "../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesAxExpDifLenSmall/$samplesize/sample$filenum1.bin ../../../data/samplesAxExpDifLenSmall/$samplesize/sample$filenum2.bin -y 0 -g 10 -s 0"
              ../../../bin/SpatialJoin -a $algo -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesAxExpDifLenSmall/$samplesize/sample$filenum1.bin ../../../data/samplesAxExpDifLenSmall/$samplesize/sample$filenum2.bin -y 0 -g 10 -s 0
            done
        done
    done
done
