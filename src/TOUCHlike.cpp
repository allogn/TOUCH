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
        << "Compared #, Compared %, ComparedMax, Duplicates, Results, Selectivity, filtered A, filtered B," // TOUCH
        << "t loading, t init, t build, t probe, t comparing, t partition, t join, t total, t deDuplicating, t analyzing, t sorting, t gridCalculate,"
        << "EmptyCells(%), MaxObj, AveObj, StdObj, repA, repB\n";
    }
    //check if file exists
            
    fout
    << algoname << "," << epsilon << "," << size_dsA << "," << size_dsB << "," << file_dsA << "," << file_dsB << ","
    << basealgo << "," << nodesize << "," << partitions << "," << localPartitions << ","
            
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
            << Ljoin << "," 
            << total << "," 
            << resultPairs.deDuplicateTime << "," 
    << analyzing << ","
            << sorting << ","
            << gridCalculate << ","
            << percentageEmpty << ","
            << maxMappedObjects << "," 
    << avg << "," 
            << std << "," 
            << repA << ","
            << repB << "\n";

}


void TOUCHlike::countSpatialGrid()
{
    gridCalculate.start();
    FLAT::Box mbr;
    for (std::vector<TreeNode*>::iterator it = tree.begin(); it != tree.end(); it++)
    { 
        for (int type = 0; type < TYPES; type++)
        {
            mbr = (type == 0)?universeA:universeB;
            mbr.isEmpty = false;
            FLAT::Box::expand(mbr,10000);
            (*it)->spatialGridHash[type] = new SpatialGridHash(mbr,localPartitions);
            (*it)->spatialGridHash[type]->epsilon = this->epsilon;
            (*it)->spatialGridHash[type]->build((*it)->attachedObjs[type]);
            
            if (this->algorithm == algo_reTOUCH || this->algorithm == algo_rereTOUCH)
            {
                (*it)->spatialGridHashAns[type] = new SpatialGridHash(mbr,localPartitions);
                (*it)->spatialGridHashAns[type]->epsilon = this->epsilon;
//                if ((*it)->parentEntry->childIndex == 34)
//                {
//                    for (int u = 0; u < (*it)->attachedObjsAns[type].size(); u++)
//                    {
//                        cout << "ancestosattacheced objects to node 34: " << (*it)->attachedObjsAns[type][u]->id << endl;
//                    }
//                }
                (*it)->spatialGridHashAns[type]->build((*it)->attachedObjsAns[type]);
            }
        }
    }
    gridCalculate.stop();
}

void TOUCHlike::deduplicateSpatialGrid()
{
        for (std::vector<TreeNode*>::iterator it = tree.begin(); it != tree.end(); it++)
        {
            for (int type = 0; type < TYPES; type++)
            {
                (*it)->spatialGridHash[type]->resultPairs.deDuplicateTime.start();
                (*it)->spatialGridHash[type]->resultPairs.deDuplicate();
                (*it)->spatialGridHash[type]->resultPairs.deDuplicateTime.stop();

                this->ItemsCompared += (*it)->spatialGridHash[type]->ItemsCompared;
                this->resultPairs.results += (*it)->spatialGridHash[type]->resultPairs.results;
                this->resultPairs.duplicates += (*it)->spatialGridHash[type]->resultPairs.duplicates;
                this->repA += (*it)->spatialGridHash[type]->repA;
                this->repB += (*it)->spatialGridHash[type]->repB;
                this->resultPairs.deDuplicateTime.add((*it)->spatialGridHash[type]->resultPairs.deDuplicateTime);
                this->initialize.add((*it)->spatialGridHash[type]->initialize);
                //(*it)->spatialGridHash[type]->resultPairs.printAllResults();
                
                if (this->algorithm == algo_reTOUCH || this->algorithm == algo_rereTOUCH)
                {
                    (*it)->spatialGridHashAns[type]->resultPairs.deDuplicateTime.start();
                    (*it)->spatialGridHashAns[type]->resultPairs.deDuplicate();
                    (*it)->spatialGridHashAns[type]->resultPairs.deDuplicateTime.stop();
                    this->ItemsCompared += (*it)->spatialGridHashAns[type]->ItemsCompared;
                    this->resultPairs.results += (*it)->spatialGridHashAns[type]->resultPairs.results;
                    this->resultPairs.duplicates += (*it)->spatialGridHashAns[type]->resultPairs.duplicates;
                    this->repA += (*it)->spatialGridHashAns[type]->repA;
                    this->repB += (*it)->spatialGridHashAns[type]->repB;
                    this->resultPairs.deDuplicateTime.add((*it)->spatialGridHashAns[type]->resultPairs.deDuplicateTime);
                    this->initialize.add((*it)->spatialGridHashAns[type]->initialize);
                    //(*it)->spatialGridHashAns[type]->resultPairs.printAllResults();
                }
            }
        }
}