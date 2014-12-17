/* 
 * File:   dTOUCH.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 22:51
 */

#include "dTOUCH.h"


dTOUCH::dTOUCH()
{
    algorithm = algo_dTOUCH;
}

dTOUCH::~dTOUCH()
{
    delete &tree;
}

void dTOUCH::run()
{
    totalTimeStart();
    
    readBinaryInput(file_dsA, file_dsB);
    
    //processing first tree
    if (verbose) std::cout << "Forming the partitions A" << std::endl; 
    createPartitions(vdsA);
    
    if (verbose) std::cout << "Assigning the objects of B" << std::endl; 
    assignment(dsB);
    if (verbose) std::cout << "Assigning B Done." << std::endl; 
    
    if (profilingEnable) 
    {
        analyze();
        if (verbose) std::cout << "Analysis A Done." << std::endl; 
    }
    
    if(localJoin == algo_SGrid)
    {
        countSpatialGrid();
        if (verbose) std::cout << "Counting grid A Done." << std::endl; 
    }
    
    if (verbose) std::cout << "Probing A, doing the join" << std::endl; 
    probe();
    if(localJoin == algo_SGrid)
    {
        if (verbose) std::cout << "Removing duplicates A" << std::endl; 
        deduplicateSpatialGrid();
    }
    if (verbose) 
    {
        std::cout << "Done with first tree. Clear tree." << std::endl;
        std::cout << "Number of results: " << resultPairs.results << std::endl;
    }
    
    //Transfer Statistics to another variables
    int LevelsA = Levels;
    thrust::host_vector<int> levelAssignedTemp;
    thrust::host_vector<double> levelAvgTemp;
    thrust::host_vector<double> levelStdTemp;
    levelAssignedTemp = levelAssigned[0];
    levelAvgTemp = levelAvg[0];
    levelStdTemp = levelStd[0];
    
    tree.clear();

    //processing second tree
    if (vdsB.size() > 0)
    {
        if (verbose) std::cout << "Forming the partitions B" << std::endl; 
        createPartitions(vdsB);

        if (verbose) std::cout << "Assigning the objects of A" << std::endl; 
        assignment(dsA);
        if (verbose) std::cout << "Assigning A Done." << std::endl; 

        if (profilingEnable) 
        {
            analyze();
            if (verbose) std::cout << "Analysis B Done." << std::endl; 
        }

        if(localJoin == algo_SGrid)
        {
            if (verbose) std::cout << "Counting grid B Done." << std::endl; 
            countSpatialGrid();
        }

        if (verbose) std::cout << "Probing A, doing the join" << std::endl; 
        probe();
        
        if(localJoin == algo_SGrid)
        {
            if (verbose) std::cout << "Removing duplicates A" << std::endl; 
            deduplicateSpatialGrid();
        }
        
        if (verbose) std::cout << "Number of results: " << resultPairs.results << std::endl;
    }
    else
    {
        if (verbose) std::cout << "Second tree is empty" << std::endl;
    }
    
    //transfer statistics back and swap
    LevelsD = Levels;
    Levels = LevelsA;
    levelAssigned[1] = levelAssigned[0];
    levelAvg[1] = levelAvg[0];
    levelStd[1] = levelStd[0];
    levelAssigned[0] = levelAssignedTemp;
    levelAvg[0] = levelAvgTemp;
    levelStd[0] = levelStdTemp;
    
    if (verbose) std::cout << "Done." << std::endl;
    
}

void dTOUCH::assignment(SpatialObjectList& ds)
{
    building.start();
    int overlaps;
    int assigned_level;
    bool assigned;
    
    if (ds.size() == 0)
        return;
    
    for (unsigned int i=0;i<ds.size();++i)
    {
        TreeEntry* obj = ds[i];
        
        TreeNode* nextNode;
        TreeNode* ptr = root;
        
        while(true)
        {
            overlaps = false;
            assigned = false;
            for (NodeList::iterator it = ptr->entries.begin(); it != ptr->entries.end(); it++)
            {    
                if ( FLAT::Box::overlap(obj->mbr,(*it)->mbr) )
                {
                    if(overlaps++ == 0)
                    {
                        overlaps = true;
                        nextNode = (*it);
                    }
                    else
                    {
                        if (obj->type == 1)
                        {
                            //should be assigned to this level
                            double coin = (rand()/(double)(RAND_MAX));

                            if (coin > exp(-(((double)ptr->level) * maxLevelCoef/100.)/(double)Levels))
                            {
                                vdsB.push_back(obj);
                            }
                            else
                            {
                                ptr->attachedObjs[obj->type].push_back(obj);
                            }
                            assigned = true;
                            assigned_level = ptr->level;
                            break;
                        }
                        else
                        {
                            ptr->attachedObjs[obj->type].push_back(obj);
                            assigned = true;
                            assigned_level = ptr->level;
                            break;
                        }
                    }
                }
            }
            if(assigned)
            {
                break;
            }
            if(!overlaps)
            {
                //filtered
                filtered[obj->type]++;
                break;
            }
            ptr = nextNode;
            if(ptr->leafnode)
            {
                ptr->attachedObjs[obj->type].push_back(obj);
                break;
            }
        }
    }
}