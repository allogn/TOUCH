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
    maxAssignmentLevel = 1;
    
    total.start(); // timing
}

dTOUCH::~dTOUCH()
{
    delete &treeA;
    delete &treeB;
    total.stop();
}

void dTOUCH::writeNode(vector<TreeEntry*> objlist,int Level,vector<TreeNode*>& tree)
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
}

void dTOUCH::createTreeLevel(vector<TreeEntry*>& input,int Level,vector<TreeNode*>& tree)
{
    unsigned int nodeSize;
    FLAT::uint64 itemsD1;
    FLAT::uint64 itemsD2;
    FLAT::uint64 i;
    
    if (Level==0) nodeSize = leafsize;
    else nodeSize = nodesize;
    
    sorting.start();
    switch (PartitioningType)
    {
        case Hilbert_Sort:
            std::sort(input.begin(),input.end(),ComparatorHilbert());
            break;
        case STR_Sort:
            cout<< "Node size " << nodeSize << endl;
            itemsD1 = nodeSize * nodeSize;
            itemsD2 = nodeSize;
            std::sort(input.begin(),input.end(),Comparator());
            i=0;
            while(true)
            {
                    if((i+1)*itemsD1 < input.size())
                            std::sort(input.begin()+i*itemsD1, input.begin()+(i+1)*itemsD1 ,ComparatorY());
                    else
                    {
                            std::sort(input.begin()+i*itemsD1, input.end() ,ComparatorY());
                            break;
                    }
                    i++;
            }
            i=0;
            while(true)
            {
                    if((i+1)*itemsD2 < input.size())
                            std::sort(input.begin()+i*itemsD2, input.begin()+(i+1)*itemsD2 ,ComparatorZ());
                    else
                    {
                            std::sort(input.begin()+i*itemsD2, input.end() ,ComparatorZ());
                            break;
                    }
                    i++;
            }
        case No_Sort:
            break;
        default:
            std::sort(input.begin(),input.end(),Comparator());
            break;
    }
    sorting.stop();

    cout << "Sort "<< input.size()<< " items in " << sorting << endl;
    
    vector<TreeEntry*> entries;
    for (vector<TreeEntry*>::iterator it=input.begin();it!=input.end();++it)
    {
            if (entries.size()<nodeSize)
            {
                    entries.push_back(*it);
                    if (entries.size()>=nodeSize)
                    {
                            writeNode(entries,Level,tree);
                            entries.clear();
                    }
            }
    }
    if (!entries.empty())
            writeNode(entries,Level,tree);
}

void dTOUCH::createPartitionsA()
{
        partition.start();
        // Build PRtree levels from bottom up
        LevelsA = 0;
        totalnodes = 0;
        while(vdsA.size()>1)
        {
                cout << "Tree Level: " << LevelsA << " treeNodes: " << tree.size() << " Remaining Input: " << vdsA.size() <<endl;
                createTreeLevel(vdsA,LevelsA++,treeA);      // writes final nodes in tree and next level in nextInput
                swap(vdsA,nextInput);					// swap input and nextInput list
                nextInput.clear();
        }

        rootA = vdsA.front();

        cout << "LevelsA " << LevelsA << endl;

        partition.stop();
}
void dTOUCH::createPartitionsB()
{
        partition.start();
        LevelsB = 0;
        
        if (vdsB.size() == 0)
        {
            cout << "Error! empty B set" << endl; //@todo not exception really
        }
        
        while(vdsB.size()>1)
        {
                cout << "Tree Level: " << LevelsB << " treeNodes: " << tree.size() << " Remaining Input: " << vdsB.size() <<endl;
                createTreeLevel(vdsB,LevelsB++,treeB);      // writes final nodes in tree and next level in nextInput
                swap(vdsB,nextInput);					// swap input and nextInput list
                nextInput.clear();
        }

        rootB = vdsB.front();

        cout << "LevelsB " << LevelsB << endl;

        partition.stop();
}

