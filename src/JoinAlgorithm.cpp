/* 
 * File:   JoinAlgorithm.cpp
 * Author: Alvis
 * 
 * Created on 30.10.2014
 */

#include "JoinAlgorithm.h"

JoinAlgorithm::JoinAlgorithm() {
    hashprobe               = 0;
    footprint               = 0;
    filtered[0]             = 0;
    filtered[1]             = 0;
    maxMappedObjects        = 0;
    avg                     = 0;
    std                     = 0;
    ItemsCompared           = 0;
    ItemsMaxCompared        = 0;
    percentageEmpty         = 0;
    repA                    = 1;
    repB                    = 1;
    algorithm               = algo_NL;
    localJoin               = algo_NL;
    partitions              = 4;
    profilingEnable         = true;
    epsilon                 = 1.5;
    
    verbose                 =  true;
    
    base = 2; // the base for S3 and SH algorithms
    logfilename = "SJ.csv"; //@todo add to parameters
    numA = 0, numB = 0;
}

JoinAlgorithm::~JoinAlgorithm() {}

void JoinAlgorithm::readBinaryInput(string in_dsA, string in_dsB) {
    
    if (verbose) std::cout << "Start reading the datasets" << std::endl;

    file_dsA = in_dsA;
    file_dsB = in_dsB;

    FLAT::DataFileReader *inputA = new FLAT::DataFileReader(file_dsA);
    FLAT::DataFileReader *inputB = new FLAT::DataFileReader(file_dsB);
    
    if (verbose)
    {
        inputA->information();
        inputB->information();
    }

    TreeEntry* newEntry;

    dataLoad.start();

    size_dsA = (numA < inputA->objectCount && (numA != 0))?numA:inputA->objectCount;
    size_dsB = (numB < inputB->objectCount && (numB != 0))?numB:inputB->objectCount;
    
    if (verbose)
    {
        std::cout << "size of A:" << size_dsA << "# from " << inputA->objectCount << "# " 
                        << size_dsA*sizeof(SpatialObjectList) / 1000.0 << "KB" << std::endl;
        std::cout << "size of B:" << size_dsB << "# from " << inputB->objectCount << "# " 
                        << size_dsB*sizeof(SpatialObjectList) / 1000.0 << "KB" << std::endl;
    }
    
    FLAT::Box mbr;
    for (int i=0;i<DIMENSION;i++)
    {
        universeA.low.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::max();
        universeA.high.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::min();
        universeB.low.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::max();
        universeB.high.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::min();
    }
    FLAT::SpatialObject* sobj;

    dsA.reserve(size_dsA);
    vdsA.reserve(size_dsA);
    numA = size_dsA;
    while(inputA->hasNext() && (numA-- != 0))
    {
        sobj = inputA->getNext();
        newEntry = new TreeEntry(sobj,0,numA,epsilon);
        
        for (int i=0;i<DIMENSION;i++)
        {
            universeA.low.Vector[i] = min(universeA.low.Vector[i],newEntry->mbr.low.Vector[i]);
            universeA.high.Vector[i] = max(universeA.high.Vector[i],newEntry->mbr.high.Vector[i]);
        }
        
        vdsA.push_back(newEntry);
        dsA.push_back(sobj);
        vdsAll.push_back(newEntry);
    }

    dsB.reserve(size_dsB);
    numB = size_dsB;
    while (inputB->hasNext() && (numB-- != 0))
    {
        sobj = inputB->getNext();
        newEntry = new TreeEntry(sobj,1,numB,epsilon);
        
        for (int i=0;i<DIMENSION;i++)
        {
                universeB.low.Vector[i] = min(universeB.low.Vector[i],newEntry->mbr.low.Vector[i]);
                universeB.high.Vector[i] = max(universeB.high.Vector[i],newEntry->mbr.high.Vector[i]);
        }
        dsB.push_back(sobj);
        vdsAll.push_back(newEntry);
    }
    
    FLAT::Box::expand(universeA,epsilon);
    FLAT::Box::expand(universeB,epsilon);

    dataLoad.stop();

    if (verbose) std::cout << "Reading Completed." << std::endl;

}

void JoinAlgorithm::print()
{
 
        
        if (verbose)
        {
            std::cout<< "\n================================\n";
            std::cout << algoname() << " using " << basealgo() << " gridSize " << localPartitions << '\n'
            << "memFP(MB) " << (footprint+0.0)/(1024.0*1024.0) << " #A " << size_dsA << " #B " << size_dsB << '\n'
            << "size" << " SOlist "<< sizeof(SpatialObjectList) << " SO* "<< sizeof(FLAT::SpatialObject *) << " Node* "<< sizeof(TreeNode*) << '\n'
            << "EmptyCells(%) " << percentageEmpty	<< " MaxObj " << maxMappedObjects << " AveObj " << avg << " StdObj " << std << '\n'

            << "Compared # " << ItemsCompared << " % " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB) << '\n'
            << "Duplicates " << resultPairs.duplicates << " Selectivity " << 100.0*(double)resultPairs.results/(double)(size_dsA*size_dsB) << '\n'
            << "Results " << resultPairs.results << '\n'
            << "filtered A " << filtered[0]	<< " B " << filtered[1] << " repA " << repA	<< " repB " << repB << '\n'

            << "Times: total " << total << '\n'
            << " loading " << dataLoad << " init " << initialize	<< " build " << building << " probe " << probing << '\n'
            << " comparing " << comparing << " partition " << partition	<< '\n'
            << " deDuplicating " << resultPairs.deDuplicateTime	<< " analyzing " << analyzing << " sorting " << sorting << '\n'
            << "Partitions " << partitions << " epsilon " << epsilon << " Fanout " << nodesize << '\n'
            << "\n================================\n"
            << "\ndatasets\n" << file_dsA << '\n' << file_dsB << '\n';

            std::cout<<"Done."<<std::endl;
        }
        else
        {
            std::cout << algoname() << " done. Result: " << resultPairs.results << std::endl;
        }

}