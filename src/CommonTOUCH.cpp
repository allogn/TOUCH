/* 
 * File:   CommonTOUCH.cpp
 * Author: Alvis
 * 
 */

#include "CommonTOUCH.h"

CommonTOUCH::CommonTOUCH() {
    localPartitions = 100;
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

/*
 * ancestorNode is node where obj is assigned.
 * First ancestor node is not checked, go through children
 */
void CommonTOUCH::joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode)
{
    queue<TreeNode*> nodes;
    nodes.push(ancestorNode);
    TreeNode* node;
    while(nodes.size()>0)
    {
        //start from checking children, each for intersection of MBR
        // then if intersects - check the assign objects of child
        // and if it is not a leaf node and intersects -> add to the queue

        node = nodes.front();
        nodes.pop();

        if (node->leafnode == true)
            continue;

        //intersect with all non-null children
        for (NodeList::iterator it = node->entries.begin(); it != node->entries.end(); it++)
        {
            //if intersects
            ItemsMaxCompared += (*it)->attachedObjs[!obj->type].size();
            comparing.start();
            if(localJoin == algo_SGrid)
            {
                (*it)->spatialGridHash[!obj->type]->probe(obj);
            }
            else
            {
                NL(obj, (*it)->attachedObjs[!obj->type]);
            }
            comparing.stop();
            if (FLAT::Box::overlap(obj->mbr, (*it)->mbr))
            {
                nodes.push((*it));
            } 
        }

    }
}

void CommonTOUCH::joinNodeToDesc(TreeNode* node)
{
    /*
     * A -> B_below
     */
    for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                    it != node->attachedObjs[0].end(); it++)
    {
        joinObjectToDesc((*it), node);
    }

    /*
     * B -> A_below
     */
    for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                                    it != node->attachedObjs[1].end(); it++)
    {
        joinObjectToDesc((*it), node);
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
                ItemsMaxCompared += node->attachedObjs[1].size();                                
                comparing.start();
                if ((algorithm == algo_dTOUCH) && (FLAT::Box::overlap((*it)->mbr, node->mbr)))
                    NL((*it), node->attachedObjs[1]);
                else
                    if (FLAT::Box::overlap((*it)->mbr, node->mbrSelfD[1]))
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
                ItemsMaxCompared += node->attachedObjs[0].size();
                comparing.start();
                if ((algorithm == algo_dTOUCH) && (FLAT::Box::overlap((*it)->mbr, node->mbr)))
                    NL((*it), node->attachedObjs[0]);
                else
                    if (FLAT::Box::overlap((*it)->mbr, node->mbrSelfD[0]))
                        NL((*it), node->attachedObjs[0]);
                comparing.stop();
            }
        }
    }
}

void CommonTOUCH::probe()
{
    probing.start();

    std::queue<TreeNode*> Qnodes;
    TreeNode* currentNode;
    Qnodes.push(root);

    int lvl = Levels;
    // A BFS on the tree then for each find all its leaf nodes by another BFS
    while(Qnodes.size()>0)
    {
        currentNode = Qnodes.front();
        Qnodes.pop();
        //BFS on the tree using Qnodes as the queue for memorizing the future nodes to be traversed
        if(!currentNode->leafnode)
            for (NodeList::iterator it = currentNode->entries.begin(); it != currentNode->entries.end(); it++)
            {
                    Qnodes.push((*it));
            }
        
        // just to display the level of the BFS traversal
        if (verbose)
            if(lvl!=currentNode->level)
            {
                    lvl = currentNode->level;
                    cout << "\n### Level " << lvl <<endl;
            }
        
        // If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
       
        if(currentNode->attachedObjs[0].size() + currentNode->attachedObjs[1].size()
                + currentNode->attachedObjsAns[0].size() + currentNode->attachedObjsAns[1].size() ==0)
            continue;
        joinNodeToDesc(currentNode);
    }
    probing.stop();
}

