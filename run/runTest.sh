#!/bin/bash
data1="../../data/RawData-segment-object-1M-Axons.bin"
data2="../../data/RawData-segment-object-1M-Dendrites.bin"
for epsilon in {5..50..10}
do
    for objnum in {4000..7000..1000}
    do
        for alg in {4,6,7,8}
	do
	    echo "../../bin/SpatialJoin -a $alg -p 100 -e $epsilon -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum"
	    ../../bin/SpatialJoin -a $alg -p 100 -e $epsilon -b 3 -t 1 -J 2 -i $data1 $data2 -n $objnum $objnum
	done
    done
   
done
