data1="../../data/RandomData-160K.bin"

for samplesize in {2000..10000..2000}
do
    mkdir ./samples/$samplesize
    echo "../../bin/SampleGenerator -i $data1 -p ./samples/$samplesize/ -s $samplesize -n 10"
    ../../bin/SampleGenerator -i $data1 -p ./samples/$samplesize/ -s $samplesize -n 10
done
