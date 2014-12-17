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
    
    for (int t = 0; t < TYPES; t++)
    {
        levelAssigned[t].resize(10,0);
        levelAvg[t].resize(10,0);
        levelStd[t].resize(10,0);
    }
}

CommonTOUCH::~CommonTOUCH() {
}

void CommonTOUCH::saveLog() {
    
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
        << "t loading, t init, t build, t probe, t comparing, t partition, t total, t deDuplicating, t analyzing, t sorting, t gridCalculate, t sizeCalculate,"
        << "EmptyCells(%), MaxObj, AveObj, StdObj, repA, repB, max level, gridP robe, tree height A, tree height B,"
        << "l0 assigned, l1 assigned, l2 assigned, l3 assigned, l4 assigned, l5 assigned, l6 assigned, l7 assigned, l8 assigned, l9 assigned,"
        << "l0 assigned B, l1 assigned B, l2 assigned B, l3 assigned B, l4 assigned B, l5 assigned B, l6 assigned B, l7 assigned B, l8 assigned B, l9 assigned B,"
        << "l0 avg, l1 avg, l2 avg, l3 avg, l4 avg, l5 avg, l6 avg, l7 avg, l8 avg, l9 avg,"
        << "l0 avg B, l1 avg B, l2 avg B, l3 avg B, l4 avg B, l5 avg B, l6 avg B, l7 avg B, l8 avg B, l9 avg B,"
        << "l0 std, l1 std, l2 std, l3 std, l4 std, l5 std, l6 std, l7 std, l8 std, l9 std, "
        << "l0 std B, l1 std B, l2 std B, l3 std B, l4 std B, l5 std B, l6 std B, l7 std B, l8 std B, l9 std B"
        << "\n";
    }
    //check if file exists
    
    FLAT::Timer t;
    t.add(probing);
    t.add(gridCalculate);
            
    fout
    << algoname() << "," << epsilon << "," << size_dsA << "," << size_dsB << "," << file_dsA << "," << file_dsB << ","
    << basealgo() << "," << nodesize << "," << leafsize << "," << localPartitions << ","
            
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
            << sizeCalculate << ","
            << percentageEmpty << ","
            << maxMappedObjects << "," 
    << avg << "," 
            << std << "," 
            << repA << ","
            << repB << ","
            << maxLevelCoef << ","
            << t << ","
            << Levels << ","
            << LevelsD << ",";
    for (int t = 0; t < TYPES; t++)
        for (int i = 0; i < 10; i++)
            fout << levelAssigned[t][i] << ",";
    
    
    for (int t = 0; t < TYPES; t++)
        for (int i = 0; i < 10; i++)
            fout << levelAvg[t][i] << ",";
    
    
    for (int t = 0; t < TYPES; t++)
        for (int i = 0; i < 10; i++)
            fout << levelStd[t][i] << ",";
    
            fout << "\n";

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

void CommonTOUCH::countSizeStatistics()
{
    //put Ans and not Ans together!
    
    sizeCalculate.start();
    FLAT::Box mbr;
    for (std::vector<TreeNode*>::iterator it = tree.begin(); it != tree.end(); it++)
    { 
        for (int type = 0; type < TYPES; type++)
        {
            (*it)->avrSize[type] = 0;
            (*it)->stdSize[type] = 0;
            for (SpatialObjectList::iterator oBit = (*it)->attachedObjs[type].begin();
                    oBit != (*it)->attachedObjs[type].end(); oBit++)
            {
                double v = FLAT::Box::volume((*oBit)->getMBR());
                (*it)->avrSize[type] += v;
                (*it)->stdSize[type] += v*v;  
            }
            for (SpatialObjectList::iterator oBit = (*it)->attachedObjsAns[type].begin();
                    oBit != (*it)->attachedObjsAns[type].end(); oBit++)
            {
                double v = FLAT::Box::volume((*oBit)->getMBR());
                (*it)->avrSize[type] += v;
                (*it)->stdSize[type] += v*v;  
            }
        }
    }
    sizeCalculate.stop();
}

