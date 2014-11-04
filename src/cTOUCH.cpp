/* 
 * File:   cTOUCH.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 22:55
 */

#include "cTOUCH.h"

void cTOUCH::analyze()
{

        analyzing.start();
        FLAT::uint64 emptyCells=0;
        FLAT::uint64 sum=0,sqsum=0;
        double differenceSquared=0;
        footprint += vdsAll.size()*(sizeof(TreeEntry*));
        footprint += dsB.size()*(sizeof(FLAT::SpatialObject*));
        LVL = Levels;
        //vector<FLAT::uint64> ItemPerLevel;
        ItemPerLevel.reserve(Levels);
        for(int i = 0 ; i<Levels ; i++)
                ItemPerLevel.push_back(0);
        for(unsigned int ni = 0; ni<tree.size() ; ni++)
        {
                SpatialObjectList objsA = tree.at(ni)->attachedObjs[0];
                SpatialObjectList objsB = tree.at(ni)->attachedObjs[1];
                FLAT::uint64 ptrs = objsA.size() + objsB.size();
                if(objsA.size() + objsB.size()==0)emptyCells++;
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

void cTOUCH::probe()
{
        //For every cell of A join it with its corresponding cell of B
        probing.start();
        queue<FLAT::uint64> Qnodes;

        TreeNode* currentNode;
        FLAT::Box localUniverse;

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
 * Create new node according to set of TreeEntries. Entries can be of both types,
 * So create to entries that point to the new node of two types.
 * Create entry iff it is not empty
 */
void cTOUCH::writeNode(vector<TreeEntry*> objlist,int Level)
{
        TreeNode* prNode = new TreeNode(Level);
        FLAT::Box mbrA;
        FLAT::Box mbrB;
        FLAT::uint64 childIndex; //pointer to Node in the tree

        childIndex = tree.size();
        totalnodes++; //only one node

        for (vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
        {
                prNode->entries.push_back(*it);

                mbrA = FLAT::Box::combineSafe((*it)->mbrL[0],mbrA);
                mbrB = FLAT::Box::combineSafe((*it)->mbrL[1],mbrB);

                (*it)->parentIndex = childIndex; //parent index of children = childIndex of new Entry				

                //clear black MBR (now is used for L&D tree (traversing during join) @todo is it necessary??

        }

        tree.push_back(prNode);
        nextInput.push_back(new TreeEntry(mbrA,mbrB,childIndex));
        prNode->parentEntry = nextInput.back();
        
        cout << "--" << nextInput.back()->mbrK[1] << endl;
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
    queue<FLAT::uint64> Qnodes;
    TreeNode* currentNode;

    bool overlaps;
    bool assigned;
    
    int opType; // opposite type

    TreeNode* ptr;
    TreeEntry* nextNode;
    FLAT::Box objMBR;
    FLAT::SpatialObject* obj;
    
    TreeEntry* ancestorEntry;
    TreeNode* ancestorNode;

    bool canFilter; //flag that shows if current object intersects SelfMbr on some level. If so, it can not be filtered

    Qnodes.push(root->childIndex);

    //getting all leaf nodes and assigned objects to leafs
    
    while(Qnodes.size()>0)
    {
        FLAT::uint64 currentNodeID = Qnodes.front(); //take one node. if leaf -  assign its objects. if not - go further
        currentNode = tree.at(currentNodeID);
        Qnodes.pop();
        if(currentNode->leafnode)
        {
            /* 
             * Get all objects of one color -> assign them
             * delete mbr of assigned objects and update tree
             * get all objects of opposite color and do the same
             * then delete all node from light tree
             */
            
            for (int current_type = 0; current_type <= 1; current_type++) // take two colors successively
            {
                opType = (current_type == 0)?1:0;
                /*
                 * For each object in the leaf node of type <current_type>
                 * do the assignment to the tree from the top
                 */
                for (unsigned int i=0;i<currentNode->entries.size();++i) // set type of object that must be assigned!
                {
                    obj = currentNode->entries.at(i)->obj;
                    if (obj->type != current_type)
                    {
                            continue;
                    }
                    objMBR = obj->getMBR();
                    FLAT::Box::expand(objMBR, epsilon);
                    objMBR.isEmpty = false;

                    ptr = tree.at(root->childIndex);
                    canFilter = true;

                    nextNode = root; 
                    /*
                     * Start traversing tree from the top and assign object <obj>
                     */
                    
                    while(true)
                    {
                        overlaps = false;
                        assigned = false;
                        
                        /*
                         * If <obj> intersects with one of Current Dark Mbr
                         * it can not be filtered anymore
                         */
                        if (canFilter == true && FLAT::Box::overlap(objMBR, nextNode->mbrSelfD[opType]))
                        {
                            canFilter = false;
                        }
                        
                        for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
                        {
                            
                            /*
                             * Check the intersection with black node
                             * That is combination of:
                             *  - dark current
                             *  - dark of descendants
                             *  - light current
                             */
                            if ( FLAT::Box::overlap(objMBR,ptr->entries.at(cChild)->mbrK[opType]) )
                            {
                                if(!overlaps)
                                {
                                    overlaps = true;
                                    nextNode = ptr->entries.at(cChild);
                                }
                                else
                                {
                                    assign(ptr, obj);
                                    assigned = true;
                                    break;
                                }
                            }
                        }

                        if(assigned)
                        {
                            break;
                        }
                        if(!overlaps)
                        {
                            if ( canFilter == false )
                            {
                                // @todo maybe add log that it was not filtered
                                assign(ptr, obj);
                                break;
                            }
                            //filtered
                            filtered ++;
                            break;
                        }
                        ptr = tree.at(nextNode->childIndex);

                        if(ptr->leafnode)
                        {
                            assign(ptr, obj);
                            break;
                        }
                    }
                }
                /* End of object assignment */

                //here - delete all leaf node (current type) and update light tree and black tree
                //do not delete - only earse mbr's
                ancestorEntry = currentNode->parentEntry;
                ancestorEntry->mbrL[current_type].isEmpty = true;
                while (1)
                {
                    if (ancestorEntry->parentIndex == 0)
                            break; // we are at root
                    ancestorNode = tree.at(ancestorEntry->parentIndex);
                    ancestorEntry = ancestorNode->parentEntry;
                    ancestorEntry->mbrL[current_type].isEmpty = true; // reset mbr
                    for (vector<TreeEntry*>::iterator it=ancestorNode->entries.begin(); it!=ancestorNode->entries.end(); ++it)
                    {
                        ancestorEntry->mbrL[current_type] = FLAT::Box::combineSafe((*it)->mbrL[current_type], ancestorEntry->mbrL[current_type]);
                    }
                    ancestorEntry->mbrK[current_type] = FLAT::Box::combineSafe(ancestorEntry->mbrD[current_type],ancestorEntry->mbrL[current_type]); //update black
                    ancestorEntry->mbrK[current_type] = FLAT::Box::combineSafe(ancestorEntry->mbrSelfD[current_type],ancestorEntry->mbrK[current_type]);
                }
            }
        }
        else
        {
            for (FLAT::uint64 child = 0; child < currentNode->entries.size(); ++child)
            {
                    Qnodes.push(currentNode->entries.at(child)->childIndex);
            }
        }
        
    }

    building.stop();
}
int ass = 0;
void cTOUCH::assign(TreeNode* ptr, FLAT::SpatialObject* obj)
{
    int current_type = obj->type;
    int opType = (current_type == 0)?1:0;; // opposite type
    
    TreeEntry* ancestorEntry;
    TreeNode* ancestorNode;
    
    FLAT::Box objMBR = obj->getMBR();
    FLAT::Box::expand(objMBR, epsilon);
    objMBR.isEmpty = false;
    
    //should be assigned to this level
    ptr->attachedObjs[current_type].push_back(obj);

    //update the cost function of current object according to level in the tree
    obj->cost += pow(nodesize,ptr->level);

    ancestorEntry = ptr->parentEntry;  // take entry for the node where we assigning

    //start updating from the first ancestor, but in the object only update SelfMBR

    ancestorEntry->mbrSelfD[current_type] = FLAT::Box::combineSafe(ancestorEntry->mbrSelfD[current_type], objMBR);
    ancestorEntry->mbrK[current_type] = FLAT::Box::combineSafe(ancestorEntry->mbrSelfD[current_type], ancestorEntry->mbrK[current_type]);

    /*
     * Update MBR's and costs of ancestors
     */
    while (1)
    {
        if (ancestorEntry->parentIndex == 0)
            break; // we are at root
        ancestorNode = tree.at(ancestorEntry->parentIndex);
        ancestorEntry = ancestorNode->parentEntry;

        ancestorEntry->mbrD[current_type] = FLAT::Box::combineSafe(ancestorEntry->mbrD[current_type], objMBR);
        ancestorEntry->mbrK[current_type] = FLAT::Box::combineSafe(ancestorEntry->mbrD[current_type], ancestorEntry->mbrK[current_type]);

        //here - update cost
        for (SpatialObjectList::iterator it = ancestorNode->attachedObjs[opType].begin();
            it != ancestorNode->attachedObjs[opType].end(); it++)
        {
            (*it)->cost++;
        }

    }
}

void cTOUCH::joinInternalobjecttodesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID)
{
        queue<FLAT::uint64> nodes;
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
                for (FLAT::uint64 child = 0; child < node->entries.size(); ++child)
                {
                        //if intersects
                        opType = (obj->type)?0:1;
                        if (FLAT::Box::overlap(obj->getMBR(), node->entries.at(child)->mbrD[opType]))
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

void cTOUCH::joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID)
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
         * A -> B
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