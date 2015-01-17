#!/bin/bash
rm ./SJ.csv
for num in {5000..30000..5000}
do
    mkdir ../data/samplesAxExpDifLenSmall/$num
        echo "./SampleGenerator -i ../data/RawData-segment-object-1M-Axons.bin -p ../data/samplesAxExpDifLen/$num/ -s $num -n 10 -e 1"
        ./SampleGenerator -i ../data/RawData-segment-object-1M-Axons.bin -p ../data/samplesAxExpDifLenSmall/$num/ -s $num -n 10 -e 1
done
