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
}