void dTOUCH::assignmentA()
{
    building.start();
    int overlaps;
    int assigned_level;
    bool assigned;
    
    for (unsigned int i=0;i<dsB.size();++i)
    {
        FLAT::SpatialObject* obj = dsB.at(i);
        FLAT::Box objMBR = obj->getMBR();
        objMBR.isEmpty = false;

        TreeEntry* nextNode;
        TreeNode* ptr = treeA.at(rootA->childIndex);

        while(true)
        {
            overlaps = false;
            assigned = false;
            for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
            {    
                if ( FLAT::Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) ) //@todo safe if cTOUCH
                {
                    if(overlaps++ == 0)
                    {
                        overlaps = true;
                        nextNode = ptr->entries.at(cChild);
                    }
                    else
                    {
                        //should be assigned to this level
                        if(ptr->level > maxAssignmentLevel)
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
                }
            }
            if(assigned)
            {
                obj->cost = overlaps*((pow(nodesize,assigned_level+1)-1)/(nodesize - 1) - 1);
                break;
            }
            if(!overlaps)
            {
                    //filtered
                    filtered ++;
                    break;
                }
                ptr = treeA.at(nextNode->childIndex);
                if(ptr->leafnode /*|| ptr->level < 2*/)
                {
                    //intersects only with one entry in the leaf
                    ptr->attachedObjs[0].push_back(obj);
                    obj->cost = 1; // one object only.
                    break;
                }
        }
    }
}