void CommonTOUCH::countSizeStatistics()
{
    //put Ans and not Ans together!
    
    sizeCalculate.start();
    FLAT::Box mbr;
    for (NodeList::iterator it = tree.begin(); it != tree.end(); it++)
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
    for (NodeList::iterator it = tree.begin(); it != tree.end(); it++)
    { 
        for (int type = 0; type < TYPES; type++)
        {
            if (algorithm == algo_TOUCH || algorithm == algo_dTOUCH)
            {
                mbr = (*it)->mbrL[type];
            } else {
                mbr = (*it)->mbrSelfD[type];
            }
            
            
            //temporary ignore global localPartition. if success - remove it from parameters
            
            //resolution is number of cells per dimension
            int resolution;
            double spaceVol = FLAT::Box::volume(mbr);
            if ((*it)->avrSize[type] == 0)
            {
                resolution = 1;
            }
            else
            {
                resolution = (int) std::pow(spaceVol/(*it)->avrSize[type],1./3.);
            }
            
            
            if (this->algorithm == algo_dTOUCH && !(*it)->leafnode)
                continue;
            
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
    for (NodeList::iterator it = tree.begin(); it != tree.end(); it++)
    {
        for (int type = 0; type < TYPES; type++)
        {
            if (this->algorithm == algo_dTOUCH && (*it)->leafnode && type == 0)
            {
                (*it)->spatialGridHash[type]->resultPairs.deDuplicate();
                SpatialGridHash::transferInfo((*it)->spatialGridHash[type], this);
                continue;
            }

            (*it)->spatialGridHash[type]->resultPairs.deDuplicate();
            SpatialGridHash::transferInfo((*it)->spatialGridHash[type], this);

            if (this->algorithm == algo_reTOUCH || this->algorithm == algo_rereTOUCH)
            {
                (*it)->spatialGridHashAns[type]->resultPairs.deDuplicate();
                SpatialGridHash::transferInfo((*it)->spatialGridHashAns[type], this);
            }
        }
    }
}

void CommonTOUCH::writeNode(SpatialObjectList& objlist)
{
    TreeNode* prNode = new TreeNode(0);
    FLAT::Box mbr;
    totalnodes++;
    
    for (SpatialObjectList::iterator it=objlist.begin(); it!=objlist.end(); ++it)
    {
        prNode->attachedObjs[(*it)->type].push_back(*it);
        mbr = FLAT::Box::combineSafe((*it)->mbr,mbr);
    }
    prNode->mbr = mbr;
    prNode->mbrL[0] = mbr;
    
    tree.push_back(prNode);
    nextInput.push_back(prNode);
}

void CommonTOUCH::writeNode(NodeList& nodelist, int Level)
{
    TreeNode* prNode = new TreeNode(Level);
    FLAT::Box mbr;
    totalnodes++;
    
    for (NodeList::iterator it=nodelist.begin(); it!=nodelist.end(); ++it)
    {
            prNode->entries.push_back(*it);
            mbr = FLAT::Box::combineSafe((*it)->mbr,mbr);
    }
    prNode->mbr = mbr;
    prNode->mbrL[0] = mbr;
    
    tree.push_back(prNode);
    nextInput.push_back(prNode);
}

void CommonTOUCH::createPartitions(SpatialObjectList& vds)
{
    partition.start();

    Levels = 1;
    totalnodes = 0;
    
    createTreeLevel(vds);
    if (verbose) std::cout << "Tree leafs sorted." << std::endl;
    NodeList nds;
    swap(nds,nextInput);
    nextInput.clear();
    while(nds.size()>1)
    {
        if (verbose) 
            std::cout << "Tree Level: " << Levels << " Tree Nodes: " << tree.size() 
                      << " Remaining Input: " << nds.size() <<endl;
            createTreeLevel(nds,Levels++);      // writes final nodes in tree and next level in nextInput
            swap(nds,nextInput);					// swap input and nextInput list
            nextInput.clear();
    }
    
    root = nds.front();
    root->root = true;
    if (verbose) std::cout << "Levels " << Levels << std::endl;
    partition.stop();
}

void CommonTOUCH::createTreeLevel(SpatialObjectList& input)
{
    sorting.start();
    switch (PartitioningType)
    {
        case Hilbert_Sort:
            thrust::sort(input.begin(),input.end(),ComparatorHilbertEntry());
            break;
        case No_Sort:
            break;
        default:
            thrust::sort(input.begin(),input.end(),ComparatorEntry());
            break;
    }
    sorting.stop();

    if (verbose) std::cout << "Sort "<< input.size()<< " leaf objects in " << sorting << std::endl;
    
    SpatialObjectList entries;
    for (SpatialObjectList::iterator it=input.begin();it!=input.end();++it)
    {
            if (entries.size()<leafsize)
            {
                    entries.push_back(*it);
                    if (entries.size()>=leafsize)
                    {
                            writeNode(entries);
                            entries.clear();
                    }
            }
    }
    if (!entries.empty())
            writeNode(entries);
}

void CommonTOUCH::createTreeLevel(NodeList& input, int Level)
{
    
    sorting.start();
    switch (PartitioningType)
    {
        case Hilbert_Sort:
            thrust::sort(input.begin(),input.end(),ComparatorHilbert());
            break;
        case No_Sort:
            break;
        default:
            thrust::sort(input.begin(),input.end(),Comparator());
            break;
    }
    sorting.stop();

    if (verbose) std::cout << "Sort "<< input.size()<< " items in " << sorting << std::endl;
    
    NodeList entries;
    for (NodeList::iterator it=input.begin();it!=input.end();++it)
    {
            if (entries.size()<nodesize)
            {
                    entries.push_back(*it);
                    if (entries.size()>=nodesize)
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
    thrust::host_vector<FLAT::uint64> ItemPerLevel[TYPES]; 
    thrust::host_vector<FLAT::uint64> ItemPerLevelAns[TYPES]; 
    
    for (int type = 0; type < TYPES; type++)
    {
        ItemPerLevel[type].resize(Levels,0);
        ItemPerLevelAns[type].resize(Levels,0);
        emptyCells = 0;
        
        for(NodeList::iterator ni = tree.begin(); ni != tree.end(); ni++)
        {
                cursum = (*ni)->attachedObjs[type].size() + (*ni)->attachedObjsAns[type].size();
                if (cursum==0) emptyCells++;
                ItemPerLevel[type][(*ni)->level] += (*ni)->attachedObjs[type].size();
                ItemPerLevelAns[type][(*ni)->level] += (*ni)->attachedObjsAns[type].size();
                sum += cursum;
                sqsum += cursum*cursum;
                if (maxMappedObjects < cursum) maxMappedObjects = cursum; // save maximum assigned objects
                if ((*ni)->level < 10)
                {
                    levelAvg[type][(*ni)->level] += (*ni)->avrSize[type];
                    levelStd[type][(*ni)->level] += (*ni)->stdSize[type];
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