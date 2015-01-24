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
    treeTraversal           = 1;
    swapMem                 = 0;
    ramMem                  = 0;
    
    verbose                 =  true;
    
    base = 2; // the base for S3 and SH algorithms
    logfilename = "SJ.csv"; //@todo add to parameters
    numA = 0, numB = 0;
    
    for (int t = 0; t < TYPES; t++)
    {
        levelAssigned[t].resize(10,0);
        levelAvg[t].resize(10,0);
        levelStd[t].resize(10,0);
    }
}



void JoinAlgorithm::saveLog() {
    
    bool headers;
    headers = ( access( logfilename.c_str(), F_OK ) == -1 );
    
    ofstream fout(logfilename.c_str(),ios_base::app);
    
    /*
     * If there is file - append
     * If not - create and (probably?) create headers
     */
    if (headers)
    {
        fout << "Algorithm, Epsilon, #A, #B, infile A, infile B, LocalJoin Alg, Fanout, Leaf size, gridSize, " // common parameters
        << "Compared #, Compared %, ComparedMax, Duplicates, Results, Selectivity, filtered A, filtered B," // TOUCH
        << "t loading, t init, t build, t probe, t comparing, t partition, t total, t deDuplicating, t analyzing, t sorting, t gridCalculate, t sizeCalculate,"
        << "EmptyCells(%), MaxObj, AveObj, StdObj, repA, repB, max level, gridP robe, tree height A, tree height B, Memory SwapFile, Memory RAM, Memory Clear, addFilter"
        << "l0 assigned, l1 assigned, l2 assigned, l3 assigned, l4 assigned, l5 assigned, l6 assigned, l7 assigned, l8 assigned, l9 assigned,"
        << "l0 assigned B, l1 assigned B, l2 assigned B, l3 assigned B, l4 assigned B, l5 assigned B, l6 assigned B, l7 assigned B, l8 assigned B, l9 assigned B,"
        << "l0 avg, l1 avg, l2 avg, l3 avg, l4 avg, l5 avg, l6 avg, l7 avg, l8 avg, l9 avg,"
        << "l0 avg B, l1 avg B, l2 avg B, l3 avg B, l4 avg B, l5 avg B, l6 avg B, l7 avg B, l8 avg B, l9 avg B,"
        << "l0 std, l1 std, l2 std, l3 std, l4 std, l5 std, l6 std, l7 std, l8 std, l9 std, "
        << "l0 std B, l1 std B, l2 std B, l3 std B, l4 std B, l5 std B, l6 std B, l7 std B, l8 std B, l9 std B"
        << "\n";
    }
    //check if file exists
    
    FLAT::Timer t;
    t.add(probing);
    t.add(gridCalculate);
            
    fout
    << algoname() << "," << epsilon << "," << size_dsA << "," << size_dsB << "," << file_dsA << "," << file_dsB << ","
    << basealgo() << "," << nodesize << "," << leafsize << "," << localPartitions << ","
            
    << ItemsCompared << "," 
            << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB) << ","
            << ItemsMaxCompared << ","
    << resultPairs.duplicates << ","
            << resultPairs.results << ","
            << 100.0*(double)resultPairs.results/(double)(size_dsA*size_dsB) << "," 
    << filtered[0] << "," 
            << filtered[1] << "," 
            << dataLoad << ","
            << initialize << ","
            << building << "," 
            << probing << "," 
    << comparing << ","
            << partition << ","
            << total << "," 
            << resultPairs.deDuplicateTime << "," 
    << analyzing << ","
            << sorting << ","
            << gridCalculate << ","
            << sizeCalculate << ","
            << percentageEmpty << ","
            << maxMappedObjects << "," 
    << avg << "," 
            << std << "," 
            << repA << ","
            << repB << ","
            << maxLevelCoef << ","
            << t << ","
            << Levels << ","
            << LevelsD << ","
            << swapMem << ","
            << ramMem   << ","
            << clearMem << ","
            << addFilter << ",";
    for (int t = 0; t < TYPES; t++)
        for (int i = 0; i < 10; i++)
            fout << levelAssigned[t][i] << ",";
    
    
    for (int t = 0; t < TYPES; t++)
        for (int i = 0; i < 10; i++)
            fout << levelAvg[t][i] << ",";
    
    
    for (int t = 0; t < TYPES; t++)
        for (int i = 0; i < 10; i++)
            fout << levelStd[t][i] << ",";
    
            fout << "\n";

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
        dsA.push_back(newEntry);
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
        dsB.push_back(newEntry);
        vdsAll.push_back(newEntry);
    }
    
    universeA.isEmpty = false;
    universeB.isEmpty = false;
    FLAT::Box::expand(universeA,epsilon);
    FLAT::Box::expand(universeB,epsilon);

    dataLoad.stop();

    if (verbose) std::cout << "Reading Completed." << std::endl;

}



void JoinAlgorithm::print()
{
    double avgs[TYPES];
    double stds[TYPES];
    avgs[0] = 0;
    avgs[1] = 1;
    stds[0] = 0;
    stds[1] = 1;
    int num[TYPES];
    num[0] = 0;
    num[1] = 0;
    for (int type = 0; type < TYPES; type++)
    {
        for(int i = 0 ; i<Levels; i++)
        {
            avgs[type] += levelAvg[type][i]*levelAssigned[type][i];
            num[type] += levelAssigned[type][i];
        }
        if (num[type] != 0)
        {
            avgs[type] = avgs[type]*1./num[type];
        }
    }
        
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
            << "Avg size: " << avgs[0] << " and " << avgs[1] << " ; "
            << "Std size: " << stds[0] << " and " << stds[1]
            << "\n================================\n"
            << "\ndatasets\n" << file_dsA << '\n' << file_dsB << '\n';

            std::cout<<"Done."<<std::endl;
        }
        else
        {
            std::cout << algoname() << " done. Result: " << resultPairs.results << "; Total time: " << total << "; Total memory: Swap/File "
                    << swapMem << " KB RAM " << ramMem << " KB Clear " << clearMem/1000 << " KB" << std::endl;
        }

}


/*
 http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
 */
// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0

void JoinAlgorithm::process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}