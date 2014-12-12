/* 
 * File:   CommonTOUCH.cpp
 * Author: Alvis
 * 
 */

#include "CommonTOUCH.h"

CommonTOUCH::CommonTOUCH() {
    localPartitions = 100;
    nodesize = base;
    leafsize = partitions;
    PartitioningType = No_Sort;
}

CommonTOUCH::~CommonTOUCH() {
}

void CommonTOUCH::printTOUCH() {
    
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
        fout << "Algorithm, Epsilon, #A, #B, infile A, infile B, LocalJoin Alg, Fanout, Leaf size, gridSize, " // common parameters
        << "Compared #, Compared %, ComparedMax, Duplicates, Results, Selectivity, filtered A, filtered B," // TOUCH
        << "t loading, t init, t build, t probe, t comparing, t partition, t total, t deDuplicating, t analyzing, t sorting, t gridCalculate,"
        << "EmptyCells(%), MaxObj, AveObj, StdObj, repA, repB, max level, gridP robe\n";
    }
    //check if file exists
    
    FLAT::Timer t;
    t.add(probing);
    t.add(gridCalculate);
            
    fout
    << algoname << "," << epsilon << "," << size_dsA << "," << size_dsB << "," << file_dsA << "," << file_dsB << ","
    << basealgo << "," << nodesize << "," << leafsize << "," << localPartitions << ","
            
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
            << repB << ","
            << maxLevelCoef << ","
            << t << "\n";

}

void CommonTOUCH::joinObjectToDesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID)
{
    queue<FLAT::uint64> nodes;
    int nodeID;

    TreeNode* node;
    TreeNode* downnode;
    nodes.push(ancestorNodeID);

    FLAT::Box objMBR = obj->getMBR();
    objMBR.isEmpty = false;
    FLAT::Box::expand(objMBR,epsilon * 1./2.);
    while(nodes.size()>0)
    {
        //start from checking children, each for intersection of MBR
        // then if intersects - check the assign objects of child
        // and if it is not a leaf node and intersects -> add to the queue

        nodeID = nodes.front();
        node = tree.at(nodeID);
        nodes.pop();

        if (node->leafnode == true)
            continue;

        //intersect with all non-null children
        for (FLAT::uint64 child = 0; child < node->entries.size(); ++child)
        {
            downnode = tree.at(node->entries.at(child)->childIndex );

            //if intersects
            ItemsMaxCompared += downnode->attachedObjs[0].size();
            comparing.start();
            if(localJoin == algo_SGrid)
            {
                downnode->spatialGridHash[0]->probe(obj);
            }
            else
            {
                NL(obj, downnode->attachedObjs[0]);
            }
            comparing.stop();
            if (FLAT::Box::overlap(objMBR, node->entries.at(child)->mbr))
            {
                nodes.push(node->entries.at(child)->childIndex);
            } 
        }

    }
}

void CommonTOUCH::joinNodeToDesc(FLAT::uint64 ancestorNodeID)
{
    TreeNode* node = tree.at(ancestorNodeID);
    
    /*
     * A -> B_below
     */
    for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                    it != node->attachedObjs[0].end(); it++)
    {
        joinObjectToDesc((*it), ancestorNodeID);
    }
    
    if (algorithm == algo_dTOUCH)
        return;

    /*
     * B -> A_below
     */
    for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                                    it != node->attachedObjs[1].end(); it++)
    {
        joinObjectToDesc((*it), ancestorNodeID);
    }

    /*
     * A -> B
     */
    if (node->attachedObjs[0].size() < node->attachedObjs[1].size())
    {
        if(localJoin == algo_SGrid)
        {
            comparing.start();
            node->spatialGridHash[0]->probe(node->attachedObjs[1]);
            comparing.stop();
        }
        else
        {
            for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                            it != node->attachedObjs[0].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                mbr.isEmpty = false;
                FLAT::Box::expand(mbr,epsilon * 1./2.);
                ItemsMaxCompared += node->attachedObjs[1].size();
                comparing.start();
                if (FLAT::Box::overlap(mbr, node->parentEntry->mbrSelfD[1]))
                    NL((*it), node->attachedObjs[1]);
                comparing.stop();
            }
        }
        
        
    }
    else
    {
        if(localJoin == algo_SGrid)
        {
            comparing.start();
            node->spatialGridHash[1]->probe(node->attachedObjs[0]);
            comparing.stop();
        }
        else
        {
            for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                    it != node->attachedObjs[1].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                mbr.isEmpty = false;
                FLAT::Box::expand(mbr,epsilon * 1./2.);
                ItemsMaxCompared += node->attachedObjs[0].size();
                comparing.start();
                if (FLAT::Box::overlap(mbr, node->parentEntry->mbrSelfD[0]))
                {
                    NL((*it), node->attachedObjs[0]);
                }
                comparing.stop();
            }
        }
    }
}

