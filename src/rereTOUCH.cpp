/* 
 * File:   rereTOUCH.cu
 * 
 */

#include "rereTOUCH.h"

void rereTOUCH::run()
{
    totalTimeStart();
    readBinaryInput(file_dsA, file_dsB);
    cout << "Forming the partitions" << std::endl; 
    createPartitions();
    cout << "Assigning the objects of B" << std::endl; 
    assignmentB();
    cout << "Assigning the objects of A" << std::endl; 
    assignmentA();
    cout << "Assigning the objects of B again" << std::endl; 
    reassignmentB();
    cout << "Assigning Done." << std::endl; 
    analyze();
    cout << "Analysis Done, counting grids if necessary." << std::endl; 
    if(localJoin == algo_SGrid)
        countSpatialGrid();
    cout << "Probing, doing the join" << std::endl; 
    probe();
    if(localJoin == algo_SGrid)
    {
        std::cout << "Removing duplicates" << std::endl; 
        deduplicateSpatialGrid();
    }
    cout << "Done." << std::endl; 
    totalTimeStop();
}

void rereTOUCH::probe()
{
    //For every cell of A join it with its corresponding cell of B
    probing.start();
    queue<TreeEntry*> Qnodes;

    TreeNode* currentNode;
    FLAT::Box localUniverse;

    Qnodes.push(root);
    //uni.push(root->mbr);

    int lvl = Levels;
    // A BFS on the tree then for each find all its leaf nodes by another BFS
    while(Qnodes.size()>0)
    {
        TreeEntry* parentNode = Qnodes.front();
        currentNode = tree[parentNode->childIndex];
        Qnodes.pop();
        //BFS on the tree using Qnodes as the queue for memorizing the future nodes to be traversed
        if(!currentNode->leafnode)
        for (std::vector<TreeEntry*>::iterator ent=currentNode->entries.begin();ent!=currentNode->entries.end();++ent)
        {
                Qnodes.push(*ent);
        }
        //join the internal node with all *its* leaves



        // just to display the level of the BFS traversal
        if(lvl!=currentNode->level)
        {
                lvl = currentNode->level;
                cout << "\n### Level " << lvl <<endl;
        }

        joinNodeToDesc(parentNode); //join node -> join each object -> join object to down tree -> join object with list of objects

    }
    probing.stop();

    //check the duplicates:
    //resultPairs.deDuplicate(); //@todo where is it?
}

void rereTOUCH::analyze()
{
    analyzing.start();
    FLAT::uint64 emptyCells=0;
    FLAT::uint64 sumA=0,sqsumA=0;
    FLAT::uint64 sumB=0,sqsumB=0;
    double differenceSquared=0;
    //statsTOUCH.footprint += dsA.size()*(sizeof(FLAT::SpatialObject*));
    //statsTOUCH.footprint += dsB.size()*(sizeof(FLAT::SpatialObject*));
    LVL = Levels;
    //vector<FLAT::uint64> ItemPerLevel;
    ItemPerLevelA.reserve(Levels);
    ItemPerLevelAans.reserve(Levels);
    ItemPerLevelBans.reserve(Levels);
    ItemPerLevelB.reserve(Levels);
    for(int i = 0 ; i<Levels ; i++)
    {
            ItemPerLevelA.push_back(0);
            ItemPerLevelAans.push_back(0);
            ItemPerLevelBans.push_back(0);
            ItemPerLevelB.push_back(0);
    }
    for(unsigned int ni = 0; ni<tree.size() ; ni++)
    {
            SpatialObjectList objA = tree[ni]->attachedObjs[0];
            FLAT::uint64 ptrs = objA.size();
            if(objA.size()==0)emptyCells++;
            ItemPerLevelA[tree[ni]->level]+=ptrs;
            ItemPerLevelAans[tree[ni]->level]+=tree[ni]->attachedObjsAns[0].size();
            ItemPerLevelBans[tree[ni]->level]+=tree[ni]->attachedObjsAns[1].size();
            sumA += ptrs+tree[ni]->attachedObjsAns[0].size();
            sqsumA += ptrs*ptrs;
            if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

            SpatialObjectList objB = tree[ni]->attachedObjs[1];
            ptrs = objB.size();
            if(objB.size()==0)emptyCells++;
            ItemPerLevelB[tree[ni]->level]+=ptrs;
            sumB += ptrs+tree[ni]->attachedObjsAns[1].size();
            sqsumB += ptrs*ptrs;
            if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

    }
    if (verbose)
    {
        for(int i = 0 ; i<Levels ; i++)
        {
                cout<< "level " << i << " A:" << ItemPerLevelA[i] << " Aans:" 
                << ItemPerLevelAans[i] << " B:" << ItemPerLevelB[i]
                << " Bans: " << ItemPerLevelBans[i]  << " = " << ItemPerLevelA[i] + ItemPerLevelB[i] <<endl;
        }
        cout<<"Total assigned A:"<<sumA<<" B:"<<sumB<<" = "<< sumA+sumB<<endl;
        cout<<"Total filtered A:"<< filtered[0] <<" B:"<< filtered[1] <<" = "<< filtered[0]+filtered[1] <<endl;
    }
    footprint += (sumA+sumB)*sizeof(FLAT::SpatialObject*) + tree.size()*(sizeof(TreeNode*));
    avg = (sumA+sumB+0.0) / (tree.size());
    percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
    differenceSquared = ((double)(sqsumA+sqsumB)/(double)tree.size())-avg*avg;
    std = sqrt(differenceSquared);
    analyzing.stop();

}

