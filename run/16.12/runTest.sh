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
                for alg in {8,9}
                do
                    ../../bin/SpatialJoin -a $alg -p 100 -e 5 -b 3 -t 1 -J 2 -i ./samples/$samplesize/sample$firstfile.bin ./samples/$samplesize/sample$secondfile.bin
                done
            fi
        done
    done
done
