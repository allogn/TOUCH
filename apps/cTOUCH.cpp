/* 
 * File:   cTOUCH.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 22:55
 */

#include "cTOUCH.h"

void cTOUCH::analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
{

        analyzing.start();
        uint64 emptyCells=0;
        uint64 sum=0,sqsum=0;
        double differenceSquared=0;
        footprint += vdsAll.size()*(sizeof(TreeEntry*));
        footprint += dsB.size()*(sizeof(SpatialObject*));
        LVL = Levels;
        //vector<uint64> ItemPerLevel;
        ItemPerLevel.reserve(Levels);
        for(int i = 0 ; i<Levels ; i++)
                ItemPerLevel.push_back(0);
        for(unsigned int ni = 0; ni<tree.size() ; ni++)
        {
                SpatialObjectList objsA = tree.at(ni)->attachedObjs[0];
                SpatialObjectList objsB = tree.at(ni)->attachedObjs[1];
                uint64 ptrs = objsA.size() + objsB.size();
                if(objsA.size() + objsB.size()==0)emptyCells++;
                ItemPerLevel[tree.at(ni)->level]+=ptrs;
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (max<ptrs) max = ptrs;

        }
        for(int i = 0 ; i<Levels ; i++)
                cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;

        footprint += sum*sizeof(SpatialObject*) + tree.size()*(sizeof(TreeNode*));
        avg = (sum+0.0) / (tree.size());
        percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
        differenceSquared = ((double)sqsum/(double)tree.size())-avg*avg;
        std = sqrt(differenceSquared);
        analyzing.stop();

}

void cTOUCH::probe()
{
        //For every cell of A join it with its corresponding cell of B
        probing.start();
        queue<uint64> Qnodes;

        TreeNode* currentNode;
        Box localUniverse;

        Qnodes.push(root->childIndex);

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
                                 //@todo - do not delete them above, only make black box empty
                                if (!(currentNode->entries.at(child)->mbrK[0].isEmpty))
                                {
                                        Qnodes.push(currentNode->entries.at(child)->childIndex);
                                }
                        }

                        //Join assigned to current node objects to all objects
                        //assigned below to dark nodes

                // If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
                if(currentNode->attachedObjs[0].size() + currentNode->attachedObjs[1].size()==0)
                        continue;

                // just to display the level of the BFS traversal
                if(lvl!=currentNode->level)
                {
                        lvl = currentNode->level;
                        cout << "\n### Level " << lvl << "; Enter childIndex: " << currentNodeID << endl;
                }

                joinIntenalnodetoleafs(currentNodeID); //join node -> join each object -> join object to down tree -> join object with list of objects

                //if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
                //	delete spatialGridHash;
        }
        probing.stop();

}

