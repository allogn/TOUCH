#!/bin/bash
num=128
for exp in {1..8..1}
do
    num=$(expr $num \* 2)
    num2=$(expr $num \* 1000)
    mkdir ../data/samplesExpNum/$num2
        echo "./SampleGenerator -i ../data/RawData-segment-object-1M-Axons.bin -p ../data/samplesAxExpDifLen/$num2/ -s $num2 -n 10 -e 1"
        ./SampleGenerator -p ../data/samplesExpNum/$num2/ -s $num2 -n 10 -e 1
done
