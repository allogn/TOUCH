/* 
 * File:   TOUCH.cpp
 * 
 */

#include "TOUCH.h"

TOUCH::TOUCH() {
    algorithm = algo_TOUCH;
}

TOUCH::~TOUCH() {
}

void TOUCH::run() {
    totalTimeStart();
    readBinaryInput(file_dsA, file_dsB);
    if (verbose) std::cout << "Forming the partitions" << std::endl; 
    createPartitions(vdsA);
    if (verbose) std::cout << "Assigning the objects of B" << std::endl; 
    assignment();
    if (verbose) std::cout << "Assigning Done." << std::endl; 
    analyze();
    if (verbose) std::cout << "Analysis Done" << std::endl; 
    if (verbose) std::cout << "Probing, doing the join" << std::endl; 
    probe();
    if (verbose) std::cout << "Done." << std::endl; 
    totalTimeStop();
}

void TOUCH::assignment()
{
    building.start();
    bool overlaps;
    bool assigned;
    for (unsigned int i=0;i<dsB.size();++i)
    {
        TreeEntry* obj = dsB[i];

        TreeNode* nextNode;
        TreeNode* ptr = root;

        nextNode = NULL;

        while(true)
        {
            overlaps = false;
            assigned = false;
            for (NodeList::iterator it = ptr->entries.begin(); it != ptr->entries.end(); it++)
            {    
                if ( FLAT::Box::overlap(obj->mbr,(*it)->mbr) )
                {
                    if(!overlaps)
                    {
                        overlaps = true;
                        nextNode = (*it);
                    }
                    else
                    {
                        // assignment to current level
                        ptr->attachedObjs[0].push_back(obj);
                        assigned = true;
                        break;
                    }
                }
            }
            if(assigned)
                    break;
            if(!overlaps)
            {
                    //filtered
                    filtered[0] ++;
                    break;
            }
            ptr = nextNode;
            if(ptr->leafnode)
            {
                ptr->attachedObjs[0].push_back(obj);
                break;
            }
        }
    }

    building.stop();
}


void TOUCH::joinNodeToDesc(TreeNode* ancestorNode)
{
    SpatialGridHash* spatialGridHash = new SpatialGridHash(this->universeA,localPartitions);
    spatialGridHash->epsilon = this->epsilon;
    queue<FLAT::uint64> leaves;
    TreeNode* leaf;
    if( localJoin == algo_SGrid )
    {
        gridCalculate.start();
        spatialGridHash->build(ancestorNode->attachedObjs[0]);
        gridCalculate.stop();
    }

    leaves.push(ancestorNode);
    while(leaves.size()>0)
    {
        leaf = leaves.front();
        leaves.pop();
        if(leaf->leafnode)
        {
            ItemsMaxCompared += ancestorNode->attachedObjs[0].size()*leaf->entries.size();
            comparing.start();
            if(localJoin == algo_SGrid)
            {
                spatialGridHash->probe(leaf);
            }
            else
            {
                NL(leaf,ancestorNode->attachedObjs[0]);
            }
            comparing.stop();
        }
        else
        {
            for (NodeList::iterator it = leaf->entries.begin(); it != leaf->entries.end(); it++)
            {
                leaves.push((*it));
            }
        }
    }

    if(localJoin == algo_SGrid)
    {
        spatialGridHash->resultPairs.deDuplicate();
        SpatialGridHash::transferInfo(spatialGridHash,this);
    }
}