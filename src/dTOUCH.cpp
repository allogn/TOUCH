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
        analyze(0);
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
            analyze(1);
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
    int assignedType = (*ds.begin())->type;
    
    for (unsigned int i=0;i<ds.size();++i)
    {
        FLAT::SpatialObject* obj = ds.at(i);
        FLAT::Box objMBR = obj->getMBR();
        objMBR.isEmpty = false;
        FLAT::Box::expand(objMBR,epsilon * 1./2.);
        TreeEntry* nextNode;
        TreeNode* ptr = tree.at(root->childIndex);
        
        while(true)
        {
            overlaps = false;
            assigned = false;
            for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
            {    
                if ( FLAT::Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) )
                {
                    if(overlaps++ == 0)
                    {
                        overlaps = true;
                        nextNode = ptr->entries.at(cChild);
                    }
                    else
                    {
                        if (assignedType == 1)
                        {
                            //should be assigned to this level
                            double coin = (rand()/(double)(RAND_MAX));

                            if (coin > exp(-(((double)ptr->level-1.) * maxLevelCoef/100.)/(double)Levels))
                            {
                                vdsB.push_back(new TreeEntry(obj));
                                vdsB.back()->expand(epsilon);
                            }
                            else
                            {
                                ptr->attachedObjs[0].push_back(obj);
                            }
                            assigned = true;
                            assigned_level = ptr->level;
                            break;
                        }
                        else
                        {
                            ptr->attachedObjs[0].push_back(obj);
                            assigned = true;
                            assigned_level = ptr->level;
                            break;
                        }
                    }
                }
            }
            if(assigned)
            {
                //obj->cost = overlaps*((pow(nodesize,assigned_level+1)-1)/(nodesize - 1) - 1);
                break;
            }
            if(!overlaps)
            {
                //filtered
                filtered[obj->type]++;
                break;
            }
            ptr = tree.at(nextNode->childIndex);
            if(ptr->leafnode /*|| ptr->level < 2*/)
            {
                ptr->attachedObjs[0].push_back(obj);
                break;
            }
        }
    }
}

void dTOUCH::analyze(int type)
{
    analyzing.start();
    FLAT::uint64 emptyCells=0;
    FLAT::uint64 sum=0,sqsum=0;
    double differenceSquared=0; 
    footprint += vdsA.size()*(sizeof(TreeEntry*));
    footprint += dsB.size()*(sizeof(FLAT::SpatialObject*));
    LVL = Levels;
    ItemPerLevel.reserve(Levels);
    for(int i = 0 ; i<Levels; i++)
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
        for(int i = 0 ; i<Levels; i++)
            cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;
    
    
    int top10Level = (Levels>10)?10:Levels;
    for(int i = 0 ; i<top10Level ; i++)
        if (type == 1)
            levelAssignedA[i] = ItemPerLevel[i];
        else
            levelAssignedB[i] = ItemPerLevel[i];

    footprint += sum*sizeof(FLAT::SpatialObject*) + tree.size()*(sizeof(TreeNode*)) + tree.size()*(sizeof(TreeNode*));
    avg = (sum+0.0) / (tree.size() + tree.size());
    percentageEmpty = (emptyCells+0.0) / (tree.size() + tree.size())*100.0;
    differenceSquared = ((double)sqsum/((double)tree.size() + (double)tree.size()))-avg*avg;
    std = sqrt(differenceSquared);


    analyzing.stop();
}

void dTOUCH::joinObjectToDesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID)
{
    queue<FLAT::uint64> nodes;
    int nodeID;

    TreeNode* node;
    nodes.push(ancestorNodeID);

    FLAT::Box objMBR = obj->getMBR();
    objMBR.isEmpty = false;
    FLAT::Box::expand(objMBR,epsilon * 1./2.);
    while(nodes.size()>0)
    {

        nodeID = nodes.front();
        node = tree.at(nodeID);
        nodes.pop();

        if (node->leafnode == true)
        {
            //intersect with all non-null children
                
            //if intersects
            ItemsMaxCompared += 1;
            comparing.start();
            if(localJoin == algo_SGrid)
            {
                node->spatialGridHash[0]->probe(obj);
            }
            else
            {
                NL(obj, node);
            }
            comparing.stop();

        }
        else
        {
            for (FLAT::uint64 child = 0; child < node->entries.size(); ++child)
            {
                if (FLAT::Box::overlap(objMBR, node->entries.at(child)->mbr))
                {
                    nodes.push(node->entries.at(child)->childIndex);
                } 
            }
        }
    }
}