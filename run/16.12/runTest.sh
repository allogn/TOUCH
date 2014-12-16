#!/bin/bash
rm -f ./SJ.csv

for firstsamplesize in {2000..2000..2000}
do
    for firstfile in {1..10..1}
    do
        for samplesize in {2000..10000..2000}
        do
            for file in {1..10..1}
            do
                for alg in {8,9}
                do
                    ../../bin/SpatialJoin -a $alg -p 100 -e 10 -b 3 -t 1 -J 2 -i ./samples/$samplesize/sample$file.bin ./samples/$firstsamplesize/sample$firstfile.bin
                done
            done
        done
    done
done
