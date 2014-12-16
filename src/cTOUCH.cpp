/* 
 * File:   cTOUCH.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 22:55
 */

#include "cTOUCH.h"

void cTOUCH::run()
{
    totalTimeStart();
    readBinaryInput(file_dsA, file_dsB);
    if (verbose) std::cout << "Forming the partitions" << std::endl; 
    createPartitions(vdsAll);
    if (verbose) std::cout << "Assigning the objects of B" << std::endl; 
    assignment();
    if (verbose) std::cout << "Assigning Done." << std::endl; 
    countSizeStatistics(); // MUST BE BEFORE analyze
    analyze();
    if (verbose) std::cout << "Analysis Done, counting grids if necessary." << std::endl; 
    if(localJoin == algo_SGrid)
        countSpatialGrid();
    if (verbose) std::cout << "Probing, doing the join" << std::endl; 
    probe();
    if(localJoin == algo_SGrid)
    {
        if (verbose) std::cout << "Removing duplicates" << std::endl; 
        deduplicateSpatialGrid();
    }
    if (verbose) std::cout << "Done." << std::endl;
    totalTimeStop();
}



/*
 * Create new node according to set of TreeEntries. Entries can be of both types,
 * So create to entries that point to the new node of two types.
 * Create entry iff it is not empty
 */

void cTOUCH::writeNode(SpatialObjectList& objlist)
{
    TreeNode* prNode = new TreeNode(0);
    FLAT::Box mbr[TYPES];
    totalnodes++;
    
    for (SpatialObjectList::iterator it=objlist.begin(); it!=objlist.end(); ++it)
    {
        prNode->attachedObjs[(*it)->type].push_back(*it);
        mbr[(*it)->type] = FLAT::Box::combineSafe((*it)->mbr,mbr[(*it)->type]);
    }
    prNode->mbr = FLAT::Box::combineSafe(mbr[0],mbr[1]);
    prNode->mbrL[0] = mbr[0];
    prNode->mbrL[1] = mbr[1];
    prNode->mbrK[0] = mbr[0];
    prNode->mbrK[1] = mbr[1];
    
    tree.push_back(prNode);
    nextInput.push_back(prNode);
}

void cTOUCH::writeNode(NodeList& nodelist, int Level)
{
    TreeNode* prNode = new TreeNode(Level);
    FLAT::Box mbrA;
    FLAT::Box mbrB;
    totalnodes++;
    
    for (NodeList::iterator it=nodelist.begin(); it!=nodelist.end(); ++it)
    {
        prNode->entries.push_back(*it);
        (*it)->parentNode = prNode;
        mbrA = FLAT::Box::combineSafe((*it)->mbrL[0],mbrA);
        mbrB = FLAT::Box::combineSafe((*it)->mbrL[1],mbrB);
    }
    prNode->mbr = FLAT::Box::combineSafe(mbrA,mbrB);
    prNode->mbrL[0] = mbrA;
    prNode->mbrL[1] = mbrB;
    prNode->mbrK[0] = mbrA;
    prNode->mbrK[1] = mbrB;
    
    tree.push_back(prNode);
    nextInput.push_back(prNode);
}

