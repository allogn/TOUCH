/* 
 * File:   TOUCHlike.cpp
 * Author: Alvis
 * 
 */

#include "TOUCHlike.h"

TOUCHlike::TOUCHlike() {
    localPartitions = 100;
    nodesize = base;
    leafsize = partitions;
    PartitioningType = No_Sort;
    total.start();
}

TOUCHlike::~TOUCHlike() {
}

void TOUCHlike::printTOUCH() {
    
    print(); // print statistics of JoinAlgorithm
    
    ofstream fout(logfilename.c_str(),ios_base::app);
    
    FLAT::uint64 comparisons = 0;
    for(int i = 0 ; i<LVL ; i++)
        comparisons += pow(base,i)*(leafsize*ItemPerLevel[i]);
    
    fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
    100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)"
        << " sortType " << PartitioningType <<
        " Filtered " <<	filtered[0] << "(#) = " << 100*(double)filtered[0]/(double)size_dsB << "(%) Level";

    for(int i = 0 ; i<LVL ; i++)
        fout << " " << i << " : " << ItemPerLevel[i] << "(#) = " 
             <<  100*(double)ItemPerLevel[i]/(double)size_dsB << "(%)";
    fout.close();
}