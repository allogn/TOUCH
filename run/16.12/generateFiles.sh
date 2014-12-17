data1="../../data/RawData-segment-object-1M-Dendrites.bin"

for samplesize in {2000..10000..2000}
do
    mkdir ./samplesDendrites/$samplesize
    echo "../../bin/SampleGenerator -i $data1 -p ./samplesDendrites/$samplesize/ -s $samplesize -n 10"
    ../../bin/SampleGenerator -i $data1 -p ./samplesDendrites/$samplesize/ -s $samplesize -n 10
done
