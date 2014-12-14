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
        FLAT::SpatialObject* obj = dsB.at(i);
        FLAT::Box objMBR = obj->getMBR();
        objMBR.isEmpty = false;
        FLAT::Box::expand(objMBR,epsilon * 1./2.);

        TreeEntry* nextNode;
        TreeNode* ptr = tree.at(root->childIndex);

        nextNode = NULL;

        while(true)
        {
            overlaps = false;
            assigned = false;
            for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
            {    
                if ( FLAT::Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) )
                {
                    if(!overlaps)
                    {
                        overlaps = true;
                        nextNode = ptr->entries.at(cChild);
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
            ptr = tree.at(nextNode->childIndex);
            if(ptr->leafnode)
            {
                    ptr->attachedObjs[0].push_back(obj);
                    break;
            }
        }
    }

    building.stop();
}


void TOUCH::joinNodeToDesc(FLAT::uint64 ancestorNodeID)
{
    SpatialGridHash* spatialGridHash = new SpatialGridHash(this->universeA,localPartitions);
    spatialGridHash->epsilon = this->epsilon;
    queue<FLAT::uint64> leaves;
    TreeNode* leaf, *ancestorNode;
    ancestorNode = tree.at(ancestorNodeID);
    if( localJoin == algo_SGrid )
    {
        gridCalculate.start();
        spatialGridHash->build(ancestorNode->attachedObjs[0]);
        gridCalculate.stop();
    }

    leaves.push(ancestorNodeID);
    while(leaves.size()>0)
    {
        leaf = tree.at(leaves.front());
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
            for (FLAT::uint64 child = 0; child < leaf->entries.size(); ++child)
            {
                leaves.push(leaf->entries.at(child)->childIndex);
            }
        }
    }

    if(localJoin == algo_SGrid)
    {
        spatialGridHash->resultPairs.deDuplicateTime.start();
        spatialGridHash->resultPairs.deDuplicate();
        spatialGridHash->resultPairs.deDuplicateTime.stop();

        
        this->ItemsCompared += spatialGridHash->ItemsCompared;
        this->resultPairs.results += spatialGridHash->resultPairs.results;
        this->resultPairs.duplicates += spatialGridHash->resultPairs.duplicates;
        this->repA += spatialGridHash->repA;
        this->repB += spatialGridHash->repB;
        this->resultPairs.deDuplicateTime.add(spatialGridHash->resultPairs.deDuplicateTime);
    }
}
	
void TOUCH::analyze()
{
    analyzing.start();
    FLAT::uint64 emptyCells=0;
    FLAT::uint64 sum=0,sqsum=0;
    double differenceSquared=0;
    footprint += vdsA.size()*(sizeof(TreeEntry*));
    footprint += dsB.size()*(sizeof(FLAT::SpatialObject*));
    LVL = Levels;
    //vector<uint64> ItemPerLevel;
    ItemPerLevel.reserve(Levels);
    for(int i = 0 ; i<Levels ; i++)
            ItemPerLevel.push_back(0);
    for(unsigned int ni = 0; ni<tree.size() ; ni++)
    {
            SpatialObjectList objs = tree.at(ni)->attachedObjs[0];
            FLAT::uint64 ptrs = objs.size();
            if(objs.size()==0)emptyCells++;
            ItemPerLevel[tree.at(ni)->level]+=ptrs;
            sum += ptrs;
            sqsum += ptrs*ptrs;
            if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

    }
    if (verbose)
        for(int i = 0 ; i<Levels ; i++)
            std::cout<< "Level " << i << " items " << ItemPerLevel[i] << std::endl;
    
    int top10Level = (Levels>10)?10:Levels;
    for(int i = 0 ; i<top10Level ; i++)
        levelAssignedA[i] = ItemPerLevel[i];

    footprint += sum*sizeof(FLAT::SpatialObject*) + tree.size()*(sizeof(TreeNode*));
    avg = (sum+0.0) / (tree.size());
    percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
    differenceSquared = ((double)sqsum/(double)tree.size())-avg*avg;
    std = sqrt(differenceSquared);
    analyzing.stop();
}