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
                    ../../../bin/SpatialJoin -a 8 -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesRandomExpand/$samplesize/sample$firstfile.bin ../../../data/samplesRandomExpand/$samplesize/sample$secondfile.bin -y 0 -g 100
            fi
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            if [ "$secondfile" -ge "$firstfile" ]; then
                echo hello
            else
                    ../../../bin/SpatialJoin -a 8 -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesRandomExpand/$samplesize/sample$firstfile.bin ../../../data/samplesRandomExpand/$samplesize/sample$secondfile.bin -y 1 -g 100
            fi
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            if [ "$secondfile" -ge "$firstfile" ]; then
                echo hello
            else
                    ../../../bin/SpatialJoin -a 8 -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesRandomExpand/$samplesize/sample$firstfile.bin ../../../data/samplesRandomExpand/$samplesize/sample$secondfile.bin -y 2 -g 100
            fi
        done
    done
done

for samplesize in {2000..10000..2000}
do
    for firstfile in {1..10..1}
    do
        for secondfile in {1..10..1}
        do
            if [ "$secondfile" -ge "$firstfile" ]; then
                echo hello
            else
                    ../../../bin/SpatialJoin -a 8 -l 100 -e 5 -b 3 -J 2 -i ../../../data/samplesRandomExpand/$samplesize/sample$firstfile.bin ../../../data/samplesRandomExpand/$samplesize/sample$secondfile.bin -y 3 -g 100
            fi
        done
    done
done