/*
 * Create new node according to set of TreeEntries. Entries can be of both types,
 * So create to entries that point to the new node of two types.
 * Create entry iff it is not empty
 */
void rereTOUCH::writeNode(std::vector<TreeEntry*> objlist,int Level)
{
    TreeNode* prNode = new TreeNode(Level);
    FLAT::Box mbr;
    FLAT::uint64 childIndex;
    totalnodes ++;
    for (std::vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
    {
            prNode->entries.push_back(*it);
            mbr = FLAT::Box::combineSafe((*it)->mbrL[0],mbr);
    }
    childIndex = tree.size();

    tree.push_back(prNode);
    nextInput.push_back(new TreeEntry(mbr,childIndex));
    prNode->parentEntry = nextInput.back();
}

void rereTOUCH::assignmentB()
{
    
    building.start();
    
    SpatialObjectList new_dsB;
    new_dsB.reserve(dsB.size());
    
    bool overlaps;
    bool assigned;
    for (unsigned int i=0;i<dsB.size();++i)
    {
        FLAT::SpatialObject* objB = dsB[i];
        FLAT::Box objMBR = objB->getMBR();
        objMBR.isEmpty = false;
        FLAT::Box::expand(objMBR,epsilon * 1./2.);
        TreeEntry* nextNode, *prevNode;

        nextNode = root;
        prevNode = nextNode;
        TreeNode* ptr = tree[nextNode->childIndex];


        while(true)
        {
            overlaps = false;
            assigned = false;
            for (std::vector<TreeEntry*>::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
            {
                if ( FLAT::Box::overlap(objMBR,(*ent)->mbrL[0]) )
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
                        new_dsB.push_back(objB);
                        prevNode->mbrSelfD[1] = FLAT::Box::combineSafe(objMBR,prevNode->mbrSelfD[1]);
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
            ptr = tree[nextNode->childIndex];
            if(ptr->leafnode)
            {
                ptr->attachedObjs[1].push_back(objB);
                new_dsB.push_back(objB);
                nextNode->mbrSelfD[1]=FLAT::Box::combineSafe(objMBR,nextNode->mbrSelfD[1]);
                nextNode->num[1]++;
                break;
            }
        }
    }

    //Fixing the mbrs of dataset B after assign
    root->mbrL[1].isEmpty=true;
    root->num[1]=mergingMbrB(root,root->mbrL[1],true);
    dsB.swap(new_dsB);
    building.stop();
}

//Fix the mbrs of object of A and the number of A in desc
FLAT::uint64 rereTOUCH::mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr)
{
        TreeNode* ptr = tree[startEntry->childIndex];
        FLAT::uint64 numA=0;
        if (ptr->leafnode)
        {
                mbr.isEmpty=true;
                return numA;
        }

        for (std::vector<TreeEntry*>::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
        {
                numA+=(*ent)->num[0];
                (*ent)->mbrL[1].isEmpty = true;
                (*ent)->mbrSelfD[1].isEmpty = true;
                tree.at((*ent)->childIndex)->attachedObjs[1].clear();
                (*ent)->num[0]=mergingMbrA((*ent),(*ent)->mbrL[0]);
                numA+=(*ent)->num[0];
                mbr=FLAT::Box::combineSafe((*ent)->mbrL[0],mbr);
                mbr=FLAT::Box::combineSafe((*ent)->mbrSelfD[0],mbr);
        }
        return numA;
}

void rereTOUCH::assignmentA()
{
    building.start();
    int overlapB=0; //number of overlaps bellow
    int overlapT=0; //number of overlaps to the assigned objects of This level
    bool Poverlaps;//Overlaps to a parent
 
    for (unsigned int i=0;i<dsA.size();++i)
    {
        FLAT::SpatialObject* objA = dsA[i];
        FLAT::Box objMBR = objA->getMBR();
        objMBR.isEmpty = false;
        FLAT::Box::expand(objMBR,epsilon * 1./2.);
        TreeEntry* nextNode=root,*Ansptr;
        TreeNode* ptr = tree[nextNode->childIndex];
        Poverlaps=false;
        // Check the root node
        if ( FLAT::Box::overlap(objMBR,root->mbrL[1]) )
                overlapB++;
        if ( FLAT::Box::overlap(objMBR,root->mbrSelfD[1]) )
        {
            if(overlapB==0)
            {
                //assign to the root
                ptr->attachedObjs[0].push_back(objA);
                //Expanding the mbr
                ptr->parentEntry->mbrSelfD[0] = FLAT::Box::combineSafe(objMBR,nextNode->mbrSelfD[0]);
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
            for (std::vector<TreeEntry*>::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
            {
                if(overlapB>1 || overlapT>1 || (overlapB==1 && overlapT==1 && Ansptr != nextNode))
						break;
                if ( FLAT::Box::overlap(objMBR,(*ent)->mbrL[1]) )
                {
                    overlapB++;
                    if(overlapB==1)
                    {
                        nextNode = (*ent);
                    }
                    //increment the cost function of B below objects
                }
                if ( FLAT::Box::overlap(objMBR,(*ent)->mbrSelfD[1]) )
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
                 //   if (objA->id == 9714) cout << "===========9714 assigned 1 case " << ptr->level << endl;
                //Expanding the mbr
                ptr->parentEntry->mbrSelfD[0] = FLAT::Box::combineSafe(objMBR,ptr->parentEntry->mbrSelfD[0]);
                ptr->parentEntry->num[0]++;
                break;
            }
            //if B&T overlaps: if single B and single T and T&B is the same node then set the Ansptr and search below, otherwise set to parent.
            if(overlapB==1)
            {
                ptr = tree[nextNode->childIndex];
                if(ptr->leafnode)
                {
                    ptr->attachedObjs[0].push_back(objA);
                   // if (objA->id == 9714) cout << "===========9714 assigned 2 case" << endl;
                    //Expanding the mbr
                    ptr->parentEntry->mbrSelfD[0] = FLAT::Box::combineSafe(objMBR,ptr->parentEntry->mbrSelfD[0]);
                    ptr->parentEntry->num[0]++;
                    break;
                }
                continue;
            }
            //overlapB==0 && overlapT==0
            //Make decision to assign or filter
            if(Poverlaps)
            {
                //assign to the Ansptr
                tree[Ansptr->childIndex]->attachedObjs[0].push_back(objA);
                   // if (objA->id == 9714) cout << "===========9714 assigned 3 case" << endl;
                //Expanding the mbr
                Ansptr->mbrSelfD[0] = FLAT::Box::combineSafe(objMBR,Ansptr->mbrSelfD[0]);
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

    //Fixing the mbrs of dataset A after assign
    root->mbrL[0].isEmpty=true;
    tree.at(root->childIndex)->attachedObjs[1].clear();
    tree.at(root->childIndex)->attachedObjsAns[1].clear();
    root->num[0] = mergingMbrA(root,root->mbrL[0]);
    //printTree(root);

    building.stop();

}

bool rereTOUCH::verifyMBR(TreeEntry* ent)
{
    FLAT::Box mbr;
    TreeNode* node = tree.at(ent->childIndex);
    for (int i=0; i < node->attachedObjs[0].size(); i++)
    {
        mbr = node->attachedObjs[0][i]->getMBR();
        mbr.isEmpty = false;
        FLAT::Box::expand(mbr,epsilon/2.);
        if (FLAT::Box::enclose(ent->mbrSelfD[0],mbr) == false)
            return false;
        if (!node->leafnode)
            for (int j = 0; j < node->entries.size(); j++)
            {
                if (FLAT::Box::enclose(ent->mbrL[0],node->entries.at(j)->mbrSelfD[0]) == false)
                    return false;
                if(!verifyMBR(node->entries.at(j))) return false;
            }
    }
    return true;
}

void rereTOUCH::reassignmentB()
{
    building.start();
    int overlapA=0; //number of overlaps bellow
    int overlapT=0; //number of overlaps to the assigned objects of This level
    bool Poverlaps;//Overlaps to a parent
 
    for (unsigned int i=0;i<dsB.size();++i)
    {
        FLAT::SpatialObject* objB = dsB[i];
        FLAT::Box objMBR = objB->getMBR();
        objMBR.isEmpty = false;
        FLAT::Box::expand(objMBR,epsilon * 1./2.);
        TreeEntry* nextNode=root,*Ansptr;
        TreeNode* ptr = tree[nextNode->childIndex];
        
        Poverlaps=false;
        // Check the root node
        if ( FLAT::Box::overlap(objMBR,root->mbrL[0]) )
                overlapA++;
        if ( FLAT::Box::overlap(objMBR,root->mbrSelfD[0]) )
        {
            if(overlapA==0)
            {
                //assign to the root
                ptr->attachedObjs[1].push_back(objB);
                //Expanding the mbr
                ptr->parentEntry->mbrSelfD[1] = FLAT::Box::combineSafe(objMBR,nextNode->mbrSelfD[1]);
                   // if (objB->id == 8034) cout << "===========8034 B reassigned" << endl;
                nextNode->num[1]++;
                continue;
            }
            overlapT++;
            Poverlaps=true;
            Ansptr = root;
        }

        if(overlapA==0 && overlapT==0)
        {
            //filter and continue
            filtered[1]++;
            continue;
        }

        //find the number of Boverlaps of belows
        //if more than one assign to the parent.
        //else, check the Toverlaps of assings of This level
        //if no B&T overlaps, if no poverlaps filter, otherwise assign to Ansptr
        //if B&T overlaps: if single B and single T and T&B is the same node then set the Ansptr and search below, otherwise set to parent.

        while(true)
        {
            overlapA = 0;
            overlapT = 0;
            //find the number of Boverlaps of belows and the number of Toverlaps of assings of This level
            for (std::vector<TreeEntry*>::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
            {
                if(overlapA>1 || overlapT>1 || (overlapA==1 && overlapT==1 && Ansptr != nextNode))
						break;
                if ( FLAT::Box::overlap(objMBR,(*ent)->mbrL[0]) )
                {
                    overlapA++;
                    if(overlapA==1)
                    {
                        nextNode = (*ent);
                    }
                    //increment the cost function of B below objects
                }
                if ( FLAT::Box::overlap(objMBR,(*ent)->mbrSelfD[0]) )
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
            if(overlapA>1 || overlapT>1 || (overlapA==1 && overlapT==1 && Ansptr != nextNode))
            {
                //should be assigned to this level
                ptr->attachedObjs[1].push_back(objB);
                //if (objB->id == 8034) cout << "===========8034 B reassigned 1c" << endl;
                //Expanding the mbr
                ptr->parentEntry->mbrSelfD[1] = FLAT::Box::combineSafe(objMBR,ptr->parentEntry->mbrSelfD[1]);
                ptr->parentEntry->num[1]++;
                break;
            }
            //if B&T overlaps: if single B and single T and T&B is the same node then set the Ansptr and search below, otherwise set to parent.
            if(overlapA==1)
            {
                ptr = tree[nextNode->childIndex];
                if(ptr->leafnode)
                {
                    ptr->attachedObjs[1].push_back(objB);
                   // if (objB->id == 8034) cout << "===========8034 B reassigned 2c" << endl;
                    //Expanding the mbr
                    ptr->parentEntry->mbrSelfD[1] = FLAT::Box::combineSafe(objMBR,ptr->parentEntry->mbrSelfD[1]);
                    ptr->parentEntry->num[1]++;
                    break;
                }
                continue;
            }
            //overlapB==0 && overlapT==0
            //Make decision to assign or filter
            if(Poverlaps)
            {
                //assign to the Ansptr
                tree[Ansptr->childIndex]->attachedObjsAns[1].push_back(objB);
             //   if (objB->id == 8034) cout << "===========8034 B reassigned 3c level " << tree[Ansptr->childIndex]->level << endl;
                //Expanding the mbr
                Ansptr->mbrSelfD[1] = FLAT::Box::combineSafe(objMBR,Ansptr->mbrSelfD[1]);
                Ansptr->num[1]++;
            }
            else
            {
                // filter
                filtered[1]++;
            }
            break;
        }
    }

    //Fixing the mbrs of dataset B after assign
    root->mbrL[1].isEmpty=true;
    root->num[1] = mergingMbrB(root,root->mbrL[1],false);
    //printTree(root);

    building.stop();

}

void rereTOUCH::joinObjectToDesc(FLAT::SpatialObject* obj, TreeEntry* ancestorNode)
{
    queue<TreeEntry*> nodes;
    
    int type = obj->type;
    int opType = (obj->type)?0:1;
    
    TreeNode* node, * downnode;
    nodes.push(ancestorNode);
    FLAT::Box objMBR = obj->getMBR();
    objMBR.isEmpty = false;
    FLAT::Box::expand(objMBR,epsilon * 1./2.);
    while(nodes.size()>0)
    {
        //start from checking children, each for intersection of MBR
        // then if intersects - check the assign objects of child
        // and if it is not a leaf node and intersects -> add to the queue


        node = tree[nodes.front()->childIndex];
        nodes.pop();

        for (FLAT::uint64 child = 0; child < node->entries.size(); ++child)
        {
            //if intersects
            downnode = tree[node->entries[child]->childIndex];
            if (FLAT::Box::overlap(objMBR, node->entries[child]->mbrSelfD[opType]))
            {
                comparing.start();
                ItemsMaxCompared += downnode->attachedObjs[opType].size()+downnode->attachedObjsAns[opType].size();
                if(localJoin == algo_SGrid)
                {
                    downnode->spatialGridHash[opType]->probe(obj);
                    downnode->spatialGridHashAns[opType]->probe(obj);
                }
                else
                {
                    NL(obj, downnode->attachedObjs[opType]);
                    NL(obj, downnode->attachedObjsAns[opType]);
                }
                comparing.stop();
            }
            if (FLAT::Box::overlap(objMBR, node->entries[child]->mbrL[opType]))
            {

                    //add child to the queue if it is not a leaf (even in dark)
                    if (!downnode->leafnode)
                            nodes.push(node->entries[child]);
            }

        }

    }
}

void rereTOUCH::joinNodeToDesc(TreeEntry* ancestorNode)
{
    //check for intersection all nodes below, where smth was attached with opposite color.
    


    // here @todo check function and parallelize
    /*
     * A -> B_below
     */
    TreeNode* node=tree[ancestorNode->childIndex];
    
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
    ItemsMaxCompared += (node->attachedObjs[1].size()+node->attachedObjsAns[1].size())*(node->attachedObjs[0].size()+node->attachedObjsAns[0].size());
    if (node->attachedObjs[0].size() < node->attachedObjs[1].size())
    {
        if(localJoin == algo_SGrid)
        {
            comparing.start();
            node->spatialGridHash[0]->probe(node->attachedObjs[1]);
            node->spatialGridHash[0]->probe(node->attachedObjsAns[1]);
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
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[1]))
                {
                    NL((*it), node->attachedObjs[1]);
                    NL((*it), node->attachedObjsAns[1]);
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
            node->spatialGridHashAns[1]->probe(node->attachedObjs[0]);
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
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[0]))
                {
                        NL((*it), node->attachedObjs[0]);
                }
                comparing.stop();

            }
            for (SpatialObjectList::iterator it = node->attachedObjsAns[1].begin();
                                    it != node->attachedObjsAns[1].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                mbr.isEmpty = false;
                FLAT::Box::expand(mbr,epsilon * 1./2.);
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[0]))
                {
                        NL((*it), node->attachedObjs[0]);
                }
                comparing.stop();

            }
        }
    }


}

FLAT::uint64 rereTOUCH::mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr, bool clearA)
{
    TreeNode* ptr = tree[startEntry->childIndex];
    FLAT::uint64 numB=0;

    if (ptr->leafnode)
    {
        ptr->entries.clear();
        mbr.isEmpty=true;
        return numB;
    }

    //fixing MBR of the descendant nodes' objects
    for (std::vector<TreeEntry*>::iterator ent = ptr->entries.begin(); ent!=ptr->entries.end(); ++ent)
    {
        numB += (*ent)->num[1];
        if (clearA) (*ent)->mbrL[0].isEmpty = true;
        (*ent)->num[1] = mergingMbrB((*ent), (*ent)->mbrL[1], clearA);
        numB += (*ent)->num[1];
        mbr = FLAT::Box::combineSafe((*ent)->mbrL[1],mbr);
        mbr = FLAT::Box::combineSafe((*ent)->mbrSelfD[1],mbr);
    }
    return numB;
}