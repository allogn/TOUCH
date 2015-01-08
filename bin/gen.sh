#!/bin/bash
rm ./SJ.csv
for num in {1000..5000..1000}
do
    for alg in {1..9..1}
    do
        echo "./SpatialJoin -e 5 -J 2 -n $num $num -a $alg"
        ./SpatialJoin -e 5 -J 2 -n $num $num -a $alg
    done
done