/*
Create new node according to set of TreeEntries. Entries can be of both types,
So create to entries that point to the new node of two types.
Create entry iff it is not empty
*/
void cTOUCH::writeNode(vector<TreeEntry*> objlist,int Level)
{
        TreeNode* prNode = new TreeNode(Level);
        Box mbrA;
        Box mbrB;
        uint64 childIndex; //pointer to Node in the tree

        childIndex = tree.size();
        totalnodes++; //only one node

        for (vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
        {
                prNode->entries.push_back(*it);


                Box::combine((*it)->mbrL[0],mbrA,mbrA);

                Box::combine((*it)->mbrL[1],mbrB,mbrB);

                (*it)->parentIndex = childIndex; //parent index of children = childIndex of new Entry				

                //clear black MBR (now is used for L&D tree (traversing during join) @todo is it necessary??

        }

        tree.push_back(prNode);
        nextInput.push_back(new TreeEntry(mbrA,mbrB,childIndex));
        prNode->parentEntry = nextInput.back();
}

void cTOUCH::createTreeLevel(vector<TreeEntry*>& input,int Level)
{
        unsigned int nodeSize;
        if (Level==0) nodeSize = leafsize;
        else nodeSize = nodesize;

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

void cTOUCH::assignment()
{
        building.start();
        queue<uint64> Qnodes;
        TreeNode* currentNode;
        Box localUniverse;

        bool overlaps;
        bool assigned;

        TreeNode* ptr;
        TreeEntry* nextNode;
        TreeEntry* ancestorEntry;
        TreeNode* ancestorNode;
        Box objMBR;
        SpatialObject* obj;

        bool canFilter; //flag that shows if current object intersects SelfMbr on some level. If so, it can not be filtered

        Qnodes.push(root->childIndex);


        //getting all leaf nodes and assigned objects to leafs
        while(Qnodes.size()>0)
        {
                uint64 currentNodeID = Qnodes.front(); //take one node. if leaf -  assign it. if not - go further
                currentNode = tree.at(currentNodeID);
                Qnodes.pop();
                if(currentNode->leafnode)
                {
                        //get all objects of one color -> assign them
                        //delete mbr of assigned objects and update tree
                        //get all objects of opposite color and do the same
                        //then delete all node from light tree
                        for (int current_type = 0; current_type <= 1; current_type++) // take two colors successively
                        {
                                for (unsigned int i=0;i<currentNode->entries.size();++i)
                                {
                                        obj = currentNode->entries.at(i)->obj;
                                        if (obj->type != current_type)
                                        {
                                                continue;
                                        }
                                        objMBR = obj->getMBR();

                                        ptr = tree.at(root->childIndex);
                                        canFilter = true;

                                        while(true) //traverse all tree from top to bottom and assign obj
                                        {
                                                overlaps = false;
                                                assigned = false;
                                                for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
                                                {

                                                        if ( Box::overlap(objMBR,ptr->entries.at(cChild)->mbrK[current_type]) ) // here mbr is dark+light
                                                        {
                                                                if(!overlaps)
                                                                {
                                                                        overlaps = true;

                                                                        if (canFilter == true && Box::overlap(objMBR, ptr->entries.at(cChild)->mbrSelfD[current_type]))
                                                                        {
                                                                                canFilter = false;
                                                                        }

                                                                        nextNode = ptr->entries.at(cChild);
                                                                }
                                                                else
                                                                {
                                                                        //should be assigned to this level
                                                                        ptr->attachedObjs[current_type].push_back(obj);

                                                                        //update the cost function of current object according to level in the tree
                                                                        obj->cost += pow(partitions,ptr->level);

                                                                        ancestorEntry = ptr->parentEntry;  // take entry for the node where we assigning

                                                                        //start updating from the first ancestor, but in the object only update SelfMBR

                                                                        Box::combine(ancestorEntry->mbrSelfD[current_type], objMBR, ancestorEntry->mbrSelfD[current_type]);
                                                                        Box::combine(ancestorEntry->mbrSelfD[current_type], ancestorEntry->mbrK[current_type], ancestorEntry->mbrK[current_type]);

                                                                        while (1)
                                                                        {
                                                                                if (ancestorEntry->parentIndex == 0)
                                                                                        break; // we are at root
                                                                                ancestorNode = tree.at(ancestorEntry->parentIndex);
                                                                                ancestorEntry = ancestorNode->parentEntry;

                                                                                Box::combine(ancestorEntry->mbrD[current_type], objMBR, ancestorEntry->mbrD[current_type]);
                                                                                Box::combine(ancestorEntry->mbrD[current_type], ancestorEntry->mbrK[current_type], ancestorEntry->mbrK[current_type]);

                                                                                //here - update cost
                                                                                for (SpatialObjectList::iterator it = ancestorNode->attachedObjs[current_type].begin();
                                                                                                it != ancestorNode->attachedObjs[current_type].end(); it++)
                                                                                {
                                                                                        (*it)->cost++;
                                                                                }

                                                                        }

                                                                        assigned = true;
                                                                        break;
                                                                }
                                                        }
                                                }

                                                if(assigned)
                                                                break;
                                                if(!overlaps && canFilter == true)
                                                {
                                                                //filtered
                                                                filtered ++;
                                                                break;
                                                }

                                                ptr = tree.at(nextNode->childIndex);

                                                if(ptr->leafnode)
                                                {
                                                                ptr->attachedObjs[obj->type].push_back(obj);
                                                                break;
                                                }
                                        }
                                }

                                //here - delete all leaf node and update light tree and black tree
                                //do not delete - only earse mbr's
                                ancestorEntry = currentNode->parentEntry;
                                while (1)
                                {
                                        if (ancestorEntry->parentIndex == 0)
                                                break; // we are at root
                                        ancestorNode = tree.at(ancestorEntry->parentIndex);
                                        ancestorEntry = ancestorNode->parentEntry;

                                        ancestorEntry->mbrL[current_type].isEmpty = true;
                                        for (vector<TreeEntry*>::iterator it=ancestorNode->entries.begin();
                                                                                                          it!=ancestorNode->entries.end(); ++it)
                                        {
                                                Box::combine((*it)->mbrL[current_type],
                                                                ancestorEntry->mbrL[current_type],ancestorEntry->mbrL[current_type]);
                                        }
                                        Box::combine(ancestorEntry->mbrD[current_type],ancestorEntry->mbrL[current_type],
                                                                                                        ancestorEntry->mbrK[current_type]); //update black
                                        Box::combine(ancestorEntry->mbrSelfD[current_type],ancestorEntry->mbrK[current_type],
                                                                                                                                                        ancestorEntry->mbrK[current_type]);
                                }

                        }


                }
                else
                {
                        for (uint64 child = 0; child < currentNode->entries.size(); ++child)
                        {
                                Qnodes.push(currentNode->entries.at(child)->childIndex);
                        }
                }
        }

        building.stop();
}

void cTOUCH::joinInternalobjecttodesc(SpatialObject* obj, uint64 ancestorNodeID)
{
        queue<uint64> nodes;
        int nodeID;
        int opType;
        TreeNode* node;
        TreeNode* downnode;
        nodes.push(ancestorNodeID);
        while(nodes.size()>0)
        {
                //start from checking children, each for intersection of MBR
                // then if intersects - check the assign objects of child
                // and if it is not a leaf node and intersects -> add to the queue



                nodeID = nodes.front();
                node = tree.at(nodeID);
                nodes.pop();

                //intersect with all non-null children
                for (uint64 child = 0; child < node->entries.size(); ++child)
                {
                        //if intersects
                        opType = (obj->type)?0:1;
                        if (Box::overlap(obj->getMBR(), node->entries.at(child)->mbrD[opType]))
                        {
                                downnode = tree.at(node->entries.at(child)->childIndex );
                                JOIN(obj, downnode->attachedObjs[opType]);

                                //add child to the queue if it is not a leaf (even in dark)
                                if (downnode->leafnode == false)
                                        nodes.push(node->entries.at(child)->childIndex);
                        }
                }

        }
}

void cTOUCH::joinIntenalnodetoleafs(uint64 ancestorNodeID)
{

        //check for intersection all nodes below, where smth was attached with opposite color.
        comparing.start();

        TreeNode* node = tree.at(ancestorNodeID);
        // here @todo check function and parallelize
        /*
         * A -> B_below
         */
        for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                        it != node->attachedObjs[0].end(); it++)
        {
                joinInternalobjecttodesc((*it), ancestorNodeID);
        }

        /*
         * B -> A_below
         */
        for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                                        it != node->attachedObjs[1].end(); it++)
        {
                joinInternalobjecttodesc((*it), ancestorNodeID);
        }

        /*
         * A -> B //@todo decide which first
         */
        if (node->attachedObjs[0].size() < node->attachedObjs[1].size())
        {
                for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                                it != node->attachedObjs[0].end(); it++)
                {
                        JOIN((*it), node->attachedObjs[1]);
                }
        }
        else
        {
                for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                        it != node->attachedObjs[1].end(); it++)
                {
                        JOIN((*it), node->attachedObjs[0]);
                }
        }


        comparing.stop();
}