void cTOUCH::assignment()
{
    building.start();
    queue<TreeNode*> Qnodes;
    queue<TreeNode*> QParentNodes;
    TreeNode* currentNode;

    bool overlaps;
    bool assigned;
    
    int opType; // opposite type

    TreeNode* ptr;
    TreeNode* nextNode;
    
    TreeNode* ancestorNode;

    bool canFilter; //flag that shows if current object intersects SelfMbr on some level. If so, it can not be filtered

    Qnodes.push(root);
    QParentNodes.push(root);

    //getting all leaf nodes and assigned objects to leafs
    
    while(Qnodes.size()>0)
    {
        TreeNode* currentNode = Qnodes.front(); //take one node. if leaf -  assign its objects. if not - go further
        
        Qnodes.pop();
        if(currentNode->leafnode)
        {
            /* 
             * Get all objects of one color -> assign them
             * delete mbr of assigned objects and update tree
             * get all objects of opposite color and do the same
             * then delete all node from light tree
             */
            
            
            /*
             * flip coin to choose which color to pick first from a leaf
             */
            double coin = (rand()/(double)(RAND_MAX));
            int current_type = (coin > 0.5)?0:1;
            
            for (int i = 0; i < TYPES; i++) // take two colors successively
            {
                current_type = !current_type;
                opType = (current_type == 0)?1:0;
                /*
                 * For each object in the leaf node of type <current_type>
                 * do the assignment to the tree from the top
                 */
                for (SpatialObjectList::iterator it = currentNode->attachedObjs[current_type].begin();
                        it != currentNode->attachedObjs[current_type].end(); it++) 
                {
                    ptr = root;
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
                        if (canFilter == true && FLAT::Box::overlap((*it)->mbr, nextNode->mbrSelfD[opType]))
                        {
                            canFilter = false;
                        }
                        
                        for (NodeList::iterator nit = ptr->entries.begin(); nit != ptr->entries.end(); nit++)
                        {
                            
                            /*
                             * Check the intersection with black node
                             * That is combination of:
                             *  - dark current
                             *  - dark of descendants
                             *  - light current
                             */
                            if ( FLAT::Box::overlap((*it)->mbr,(*nit)->mbrK[opType]) )
                            {
                                if(!overlaps)
                                {
                                    overlaps = true;
                                    nextNode = (*nit);
                                }
                                else
                                {
                                    assign(ptr, (*it));
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
                                assign(ptr, (*it));
                                break;
                            }
                            //filtered
                            filtered[current_type]++;
                            break;
                        }
                        ptr = nextNode;

                        if(ptr->leafnode)
                        {
                            assign(ptr, (*it));
                            break;
                        }
                    }
                }
                /* End of object assignment */

                //here - delete all leaf node (current type) and update light tree and black tree
                //do not delete - only earse mbr's
                currentNode->mbrL[current_type].isEmpty = true;
                while (1)
                {
                    if (currentNode->root == 1)
                            break; // we are at root
                    ancestorNode = currentNode->parentNode;
                    ancestorNode->mbrL[current_type].isEmpty = true; // reset mbr
                    for (NodeList::iterator it=ancestorNode->entries.begin(); it!=ancestorNode->entries.end(); ++it)
                    {
                        ancestorNode->mbrL[current_type] = FLAT::Box::combineSafe((*it)->mbrL[current_type], ancestorNode->mbrL[current_type]);
                    }
                    ancestorNode->mbrK[current_type] = FLAT::Box::combineSafe(ancestorNode->mbrD[current_type],ancestorNode->mbrL[current_type]); //update black
                    ancestorNode->mbrK[current_type] = FLAT::Box::combineSafe(ancestorNode->mbrSelfD[current_type],ancestorNode->mbrK[current_type]);
                }
            }
        }
        else
        {
            for (NodeList::iterator nextit = currentNode->entries.begin(); 
                    nextit != currentNode->entries.end(); nextit++)
            {
                    Qnodes.push((*nextit));
            }
        }
        
    }

    building.stop();
}

void cTOUCH::assign(TreeNode* ptr, TreeEntry* obj)
{
    int current_type = obj->type;
    int opType = (current_type == 0)?1:0; // opposite type
    
    TreeNode* ancestorNode;
    
    //should be assigned to this level
    ptr->attachedObjs[current_type].push_back(obj);

    //start updating from the first ancestor, but in the object only update SelfMBR

    ancestorNode->mbrSelfD[current_type] = FLAT::Box::combineSafe(ancestorNode->mbrSelfD[current_type], obj->mbr);
    ancestorNode->mbrK[current_type] = FLAT::Box::combineSafe(ancestorNode->mbrSelfD[current_type], ancestorNode->mbrK[current_type]);

    /*
     * Update MBR's and costs of ancestors
     */
    while (1)
    {
        if (ancestorNode->root == 1)
            break; // we are at root
        ancestorNode = ancestorNode->parentNode;

        ancestorNode->mbrD[current_type] = FLAT::Box::combineSafe(ancestorNode->mbrD[current_type], obj->mbr);
        ancestorNode->mbrK[current_type] = FLAT::Box::combineSafe(ancestorNode->mbrD[current_type], ancestorNode->mbrK[current_type]);

        //here - update cost
//        for (SpatialObjectList::iterator it = ancestorNode->attachedObjs[opType].begin();
//            it != ancestorNode->attachedObjs[opType].end(); it++)
//        {
//            (*it)->cost++;
//        }

    }
}

void cTOUCH::joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode)
{
        queue<TreeNode*> nodes;
        
        int opType = (obj->type)?0:1;
        
        TreeNode* node;
        TreeNode* downnode;
        nodes.push(ancestorNode);
        
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
                        downnode = (*it);
                        
                        //if intersects
                        if (FLAT::Box::overlap(obj->mbr, (*it)->mbrSelfD[opType]))
                        {
                            ItemsMaxCompared += downnode->attachedObjs[opType].size();
                            comparing.start();
                            if(localJoin == algo_SGrid)
                            {
                                downnode->spatialGridHash[opType]->probe(obj);
                            }
                            else
                            {
                                NL(obj, downnode->attachedObjs[opType]);
                            }
                            comparing.stop();
                        } 
                        if (FLAT::Box::overlap(obj->mbr, (*it)->mbrD[opType]))
                        {
                            nodes.push((*it));
                        } 
                }

        }
}
