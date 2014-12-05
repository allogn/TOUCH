#!/bin/bash
data1="../../data/RandomData-100K.bin"
data2="../../data/RandomData-Normal-500-250-160K.bin"
rm ./SJ.csv

for epsilon in {1..10..2}
do
    for objnum in {6000..8000..2000}
    do
        for alg in {4,6,7,8,9}
	do
	    echo "../../bin/SpatialJoin -a $alg -p 100 -e $epsilon -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum"
	    ../../bin/SpatialJoin -a $alg -p 100 -e $epsilon -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum
	done
    done
   
done
