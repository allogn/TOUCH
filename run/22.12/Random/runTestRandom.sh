#!/bin/bash
rm -f ./SJ.csv

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            if [ "$secondfile" -ge "$firstfile" ]; then
                echo hello
            else
                for eps in {1..30..3}
                do
                    for alg in {4,6,7,8,9}
                    do
                        ../../../bin/SpatialJoin -a $alg -l 100 -e $eps -b 3 -t 1 -J 2 -i ../../../data/samplesRandomUni/$samplesize/sample$firstfile.bin ../../../data/samplesRandomUni/$samplesize/sample$secondfile.bin
                    done
                done
            fi
        done
    done
done
