/* 
 * File:   TOUCH.cpp
 * Author: Alvis
 * 
 */

#include "TOUCH.h"

TOUCH::TOUCH() {
    stats.initialize.start();

    stats.gridSize = buckets;

    leafsize = ceil((double)vdsA.size()/(double)buckets);
    nodesize = base;

    stats.initialize.stop();
}

TOUCH::TOUCH(const TOUCH& orig) {
}

TOUCH::~TOUCH() {
    delete &tree;
}

void TOUCH::writeNode(std::vector<TreeEntry*> objlist,int Level);
{
    TreeNode* prNode = new TreeNode(Level);
    FLAT::Box mbr;
    FLAT::uint64 childIndex;
    totalnodes++;
    
    for (std::vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
    {
            prNode->entries.push_back(*it);
            FLAT::Box::combine((*it)->mbr,mbr,mbr);
            count++;
    }
    childIndex = tree.size();

    tree.push_back(prNode);
    nextInput.push_back(new TreeEntry(mbr,childIndex));
}

void TOUCH::createTreeLevel(vector<TreeEntry*>& input,int Level)
{
    unsigned int nodeSize;
    if (Level==0) nodeSize = leafsize;
    else nodeSize = nodesize;

    stats.sorting.start();
    if(PartitioningType != No_Sort)
    {
            std::sort(input.begin(),input.end(),Comparator());
    }

    if(PartitioningType == STR_Sort)
    {
            //First sort by X then assigns the objects to partitions then sort by Y 
        //each partition separately then again assigns the objects to partitions then sort by Z objects of each partition separately
            cout<< "Node size " << nodeSize << endl;
            uint64 itemsD1 = nodeSize * nodeSize;
            uint64 itemsD2 = nodeSize;
            std::sort(input.begin(),input.end(),Comparator());
            uint64 i=0;
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
    }
    stats.sorting.stop();

        cout << "Sort "<< input.size()<< " items in " << stats.sorting << endl;


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

void TOUCH::createPartitions()
{
        stats.partition.start();
        // Build PRtree levels from bottom up
        Levels = 0;
        totalnodes = 0;
        while(vdsA.size()>1)
        {
                cout << "Tree Level: " << Levels << " treeNodes: " << tree.size() << " Remaining Input: " << vdsA.size() <<endl;
                createTreeLevel(vdsA,Levels++);      // writes final nodes in tree and next level in nextInput
                swap(vdsA,nextInput);					// swap input and nextInput list
                nextInput.clear();
        }

        root = vdsA.front();

        cout << "Levels " << Levels << endl;

        stats.partition.stop();
}

void TOUCH::assignment(const SpatialObjectList& ds)
{
        stats.building.start();
        bool overlaps;
        bool assigned;
        for (unsigned int i=0;i<ds.size();++i)
        {
                //if(i%1000==0)cout<<i<<endl;
                SpatialObject* obj = ds.at(i);
                Box objMBR = obj->getMBR();
                //objMBR.low = objMBR.low - epsilon;
                //objMBR.high = objMBR.high + epsilon;

                TreeEntry* nextNode;
                TreeNode* ptr = tree.at(root->childIndex);

                nextNode = NULL;

                while(true)
                {
                        overlaps = false;
                        assigned = false;
                        for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
                                if ( Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) )
                                {
                                        if(!overlaps)
                                        {
                                                overlaps = true;
                                                nextNode = ptr->entries.at(cChild);
                                        }
                                        else
                                        {
                                                //should be assigned to this level
                                                ptr->attachedObjs.push_back(obj);
                                                assigned = true;
                                                break;
                                        }
                                }

                        if(assigned)
                                break;
                        if(!overlaps)
                        {
                                //filtered
                                stats.filtered ++;
                                break;
                        }
                        ptr = tree.at(nextNode->childIndex);
                        if(ptr->leafnode /*|| ptr->level < 2*/)
                        {
                                ptr->attachedObjs.push_back(obj);
                                break;
                        }
                }
        }

        stats.building.stop();
}


void TOUCH::joinIntenalnodetoleafs(uint64 ancestorNodeID, ResultPairs& results)
{
        SpatialGridHash* spatialGridHash;
        queue<uint64> leaves;
        TreeNode* leaf, *ancestorNode;
        ancestorNode = tree.at(ancestorNodeID);
        if( localJoin == algo_SGrid )// && localPartitions < internalObjsCount)// && internal->level >0)
        {
                //constructing the grid for the current internal node that we want to join it with all its desendet leaf nodes
                //spatialGridHash = new SpatialGridHash(localUniverse,localPartitions);
                stats.deDuplicate.start();
                //spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
                spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
                spatialGridHash->build(ancestorNode->attachedObjs);
                stats.deDuplicate.stop();
        }

        leaves.push(ancestorNodeID);
        while(leaves.size()>0)
        {
                leaf = tree.at(leaves.front());
                leaves.pop();
                if(leaf->leafnode)
                {
                        stats.comparing.start();
                        // join leaf->entries and vect

                        if(localJoin == algo_SGrid)// && localPartitions < internalObjsCount)// && internal->level >0)
                        {
                                ResultList temp;
                                spatialGridHash->probe(leaf , temp);
                                stats.duplicates -= temp.size();
                                //The following or stats.results += temp.size();
                                for (ResultList::iterator it=temp.begin(); it!=temp.end(); ++it)
                                {
                                        results.addPair(it->first,it->second);
                                }
                        }
                        else
                                JOIN(leaf,ancestorNode->attachedObjs,results);

                        stats.comparing.stop();
                        //else if(PartitioningType == 2)
                                //joinVectorLeaf(leaf,hb,results);
                }
                else
                {
                        for (uint64 child = 0; child < leaf->entries.size(); ++child)
                        {
                                leaves.push(leaf->entries.at(child)->childIndex);
                        }
                }
        }
}
        
void TOUCH::probe(ResultPairs& results)
{
        //For every cell of A join it with its corresponding cell of B
        stats.probing.start();
        queue<uint64> Qnodes;

        TreeNode* currentNode;
    Box localUniverse;

    Qnodes.push(root->childIndex);
        //uni.push(root->mbr);

        int lvl = Levels;
        // A BFS on the tree then for each find all its leaf nodes by another BFS
        while(Qnodes.size()>0)
        {
                uint64 currentNodeID = Qnodes.front();
                currentNode = tree.at(currentNodeID);
                Qnodes.pop();
                //BFS on the tree using Qnodes as the queue for memorizing the future nodes to be traversed
                if(!currentNode->leafnode)
                for (uint64 child = 0; child < currentNode->entries.size(); ++child)
                {
                        Qnodes.push(currentNode->entries.at(child)->childIndex);
                }
                //join the internal node with all *its* leaves

                // If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
                if(currentNode->attachedObjs.size()==0)
                        continue;

                // just to display the level of the BFS traversal
                if(lvl!=currentNode->level)
                {
                        lvl = currentNode->level;
                        cout << "\n### Level " << lvl <<endl;
                }

                joinIntenalnodetoleafs(currentNodeID, results);

                //if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
                //	delete spatialGridHash;
        }
        stats.probing.stop();

}
	
void TOUCH::analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
{

        stats.analyzing.start();
        uint64 emptyCells=0;
        uint64 sum=0,sqsum=0;
        double differenceSquared=0;
        stats.footprint += vdsA.size()*(sizeof(TreeEntry*));
        stats.footprint += dsB.size()*(sizeof(SpatialObject*));
        LVL = Levels;
        //vector<uint64> ItemPerLevel;
        ItemPerLevel.reserve(Levels);
        for(int i = 0 ; i<Levels ; i++)
                ItemPerLevel.push_back(0);
        for(unsigned int ni = 0; ni<tree.size() ; ni++)
        {
                SpatialObjectList objs = tree.at(ni)->attachedObjs;
                uint64 ptrs = objs.size();
                if(objs.size()==0)emptyCells++;
                ItemPerLevel[tree.at(ni)->level]+=ptrs;
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (stats.max<ptrs) stats.max = ptrs;

        }
        for(int i = 0 ; i<Levels ; i++)
                cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;

        stats.footprint += sum*sizeof(SpatialObject*) + tree.size()*(sizeof(TreeNode*));
        stats.avg = (sum+0.0) / (tree.size());
        stats.percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
        differenceSquared = ((double)sqsum/(double)tree.size())-stats.avg*stats.avg;
        stats.std = sqrt(differenceSquared);
        stats.analyzing.stop();
}