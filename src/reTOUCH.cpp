/* 
 * File:   reTOUCH.cu
 * 
 */

#include "reTOUCH.h"

void reTOUCH::run()
{
    totalTimeStart();
    readBinaryInput(file_dsA, file_dsB);
    if (verbose) std::cout << "Forming the partitions" << std::endl; 
    createPartitions(vdsA);
    if (verbose) std::cout << "Assigning the objects of B" << std::endl; 
    assignmentB();
    if (verbose) std::cout << "Assigning the objects of A" << std::endl; 
    assignmentA();
    if (verbose) std::cout << "Assigning Done." << std::endl; 
    analyze();
    if (verbose) std::cout << "Analysis Done, counting grids if necessary." << std::endl; 
    if(localJoin == algo_SGrid)
        countSpatialGrid();
    if (verbose) std::cout << "Probing, doing the join" << std::endl; 
    probe();
    //probeDownUp();
    if(localJoin == algo_SGrid)
    {
        if (verbose) std::cout << "Removing duplicates" << std::endl; 
        deduplicateSpatialGrid();
    }
    if (verbose) std::cout << "Done." << std::endl; 
    totalTimeStop();
}

void reTOUCH::assignmentB()
{
    building.start();
    bool overlaps;
    bool assigned;
    for (unsigned int i=0;i<dsB.size();++i)
    {
        TreeEntry* objB = dsB[i];
        TreeNode* nextNode, *prevNode;
        
        nextNode = root;
        prevNode = nextNode;
        TreeNode* ptr = nextNode;
        while(true)
        {
            overlaps = false;
            assigned = false;
            for (NodeList::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
            {
                if ( FLAT::Box::overlap(objB->mbr,(*ent)->mbrL[0]) )
                {
                    if(!overlaps)
                    {
                        overlaps = true;
                        prevNode = nextNode;
                        nextNode = (*ent);
                    }
                    else
                    {
                        //should be assigned to this level
                        ptr->attachedObjs[1].push_back(objB);
                        //Expand the prevnode->mbrSelfD[1]
                        //Expanding the mbrL[1] to include the above A assigned object
                        prevNode->mbrSelfD[1] = FLAT::Box::combineSafe(objB->mbr,prevNode->mbrSelfD[1]);
                        prevNode->num[1]++;
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
                    filtered[1]++;
                    break;
            }
            ptr = nextNode;
            if(ptr->leafnode)
            {
                ptr->attachedObjs[1].push_back(objB);
                //Expand the prevnode->mbrSelfD[1]
                //Expanding the mbrL[1] to include the above A assigned object
                nextNode->mbrSelfD[1]=FLAT::Box::combineSafe(objB->mbr,nextNode->mbrSelfD[1]);
                nextNode->num[1]++;
                break;
            }
        }
    }

    //Fixing the mbrs of dataset B after assign
    root->mbrL[1].isEmpty=true;
    root->num[1]=mergingMbrB(root,root->mbrL[1]);

    building.stop();
}

//Fix the mbrs of object of A and the number of A in desc
FLAT::uint64 reTOUCH::mergingMbrA(TreeNode* startNode, FLAT::Box &mbr)
{
        TreeNode* ptr = startNode;
        FLAT::uint64 numA=0;
        if (ptr->leafnode)
        {
                mbr.isEmpty=true;
                //mbrL[0]=new Box(true);
                return numA;
        }

        //fixing MBR of the descendant nodes' objects
        //numA+=ptr->attachedObjs[0].size()+ptr->attachedObjsAns[0].size();
        for (NodeList::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
        {
                numA+=(*ent)->num[0];
                (*ent)->num[0]=mergingMbrA((*ent),(*ent)->mbrL[0]);
                numA+=(*ent)->num[0];
                mbr=FLAT::Box::combineSafe((*ent)->mbrL[0],mbr);
                mbr=FLAT::Box::combineSafe((*ent)->mbrSelfD[0],mbr);
        }
        return numA;
}

void reTOUCH::assignmentA()
{
    building.start();
    int overlapB=0; //number of overlaps bellow
    int overlapT=0; //number of overlaps to the assigned objects of This level
    bool Poverlaps;//Overlaps to a parent
 
    for (unsigned int i=0;i<dsA.size();++i)
    {
        TreeEntry* objA = dsA[i];
        TreeNode* nextNode=root,*Ansptr;
        TreeNode* ptr = nextNode;
        Poverlaps=false;
        // Check the root node
        if ( FLAT::Box::overlap(objA->mbr,root->mbrL[1]) )
                overlapB++;
        if ( FLAT::Box::overlap(objA->mbr,root->mbrSelfD[1]) )
        {
            if(overlapB==0)
            {
                //assign to the root
                ptr->attachedObjs[0].push_back(objA);
                //if (objA->id == 5579) cout << "5579 assigned to the level because no down overlap " << ptr->level << endl;
                //Expanding the mbr
                ptr->mbrSelfD[0] = FLAT::Box::combineSafe(objA->mbr,nextNode->mbrSelfD[0]);
                nextNode->num[0]++;
                continue;
            }
            overlapT++;
            Poverlaps=true;
            Ansptr = root;
        }

        if(overlapB==0 && overlapT==0)
        {
            //filter and continue
            filtered[0]++;
            continue;
        }

        //find the number of Boverlaps of belows
        //if more than one assign to the parent.
        //else, check the Toverlaps of assings of This level
        //if no B&T overlaps, if no poverlaps filter, otherwise assign to Ansptr
        //if B&T overlaps: if single B and single T and T&B is the same node then set the Ansptr and search below, otherwise set to parent.

        while(true)
        {
            overlapB = 0;
            overlapT = 0;
            //find the number of Boverlaps of belows and the number of Toverlaps of assings of This level
            for (NodeList::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
            {   
                if(overlapB>1 || overlapT>1 || (overlapB==1 && overlapT==1 && Ansptr != nextNode))
						break;
                if ( FLAT::Box::overlap(objA->mbr,(*ent)->mbrL[1]) )
                {
                    overlapB++;
                    if(overlapB==1)
                    {
                        ptr = nextNode;
                        nextNode = (*ent);
                    }
                    //increment the cost function of B below objects
                }
                if ( FLAT::Box::overlap(objA->mbr,(*ent)->mbrSelfD[1]) )
                {
                    Poverlaps=true;
                    overlapT++;
                    if(overlapT==1)
                    {
                        Ansptr = (*ent);
                    }
                    //increment the cost function of Assigned B only
                }
            }
            //if more than one below overlap, assign to the parent.
            if(overlapB>1 || overlapT>1 || (overlapB==1 && overlapT==1 && Ansptr != nextNode))
            {
                //should be assigned to this level
                ptr->attachedObjs[0].push_back(objA);
                //if (objA->id == 5579) cout << "objA assigned to the level because many down overlaps" << ptr->level << endl;
                //Expanding the mbr
                ptr->mbrSelfD[0] = FLAT::Box::combineSafe(objA->mbr,ptr->mbrSelfD[0]);
                ptr->num[0]++;
                break;
            }
            //if B&T overlaps: if single B and single T and T&B is the same node then set the Ansptr and search below, otherwise set to parent.
            if(overlapB==1)
            {
                ptr = nextNode;
                if(ptr->leafnode)
                {
                    ptr->attachedObjs[0].push_back(objA);
                    //Expanding the mbr
                    ptr->mbrSelfD[0] = FLAT::Box::combineSafe(objA->mbr,ptr->mbrSelfD[0]);
                    ptr->num[0]++;
                    break;
                }
                continue;
            }
            //overlapB==0 && overlapT==0
            //Make decision to assign or filter
            if(Poverlaps)
            {
                //assign to the Ansptr
                Ansptr->attachedObjsAns[0].push_back(objA);
                //Expanding the mbr
                Ansptr->mbrSelfD[0] = FLAT::Box::combineSafe(objA->mbr,Ansptr->mbrSelfD[0]);
                Ansptr->num[0]++;
            }
            else
            {
                // filter
                filtered[0]++;
            }
            break;
        }
    }

    //Fixing the mbrs of dataset B after assign
    root->mbrL[0].isEmpty=true;
    root->num[0] = mergingMbrA(root,root->mbrL[0]);
    //printTree(root);

    building.stop();

}

void reTOUCH::joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode)
{
    queue<TreeNode*> nodes;
    TreeNode* node, * downnode;
    nodes.push(ancestorNode);
    FLAT::Box& objMBR = obj->getMBR();
    
    while(nodes.size()>0)
    {
        //start from checking children, each for intersection of MBR
        // then if intersects - check the assign objects of child
        // and if it is not a leaf node and intersects -> add to the queue


        node = nodes.front();
        nodes.pop();


        if (obj->type == 0)
        {
            //intersect with all non-null children
            for (NodeList::iterator it = node->entries.begin(); it != node->entries.end(); ++it)
            {
                //if intersects
                if (FLAT::Box::overlap(objMBR, (*it)->mbrSelfD[1]))
                {
                    ItemsMaxCompared += (*it)->attachedObjs[1].size();
                    comparing.start();
                    if(localJoin == algo_SGrid)
                    {
                        (*it)->spatialGridHash[1]->probe(obj);
                    }
                    else
                    {
                        NL(obj, (*it)->attachedObjs[1]);
                    }
                    comparing.stop();
                }
                if (FLAT::Box::overlap(objMBR, (*it)->mbrL[1]))
                {
                        //add child to the queue if it is not a leaf (even in dark)
                        if (!(*it)->leafnode)
                                nodes.push((*it));
                }
            }
        }
        else
        {
            for (NodeList::iterator it = node->entries.begin(); it != node->entries.end(); ++it)
            {
                //if intersects
                
                downnode = (*it);
                if (FLAT::Box::overlap(objMBR, (*it)->mbrSelfD[0]))
                {
                    comparing.start();
                    ItemsMaxCompared += downnode->attachedObjs[0].size()+downnode->attachedObjsAns[0].size();
                    
                    if(localJoin == algo_SGrid)
                    {
                        downnode->spatialGridHash[0]->probe(obj);
                        downnode->spatialGridHashAns[0]->probe(obj);
                    }
                    else
                    {
                        NL(obj, downnode->attachedObjs[0]);
                        NL(obj, downnode->attachedObjsAns[0]);
                    }
                    comparing.stop();
                }
                if (FLAT::Box::overlap(objMBR, (*it)->mbrL[0]))
                {
                        //add child to the queue if it is not a leaf (even in dark)
                        if (!downnode->leafnode)
                                nodes.push((*it));
                }

            }
        }

    }
}

void reTOUCH::joinNodeToDesc(TreeNode* ancestorNode)
{
    //check for intersection all nodes below, where smth was attached with opposite color.

    /*
     * A -> B_below
     */
    TreeNode* node=ancestorNode;
    for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                    it != node->attachedObjs[0].end(); it++)
    {
        joinObjectToDesc((*it), ancestorNode);
    }

    /*
     * B -> A_below
     */
    for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                                    it != node->attachedObjs[1].end(); it++)
    {
        joinObjectToDesc((*it), ancestorNode);
    }

    /*
     * A -> B
     */
    ItemsMaxCompared += node->attachedObjs[1].size()*(node->attachedObjs[0].size()+node->attachedObjsAns[0].size());
    if (node->attachedObjs[0].size() < node->attachedObjs[1].size())
    {
        if(localJoin == algo_SGrid)
        {
            comparing.start();
            node->spatialGridHash[0]->probe(node->attachedObjs[1]);
            node->spatialGridHashAns[0]->probe(node->attachedObjs[1]);
            comparing.stop();
        }
        else
        {
            for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                            it != node->attachedObjs[0].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[1]))
                {
                    NL((*it), node->attachedObjs[1]);
                }
                comparing.stop();
            }
            for (SpatialObjectList::iterator it = node->attachedObjsAns[0].begin();
                                                            it != node->attachedObjsAns[0].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[1]))
                {
                    NL((*it), node->attachedObjs[1]);
                }
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
            node->spatialGridHash[1]->probe(node->attachedObjsAns[0]);
            comparing.stop();
        }
        else
        {
            for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                    it != node->attachedObjs[1].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[0]))
                {
                        NL((*it), node->attachedObjs[0]);
                        NL((*it), node->attachedObjsAns[0]);
                }
                comparing.stop();

            }
        }
    }


}

FLAT::uint64 reTOUCH::mergingMbrB(TreeNode* startEntry, FLAT::Box &mbr)
{
    TreeNode* ptr = startEntry;
    FLAT::uint64 numB=0;

    if (ptr->leafnode)
    {
        ptr->entries.clear();
        mbr.isEmpty=true;
        return numB;
    }

    //fixing MBR of the descendant nodes' objects
    for (NodeList::iterator ent = ptr->entries.begin(); ent!=ptr->entries.end(); ++ent)
    {
        numB += (*ent)->num[1];
        (*ent)->mbrL[0].isEmpty = true;
        (*ent)->attachedObjs[0].clear();
        (*ent)->num[1] = mergingMbrB((*ent), (*ent)->mbrL[1]);
        numB += (*ent)->num[1];
        mbr = FLAT::Box::combineSafe((*ent)->mbrL[1],mbr);
        mbr = FLAT::Box::combineSafe((*ent)->mbrSelfD[1],mbr);
    }
    return numB;
}