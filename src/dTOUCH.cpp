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
    
    if (verbose) std::cout << "Probing A, doing the join" << std::endl; 
    probe();
    
    if (verbose) 
    {
        std::cout << "Done with first tree. Clear tree." << std::endl;
        std::cout << "Number of results: " << resultPairs.results << std::endl;
    }
    
    //Transfer Statistics to another variables
    int LevelsA = Levels;
    std::vector<int> levelAssignedTemp;
    std::vector<double> levelAvgTemp;
    std::vector<double> levelStdTemp;
    levelAssignedTemp = levelAssigned[0];
    levelAvgTemp = levelAvg[0];
    levelStdTemp = levelStd[0];
    
    tree.clear();
    ItemPerLevel[0].clear();
    ItemPerLevelAns[0].clear();
    ItemPerLevel[1].clear();
    ItemPerLevelAns[1].clear();
    levelAvg[0].clear();
    levelAvg[1].clear();
    levelStd[1].clear();
    levelStd[0].clear();
    
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

        if (verbose) std::cout << "Probing A, doing the join" << std::endl; 
        probe();
        
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
    
    dTOUCHleafType = !ds.front()->type;
    
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

                            
                            //old good version
                            //if (coin > exp(-(((double)ptr->level) * maxLevelCoef/100.)/(double)Levels))
                            
                            
                            
                            double x = (double)(ptr->level)/(double)Levels; // zero level is below and is always one
                            //cout << coin << " < " << exp(-x*maxLevelCoef)*(1-x) << " with level " << ptr->level << endl;
                            if (coin > exp(-x*maxLevelCoef)*(1-x))
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

//void dTOUCH::joinNodeToDesc(TreeNode* ancestorNode)
//{
//    for (int type = 0; type < TYPES; type++)
//    {
//        SpatialGridHash* spatialGridHash;
//        queue<TreeNode*> leaves;
//        TreeNode* leaf;
//        if( localJoin == algo_SGrid )
//        {
//            spatialGridHash = new SpatialGridHash(this->universeA,localPartitions);
//            spatialGridHash->epsilon = this->epsilon;
//            gridCalculate.start();
//            spatialGridHash->build(ancestorNode->attachedObjs[type]);
//            gridCalculate.stop();
//        }
//
//        leaves.push(ancestorNode);
//        while(leaves.size()>0)
//        {
//            leaf = leaves.front();
//            leaves.pop();
//            if(leaf->leafnode)
//            {
//                ItemsMaxCompared += ancestorNode->attachedObjs[type].size()*leaf->attachedObjs[!type].size();
//                comparing.start();
//                if(localJoin == algo_SGrid)
//                {
//                    spatialGridHash->probe(leaf->attachedObjs[!type]);
//                }
//                else
//                {
//                    NL(leaf->attachedObjs[!type],ancestorNode->attachedObjs[type]);
//                }
//                comparing.stop();
//            }
//            else
//            {
//                for (NodeList::iterator it = leaf->entries.begin(); it != leaf->entries.end(); it++)
//                {
//                    leaves.push((*it));
//                }
//            }
//        }
//
//        if(localJoin == algo_SGrid)
//        {
//            spatialGridHash->resultPairs.deDuplicate();
//            SpatialGridHash::transferInfo(spatialGridHash,this);
//        }
//    }
//}