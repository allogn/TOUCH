#!/bin/bash
for size in {2000..10000..2000}
do
    mkdir ./sampleExt/$size
    ./SampleGenerator -i ../data/RandomData-320K.bin -p ./sampleExt/$size/ -s $size -v 1 -n 10 -e 1
    #echo "./SampleGenerator -i ../data/RandomData-320K.bin -p ./sampleExt/$size/ -s $size -v -n 10 -e 1"
done
