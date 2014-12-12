/* 
 * File:   TOUCH.cpp
 * 
 */

#include "TOUCH.h"

TOUCH::TOUCH() {
    algorithm = algo_TOUCH; //@todo do it for all
    total.start(); // timing
}

TOUCH::~TOUCH() {
    delete &tree;
    total.stop();
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

void TOUCH::writeNode(std::vector<TreeEntry*> objlist,int Level)
{
    TreeNode* prNode = new TreeNode(Level);
    FLAT::Box mbr;
    FLAT::uint64 childIndex;
    totalnodes++;
    
    for (std::vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
    {
            prNode->entries.push_back(*it);
            mbr = FLAT::Box::combineSafe((*it)->mbr,mbr);
    }
    childIndex = tree.size();
    tree.push_back(prNode);
    nextInput.push_back(new TreeEntry(mbr,childIndex));
    prNode->parentEntry = nextInput.back();
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


void TOUCH::joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID)
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
                JOIN(leaf,ancestorNode->attachedObjs[0]);
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
        
void TOUCH::probe()
{
    probing.start();
    std::queue<FLAT::uint64> Qnodes;

    TreeNode* currentNode;
    Qnodes.push(root->childIndex);

    int lvl = Levels;
    // A BFS on the tree then for each find all its leaf nodes by another BFS
    while(Qnodes.size()>0)
    {
        FLAT::uint64 currentNodeID = Qnodes.front();
        currentNode = tree.at(currentNodeID);
        Qnodes.pop();
        //BFS on the tree using Qnodes as the queue for memorizing the future nodes to be traversed
        if(!currentNode->leafnode)
        for (FLAT::uint64 child = 0; child < currentNode->entries.size(); ++child)
        {
                Qnodes.push(currentNode->entries.at(child)->childIndex);
        }
        //join the internal node with all *its* leaves

        // If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
        if(currentNode->attachedObjs[0].size()==0)
                continue;

        // just to display the level of the BFS traversal
        if (verbose)
            if(lvl!=currentNode->level)
            {
                    lvl = currentNode->level;
                    cout << "\n### Level " << lvl <<endl;
            }

        joinIntenalnodetoleafs(currentNodeID);
    }
    probing.stop();
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
        for(int i = 0 ; i<Levels ; i++)
                cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;

        footprint += sum*sizeof(FLAT::SpatialObject*) + tree.size()*(sizeof(TreeNode*));
        avg = (sum+0.0) / (tree.size());
        percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
        differenceSquared = ((double)sqsum/(double)tree.size())-avg*avg;
        std = sqrt(differenceSquared);
        analyzing.stop();
}