void CommonTOUCH::probe()
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

        // If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
        if(currentNode->attachedObjs[0].size() + currentNode->attachedObjs[1].size()==0)
            continue;
        
        for (int i = 0; i < currentNode->attachedObjs[0].size(); i++)
        {
            cout << "object in the node:11 " << currentNode->attachedObjs[0][i]->id << endl;
        }

        // just to display the level of the BFS traversal
        if (verbose)
            if(lvl!=currentNode->level)
            {
                    lvl = currentNode->level;
                    cout << "\n### Level " << lvl <<endl;
            }

        joinNodeToDesc(currentNodeID);
    }
    probing.stop();
}


void CommonTOUCH::countSpatialGrid()
{
    gridCalculate.start();
    FLAT::Box mbr;
    for (std::vector<TreeNode*>::iterator it = tree.begin(); it != tree.end(); it++)
    { 
        for (int type = 0; type < TYPES; type++)
        {
            mbr = (type == 0)?universeA:universeB;
            mbr.isEmpty = false;
            
            if (this->algorithm == algo_dTOUCH && (*it)->leafnode && type==0)
            {
                (*it)->spatialGridHash[type] = new SpatialGridHash(mbr,localPartitions);
                (*it)->spatialGridHash[type]->epsilon = this->epsilon;
                (*it)->spatialGridHash[type]->build((*it)->entries);
                continue;
            }
            
            (*it)->spatialGridHash[type] = new SpatialGridHash(mbr,localPartitions);
            (*it)->spatialGridHash[type]->epsilon = this->epsilon;
            (*it)->spatialGridHash[type]->build((*it)->attachedObjs[type]);
            
            if (this->algorithm == algo_reTOUCH || this->algorithm == algo_rereTOUCH)
            {
                (*it)->spatialGridHashAns[type] = new SpatialGridHash(mbr,localPartitions);
                (*it)->spatialGridHashAns[type]->epsilon = this->epsilon;
                (*it)->spatialGridHashAns[type]->build((*it)->attachedObjsAns[type]);
            }
        }
    }
    gridCalculate.stop();
}

void CommonTOUCH::deduplicateSpatialGrid()
{
    for (std::vector<TreeNode*>::iterator it = tree.begin(); it != tree.end(); it++)
    {
        for (int type = 0; type < TYPES; type++)
        {
            if (this->algorithm == algo_dTOUCH && (*it)->leafnode && type == 0)
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
                continue;
            }

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
            }
        }
    }
}

void CommonTOUCH::writeNode(vector<TreeEntry*> objlist, int Level)
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

void CommonTOUCH::createPartitions(std::vector<TreeEntry*> vds)
{
    partition.start();

    Levels = 0;
    totalnodes = 0;
    while(vds.size()>1)
    {
        if (verbose) 
            std::cout << "Tree Level: " << Levels << " Tree Nodes: " << tree.size() 
                      << " Remaining Input: " << vds.size() <<endl;
            createTreeLevel(vds,Levels++);      // writes final nodes in tree and next level in nextInput
            swap(vds,nextInput);					// swap input and nextInput list
            nextInput.clear();
    }

    root = vds.front();
    if (verbose) std::cout << "Levels " << Levels << std::endl;
    partition.stop();
}

void CommonTOUCH::createTreeLevel(vector<TreeEntry*>& input, int Level)
{
    unsigned int nodeSize;
    
    if (Level==0) nodeSize = leafsize;
    else nodeSize = nodesize;
    
    sorting.start();
    switch (PartitioningType)
    {
        case Hilbert_Sort:
            std::sort(input.begin(),input.end(),ComparatorHilbert());
            break;
        case No_Sort:
            break;
        default:
            std::sort(input.begin(),input.end(),Comparator());
            break;
    }
    sorting.stop();

    if (verbose) std::cout << "Sort "<< input.size()<< " items in " << sorting << std::endl;
    
    vector<TreeEntry*> entries;
    for (vector<TreeEntry*>::iterator it=input.begin();it!=input.end();++it)
    {
            if (entries.size()<nodeSize)
            {
                    entries.push_back(*it);
                    if (entries.size()>=nodeSize)
                    {
                            writeNode(entries,Level);
                            entries.clear();
                    }
            }
    }
    if (!entries.empty())
            writeNode(entries,Level);
}