void dTOUCH::assignmentB()
{
    building.start();
    int overlaps;
    int assigned_level;
    bool assigned;
    
    for (unsigned int i=0;i<dsA.size();++i)
    {
        FLAT::SpatialObject* obj = dsA.at(i);
        FLAT::Box objMBR = obj->getMBR();
        objMBR.isEmpty = false;

        TreeEntry* nextNode;
        TreeNode* ptr = treeB.at(rootB->childIndex);

        while(true)
        {
            overlaps = false;
            assigned = false;
            for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
            {    
                if ( FLAT::Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) ) //@todo safe if cTOUCH
                {
                    if(overlaps++ == 0)
                    {
                        overlaps = true;
                        nextNode = ptr->entries.at(cChild);
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
            if(assigned)
            {
                obj->cost = overlaps*((pow(nodesize,assigned_level+1)-1)/(nodesize - 1) - 1);
                break;
            }
            if(!overlaps)
            {
                    //filtered
                    filtered ++;
                    break;
                }
                ptr = treeB.at(nextNode->childIndex);
                if(ptr->leafnode /*|| ptr->level < 2*/)
                {
                    //intersects only with one entry in the leaf
                    ptr->attachedObjs[0].push_back(obj);
                    obj->cost = 1; // one object only.
                    break;
                }
        }
    }
    building.stop();
}


void dTOUCH::joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID, vector<TreeNode*>& tree, TreeEntry* root)
{
        SpatialGridHash* spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
        queue<FLAT::uint64> leaves;
        TreeNode* leaf, *ancestorNode;
        ancestorNode = tree.at(ancestorNodeID);
        if( localJoin == algo_SGrid )// && localPartitions < internalObjsCount)// && internal->level >0)
        {
                //constructing the grid for the current internal node that we want to join it with all its desendet leaf nodes
                //spatialGridHash = new SpatialGridHash(localUniverse,localPartitions);
                resultPairs.deDuplicateTime.start();
                //spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
                spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
                spatialGridHash->build(ancestorNode->attachedObjs[0]);
                resultPairs.deDuplicateTime.stop();
        }

        leaves.push(ancestorNodeID);
        while(leaves.size()>0)
        {
                leaf = tree.at(leaves.front());
                leaves.pop();
                if(leaf->leafnode)
                {
                        comparing.start();
                        // join leaf->entries and vect

                        if(localJoin == algo_SGrid)// && localPartitions < internalObjsCount)// && internal->level >0)
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
}
//Optimized probing function
void dTOUCH::probe()
{
        //For every cell of A join it with its corresponding cell of B
        probing.start();

        //one tree
        std::queue<FLAT::uint64> Qnodes;

        TreeNode* currentNode;

        Qnodes.push(rootA->childIndex);
        //uni.push(root->mbr);

        int lvl = LevelsA;
        // A BFS on the tree then for each find all its leaf nodes by another BFS
        while(Qnodes.size()>0)
        {
                FLAT::uint64 currentNodeID = Qnodes.front();
                currentNode = treeA.at(currentNodeID);
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
                if(lvl!=currentNode->level)
                {
                        lvl = currentNode->level;
                        cout << "\n### Level " << lvl <<endl;
                }

                joinIntenalnodetoleafs(currentNodeID, treeA, rootA);

                //if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
        }

        //second tree
        //Qnodes is empty

        Qnodes.push(rootB->childIndex);

        lvl = LevelsB;
        // A BFS on the tree then for each find all its leaf nodes by another BFS
        while(Qnodes.size()>0)
        {
                FLAT::uint64 currentNodeID = Qnodes.front();
                currentNode = treeB.at(currentNodeID);
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
                if(lvl!=currentNode->level)
                {
                        lvl = currentNode->level;
                        cout << "\n### Level " << lvl <<endl;
                }

                joinIntenalnodetoleafs(currentNodeID, treeB, rootB);

                //if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
                //	delete spatialGridHash;
        }

        probing.stop();

}
void dTOUCH::analyze()
{

        analyzing.start();
        FLAT::uint64 emptyCells=0;
        FLAT::uint64 sum=0,sqsum=0;
        double differenceSquared=0;
        footprint += vdsA.size()*(sizeof(TreeEntry*));
        footprint += dsB.size()*(sizeof(FLAT::SpatialObject*));
        LVLA = LevelsA;
        LVLB = LevelsB;
        //tree A
        cout << "== TREE A (B assigned) ==" << endl;
        ItemPerLevelA.reserve(LevelsA);
        for(int i = 0 ; i<LevelsA ; i++)
                ItemPerLevelA.push_back(0);
        for(unsigned int ni = 0; ni<treeA.size() ; ni++)
        {
                SpatialObjectList objs = treeA.at(ni)->attachedObjs[0];
                FLAT::uint64 ptrs = objs.size();
                if(objs.size()==0)emptyCells++;
                ItemPerLevelA[treeA.at(ni)->level]+=ptrs;
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

        }
        for(int i = 0 ; i<LevelsA ; i++)
                cout<< "level " << i << " items " << ItemPerLevelA[i] <<endl;

        cout << "== TREE B (A assigned) ==" << endl; //@todo
        ItemPerLevelB.reserve(LevelsB);
        for(int i = 0 ; i<LevelsB ; i++)
                ItemPerLevelB.push_back(0);
        for(unsigned int ni = 0; ni<treeB.size() ; ni++)
        {
                SpatialObjectList objs = treeB.at(ni)->attachedObjs[0];
                FLAT::uint64 ptrs = objs.size();
                if(objs.size()==0)emptyCells++;
                ItemPerLevelB[treeB.at(ni)->level]+=ptrs;
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

        }
        for(int i = 0 ; i<LevelsB ; i++)
                cout<< "level " << i << " items " << ItemPerLevelB[i] <<endl;

        footprint += sum*sizeof(FLAT::SpatialObject*) + treeB.size()*(sizeof(TreeNode*)) + treeA.size()*(sizeof(TreeNode*));
        avg = (sum+0.0) / (treeB.size() + treeA.size());
        percentageEmpty = (emptyCells+0.0) / (treeB.size() + treeA.size())*100.0;
        differenceSquared = ((double)sqsum/((double)treeB.size() + (double)treeA.size()))-avg*avg;
        std = sqrt(differenceSquared);


        analyzing.stop();
}



void dTOUCH::printTOUCH() {
    
    print(); // print statistics of JoinAlgorithm
    
    ofstream fout(logfilename.c_str(),ios_base::app);
    
    //// A tree
    FLAT::uint64 comparisons = 0;
    for(int i = 0 ; i<LVLA ; i++)
        comparisons += pow(base,i)*(leafsize*ItemPerLevelA[i]);
    
    fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
    100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)"
        << " sortType " << PartitioningType <<
        " Filtered " <<	filtered << "(#) = " << 100*(double)filtered/(double)size_dsB << "(%) Level";

    for(int i = 0 ; i<LVLA ; i++)
        fout << " " << i << " : " << ItemPerLevelA[i] << "(#) = " 
             <<  100*(double)ItemPerLevelA[i]/(double)size_dsA << "(%)";
    
        //// B tree
    comparisons = 0;
    for(int i = 0 ; i<LVLB ; i++)
        comparisons += pow(base,i)*(leafsize*ItemPerLevelB[i]);
    
    fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
    100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)"
        << " sortType " << PartitioningType <<
        " Filtered " <<	filtered << "(#) = " << 100*(double)filtered/(double)size_dsB << "(%) Level";

    for(int i = 0 ; i<LVLB ; i++)
        fout << " " << i << " : " << ItemPerLevelB[i] << "(#) = " 
             <<  100*(double)ItemPerLevelB[i]/(double)size_dsA << "(%)";
    
    
    fout.close();
}