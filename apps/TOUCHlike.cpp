/* 
 * File:   TOUCHlike.cpp
 * Author: Alvis
 * 
 */

#include "TOUCHlike.h"

TOUCHlike::TOUCHlike() {
}

TOUCHlike::~TOUCHlike() {
}

TOUCHlike::printTOUCH() {
    
    print();
    ofstream fout(logfilename.c_str(),ios_base::app);
    uint64 leafsize = ceil((double)size_dsA / (double)partitions);
    uint64 comparisons = 0;
    for(int i = 0 ; i<LVL ; i++)
        comparisons += pow(base,i)*(leafsize*ItemPerLevel[i]);
    //ofstream lg; lg.open("tree.txt",ofstream::app);
    fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
    100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)" <<
    " Filtered " <<	filtered << "(#) = " << 100*(double)filtered/(double)size_dsB << "(%) Level";

    for(int i = 0 ; i<LVL ; i++)
        fout<< " " << i << " : " << ItemPerLevel[i] << "(#) = " <<  100*(double)ItemPerLevel[i]/(double)size_dsB << "(%)";
        fout.close();
}