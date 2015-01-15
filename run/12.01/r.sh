#!/bin/bash
rm -f ./SJ.csv
epsilon="50"
for samplesize in {2000..10000..2000}
do
    for firstfile in {1..2..1}
    do
        for secondfile in {1..2..1}
        do
echo " ../../../bin/SpatialJoin -e $epsilon -l 100 -b 2 -J 2 -a 8 -y 0 -g 10 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin"
		    ../../../bin/SpatialJoin -e $epsilon -l 100 -b 2 -J 2 -a 8 -y 0 -g 4 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
			../../../bin/SpatialJoin -e $epsilon -l 100 -b 2 -J 2 -a 4 -g 4 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
			../../../bin/SpatialJoin -e $epsilon -l 1000 -b 2 -J 2 -a 6 -g 4 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
			../../../bin/SpatialJoin -e $epsilon -l 100 -b 2 -J 2 -a 7 -g 4 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin
			../../../bin/SpatialJoin -e $epsilon -a 2 -i ../../../data/samplesDendrites/$samplesize/sample$firstfile.bin ../../../data/samplesAxons/$samplesize/sample$secondfile.bin

        done
    done
done

