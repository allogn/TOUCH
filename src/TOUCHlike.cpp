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
    
    bool headers;
    headers = ( access( logfilename.c_str(), F_OK ) == -1 );
    
    ofstream fout(logfilename.c_str(),ios_base::app);
    
    /*
     * If there is file - append
     * If not - create and (probably?) create headers
     */
    if (headers)
    {
        fout << "Algorithm, Epsilon, #A, #B, infile A, infile B, LocalJoin Alg, Fanout, Partitions, gridSize, " // common parameters
        << "Compared #, Compared %, Duplicates, Results, Selectivity, filtered A, filtered B," // TOUCH
        << "t loading, t init, t build, t probe, t comparing, t partition, t join, t total, t deDuplicating, t analyzing, t sorting,"
        << "EmptyCells(%), MaxObj, AveObj, StdObj, repA, repB\n";
    }
    //check if file exists
            
    fout
    << algoname << "," << epsilon << "," << size_dsA << "," << size_dsB << "," << file_dsA << "," << file_dsB << ","
    << basealgo << "," << base << "," << partitions << "," << gridSize << ","
            
    << ItemsCompared << "," 
            << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB) << ","
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
            << Ljoin << "," 
            << total << "," 
            << resultPairs.deDuplicateTime << "," 
    << analyzing << ","
            << sorting << ","
            << percentageEmpty << ","
            << maxMappedObjects << "," 
    << avg << "," 
            << std << "," 
            << repA << ","
            << repB << "\n";

}