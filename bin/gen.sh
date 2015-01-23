#!/bin/bash
num=64
for exp in {1..1..1}
do
    num=$(expr $num \* 2)
    num2=$(expr $num \* 1000)
    mkdir ../data/samplesExpNumStart0-10only128/$num2
        echo "./SampleGenerator -i ../data/RawData-segment-object-1M-Axons.bin -p ../data/samplesAxExpDifLen/$num2/ -s $num2 -n 10 -e 1"
        ./SampleGenerator -p ../data/20to30/$num2/ -s $num2 -n 10 -e 1 -v 1
done