void CommonTOUCH::countSpatialGrid()
{
    gridCalculate.start();
    FLAT::Box mbr;
    for (std::vector<TreeNode*>::iterator it = tree.begin(); it != tree.end(); it++)
    { 
        for (int type = 0; type < TYPES; type++)
        {
            if (algorithm == algo_TOUCH || algorithm == algo_dTOUCH)
            {
                mbr = (*it)->parentEntry->mbrL[type];
            } else {
                mbr = (*it)->parentEntry->mbrSelfD[type];
            }
            
            
            //temporary ignore global localPartition. if success - remove it from parameters
            
            //resolution is number of cells per dimension
            int resolution;
            double spaceVol = FLAT::Box::volume(mbr);
            if ((*it)->avrSize[type] == 0)
            {
                resolution = 100;
            }
            else
            {
                resolution = (int) std::pow(spaceVol/(*it)->avrSize[type],1./3.);
            }
            
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



void CommonTOUCH::analyze()
{
    countSizeStatistics(); // must be before analysis to count average sizes

    analyzing.start();
    FLAT::uint64 emptyCells;
    FLAT::uint64 sum=0, sqsum=0;
    double differenceSquared=0;
    //footprint += vdsA.size()*(sizeof(TreeEntry*));
    //footprint += dsB.size()*(sizeof(FLAT::SpatialObject*));
    FLAT::uint64 cursum;
    vector<FLAT::uint64> ItemPerLevel[TYPES]; 
    vector<FLAT::uint64> ItemPerLevelAns[TYPES]; 
    
    for (int type = 0; type < TYPES; type++)
    {
        ItemPerLevel[type].resize(Levels,0);
        ItemPerLevelAns[type].resize(Levels,0);
        emptyCells = 0;
        
        for(unsigned int ni = 0; ni<tree.size() ; ni++)
        {
                cursum = tree.at(ni)->attachedObjs[type].size() + tree.at(ni)->attachedObjsAns[type].size();
                if (cursum==0) emptyCells++;
                ItemPerLevel[type][tree.at(ni)->level] += tree.at(ni)->attachedObjs[type].size();
                ItemPerLevelAns[type][tree.at(ni)->level] += tree.at(ni)->attachedObjsAns[type].size();
                sum += cursum;
                sqsum += cursum*cursum;
                if (maxMappedObjects < cursum) maxMappedObjects = cursum; // save maximum assigned objects
                if (tree.at(ni)->level < 10)
                {
                    levelAvg[type][tree.at(ni)->level] += tree.at(ni)->avrSize[type];
                    levelStd[type][tree.at(ni)->level] += tree.at(ni)->stdSize[type];
                }
        }
        if (verbose)
            for(int i = 0 ; i<Levels; i++)
                std::cout<< "Type " << type << " Level " << i << ": Items " << ItemPerLevel[type][i] 
                        <<  " Ans " << ItemPerLevelAns[type][i] << endl;


        int top10Level = (Levels>10)?10:Levels;
        for(int i = 0 ; i<top10Level ; i++)
        {
            levelAssigned[type][i] = ItemPerLevel[type][i]+ItemPerLevelAns[type][i];
            if (levelAssigned[type][i] != 0)
            {
                levelAvg[type][i] /= (double)levelAssigned[type][i];
                levelStd[type][i] = sqrt(levelStd[type][i]/(double)levelAssigned[type][i] - levelAvg[type][i]*levelAvg[type][i]);
            }
        }
        
    }
    
    /*
     * Size statistics counts for each type and each cell. So, there is total
     * 2*tree.size cells with objects (for each type)
     */
    
    //footprint += sum*sizeof(FLAT::SpatialObject*) + tree.size()*(sizeof(TreeNode*));
    avg = (sum+0.0) / (TYPES*tree.size());
    percentageEmpty = (emptyCells+0.0) / (TYPES*tree.size())*100.0;
    differenceSquared = ((double)sqsum/(TYPES*(double)tree.size()))-avg*avg;
    std = sqrt(differenceSquared);
    
    analyzing.stop();

}