#!/bin/bash
rm -f ./SJ.csv
num=128
for samplesize in {1..8..1}
do
    num=$(expr $num \* 2)
    num2=$(expr $num \* 1000)
    for epsilon in {10..100..10}
    do
        for file1 in {1..10..1}
        do
            for file2 in {$file1..10..1}
            do
                echo "running E = $epsilon and ObjNum = $samplesize"
                ../../../bin/SpatialJoin -a 4 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesExpNum/$num/sample$file1.bin ../../../data/samplesExpNum/$num/sample$file2.bin -y 1 -g 100 -s 0
                ../../../bin/SpatialJoin -a 2 -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesExpNum/$num/sample$file1.bin ../../../data/samplesExpNum/$num/sample$file2.bin -y 0 -g 100 -s 0
                for algo in {1,5,6,7,8,9}
                do
                  ../../../bin/SpatialJoin -a $algo -l 100 -e $epsilon -b 3 -J 2 -i ../../../data/samplesExpNum/$num/sample$file1.bin ../../../data/samplesExpNum/$num/sample$file2.bin -y 0 -g 10 -s 2
                done
            done
        done
    done
done
