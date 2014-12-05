/* 
 * File:   reTOUCH.cu
 * 
 */

#include "reTOUCH.h"

void reTOUCH::analyze()
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
    ItemPerLevelB.reserve(Levels);
    for(int i = 0 ; i<Levels ; i++)
    {
            ItemPerLevelA.push_back(0);
            ItemPerLevelAans.push_back(0);
            ItemPerLevelB.push_back(0);
    }
    for(unsigned int ni = 0; ni<tree.size() ; ni++)
    {
            SpatialObjectList objA = tree[ni]->attachedObjs[0];
            FLAT::uint64 ptrs = objA.size();
            if(objA.size()==0)emptyCells++;
            ItemPerLevelA[tree[ni]->level]+=ptrs;
            ItemPerLevelAans[tree[ni]->level]+=tree[ni]->attachedObjsAns[0].size();
            sumA += ptrs+tree[ni]->attachedObjsAns[0].size();
            sqsumA += ptrs*ptrs;
            if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

            SpatialObjectList objB = tree[ni]->attachedObjs[1];
            ptrs = objB.size();
            if(objB.size()==0)emptyCells++;
            ItemPerLevelB[tree[ni]->level]+=ptrs;
            sumB += ptrs;
            sqsumB += ptrs*ptrs;
            if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;

    }
    for(int i = 0 ; i<Levels ; i++)
    {
            cout<< "level " << i << " A:" << ItemPerLevelA[i] << " Aans:" 
            << ItemPerLevelAans[i] << " B:" << ItemPerLevelB[i] << " =" << ItemPerLevelA[i] + ItemPerLevelB[i] <<endl;
    }
    cout<<"Total assigned A:"<<sumA<<" B:"<<sumB<<" ="<< sumA+sumB<<endl;
    cout<<"Total filtered A:"<< dsA.size() - sumA<<" B:"<<dsB.size() -sumB<<" ="<< dsA.size()+dsB.size() -(sumA+sumB)<<endl;
    footprint += (sumA+sumB)*sizeof(FLAT::SpatialObject*) + tree.size()*(sizeof(TreeNode*));
    avg = (sumA+sumB+0.0) / (tree.size());
    percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
    differenceSquared = ((double)(sqsumA+sqsumB)/(double)tree.size())-avg*avg;
    std = sqrt(differenceSquared);
    analyzing.stop();

}

void reTOUCH::probe()
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

        joinIntenalnodetoleafs(parentNode); //join node -> join each object -> join object to down tree -> join object with list of objects

    }
    probing.stop();

    //check the duplicates:
    //resultPairs.deDuplicate(); //@todo where is it?
}

/*
 * Create new node according to set of TreeEntries. Entries can be of both types,
 * So create to entries that point to the new node of two types.
 * Create entry iff it is not empty
 */
void reTOUCH::writeNode(std::vector<TreeEntry*> objlist,int Level)
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

void reTOUCH::createTreeLevel(std::vector<TreeEntry*>& input,int Level)
{
    unsigned int nodeSize;
    if (Level==0) nodeSize = leafsize;
    else nodeSize = nodesize;

    if(PartitioningType != No_Sort)
    {
        sorting.start();
        std::sort(input.begin(),input.end(),Comparator());
        sorting.stop();
    }

    if(PartitioningType == STR_Sort)
    {
        //First sort by X then assigns the objects to partitions then sort by Y each partition separately then again assigns the objects to partitions then sort by Z objects of each partition separately
        sorting.start();
        cout<< "Node size " << nodeSize << endl;
        FLAT::uint64 itemsD1 = nodeSize * nodeSize;
        FLAT::uint64 itemsD2 = nodeSize;
        std::sort(input.begin(),input.end(),Comparator());
        FLAT::uint64 i=0;
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
        sorting.stop();
    }

    cout << "Sort "<< input.size()<< " items in " << sorting << endl;


    std::vector<TreeEntry*> entries;
    for (std::vector<TreeEntry*>::iterator it=input.begin();it!=input.end();++it)
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

void reTOUCH::assignmentB()
{
    building.start();
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
        int temptodelete;
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
                        if (objB->id == 2572) cout << "object 2572 assigned to the level " << ptr->level << " node " << temptodelete <<endl;
                        ptr->attachedObjs[1].push_back(objB);
                        //Expand the prevnode->mbrSelfD[1]
                        //Expanding the mbrL[1] to include the above A assigned object
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
                        if (objB->id == 2572) cout << "object 2572 filtered" << ptr->level << endl;
                    filtered[1]++;
                    break;
            }
            ptr = tree[nextNode->childIndex];
            temptodelete = nextNode->childIndex;
            if(ptr->leafnode /*|| ptr->level < 2*/)
            {
                        if (objB->id == 2572) cout << "object 2572 assigned to the level leaf " << ptr->level << " number " << nextNode->childIndex << endl;
                ptr->attachedObjs[1].push_back(objB);
                //Expand the prevnode->mbrSelfD[1]
                //Expanding the mbrL[1] to include the above A assigned object
                nextNode->mbrSelfD[1]=FLAT::Box::combineSafe(objMBR,nextNode->mbrSelfD[1]);
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
FLAT::uint64 reTOUCH::mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr)
{
        TreeNode* ptr = tree[startEntry->childIndex];
        FLAT::uint64 numA=0;
        if (ptr->leafnode)
        {
                mbr.isEmpty=true;
                //mbrL[0]=new Box(true);
                return numA;
        }

        //fixing MBR of the descendant nodes' objects
        //numA+=ptr->attachedObjs[0].size()+ptr->attachedObjsAns[0].size();
        for (std::vector<TreeEntry*>::iterator ent=ptr->entries.begin();ent!=ptr->entries.end();++ent)
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
                //if (objA->id == 5579) cout << "5579 assigned to the level because no down overlap " << ptr->level << endl;
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
                        ptr->parentEntry = nextNode;
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
                //if (objA->id == 5579) cout << "objA assigned to the level because many down overlaps" << ptr->level << endl;
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
                    //if (objA->id == 5579) cout << "objA assigned to the level because one overlap" << ptr->level << endl;
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
                tree[Ansptr->childIndex]->attachedObjsAns[0].push_back(objA);
                //if (objA->id == 5579) cout << "objA assigned to ancestor to the level " << tree[Ansptr->childIndex]->level << "   node: " << Ansptr->childIndex << endl;
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

    //Fixing the mbrs of dataset B after assign
    root->mbrL[0].isEmpty=true;
    root->num[0] = mergingMbrA(root,root->mbrL[0]);
    //printTree(root);

    building.stop();

}

void reTOUCH::joinInternalobjecttodesc(FLAT::SpatialObject* obj, TreeEntry* ancestorNode, bool isA)
{
    queue<TreeEntry*> nodes;
    TreeNode* node, * downnode;
    nodes.push(ancestorNode);
    FLAT::Box objMBR = obj->getMBR();
    objMBR.isEmpty = false;
    FLAT::Box::expand(objMBR,epsilon * 1./2.);
    //if (obj->id == 2572 && ancestorNode->childIndex == 34) cout << "check 2572 with 34" << endl;
    while(nodes.size()>0)
    {
        //start from checking children, each for intersection of MBR
        // then if intersects - check the assign objects of child
        // and if it is not a leaf node and intersects -> add to the queue


        node = tree[nodes.front()->childIndex];
        nodes.pop();


        if (isA)
        {
            if (obj->id == 2572 && ancestorNode->childIndex == 34) cout << "is A" << endl;
            //intersect with all non-null children
            for (FLAT::uint64 child = 0; child < node->entries.size(); ++child)
            {
                downnode = tree[node->entries[child]->childIndex];
                //if intersects
                if (FLAT::Box::overlap(objMBR, node->entries[child]->mbrSelfD[1]))
                {
                    ItemsMaxCompared += downnode->attachedObjs[1].size();
                    comparing.start();
                    if(localJoin == algo_SGrid)
                    {
                        downnode->spatialGridHash[1]->probe(obj);
                    }
                    else
                    {
                        JOIN(obj, downnode->attachedObjs[1]);
                    }
                    comparing.stop();
                }
                if (FLAT::Box::overlap(objMBR, node->entries[child]->mbrL[1]))
                {
                        //add child to the queue if it is not a leaf (even in dark)
                        if (!downnode->leafnode)
                                nodes.push(node->entries[child]);
                }
            }
        }
        else
        {
            if (obj->id == 2572 && ancestorNode->childIndex == 34) cout << "is B" << endl;
            for (FLAT::uint64 child = 0; child < node->entries.size(); ++child)
            {
                //if intersects
                
                    //if (node->entries[child]->childIndex == 58 && obj->id == 2572) cout << "check 58" << endl;
                downnode = tree[node->entries[child]->childIndex];
                if (node->entries[child]->childIndex == 58 && obj->id == 2572) cout << node->entries[child]->mbrSelfD[0] << " obj " << objMBR << endl;
                if (FLAT::Box::overlap(objMBR, node->entries[child]->mbrSelfD[0]))
                {

                    //if (node->entries[child]->childIndex == 58 && obj->id == 2572) cout << "check down 58" << endl;
                    comparing.start();
                    ItemsMaxCompared += downnode->attachedObjs[0].size()+downnode->attachedObjsAns[0].size();
                    
//                    int t1 = downnode->spatialGridHashAns[0]->resultPairs.results;
//                    downnode->spatialGridHashAns[0]->probe(obj);
//                    int t2 = downnode->spatialGridHashAns[0]->resultPairs.results;
//                    int t3 = this->resultPairs.results;
//                    JOIN(obj, downnode->attachedObjsAns[0]);
//                    int t4 = this->resultPairs.results;
//                    if (t2-t1 != t4-t3)
//                    {
//                        cout << "-------Error: " << t2-t1 << " : " << t4-t3 << endl;
//                    }
                    if(localJoin == algo_SGrid)
                    {
                        downnode->spatialGridHash[0]->probe(obj);
                        downnode->spatialGridHashAns[0]->probe(obj);
                    }
                    else
                    {
                        JOIN(obj, downnode->attachedObjs[0]);
                        JOIN(obj, downnode->attachedObjsAns[0]);
                    }
                    comparing.stop();
                }
                if (FLAT::Box::overlap(objMBR, node->entries[child]->mbrL[0]))
                {
                        //add child to the queue if it is not a leaf (even in dark)
                        if (!downnode->leafnode)
                                nodes.push(node->entries[child]);
                }

            }
        }

    }
}

void reTOUCH::joinIntenalnodetoleafs(TreeEntry* ancestorNode)
{
    //check for intersection all nodes below, where smth was attached with opposite color.
    

    
    // here @todo check function and parallelize
    /*
     * A -> B_below
     */
       // if (ancestorNode->childIndex == 34) cout << "found 34" << endl;
    TreeNode* node=tree[ancestorNode->childIndex];
    for (SpatialObjectList::iterator it = node->attachedObjs[0].begin();
                                                    it != node->attachedObjs[0].end(); it++)
    {
        joinInternalobjecttodesc((*it), ancestorNode,true);
    }

    /*
     * B -> A_below
     */
    for (SpatialObjectList::iterator it = node->attachedObjs[1].begin();
                                                    it != node->attachedObjs[1].end(); it++)
    {
       // if ((*it)->id == 2572) cout << "check 2572" << endl;
        joinInternalobjecttodesc((*it), ancestorNode,false);
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
                mbr.isEmpty = false;
                FLAT::Box::expand(mbr,epsilon * 1./2.);
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[1]))
                {
                    JOIN((*it), node->attachedObjs[1]);
                }
                comparing.stop();
            }
            for (SpatialObjectList::iterator it = node->attachedObjsAns[0].begin();
                                                            it != node->attachedObjsAns[0].end(); it++)
            {
                FLAT::Box mbr = (*it)->getMBR();
                mbr.isEmpty = false;
                FLAT::Box::expand(mbr,epsilon * 1./2.);
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[1]))
                {
                    JOIN((*it), node->attachedObjs[1]);
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
                mbr.isEmpty = false;
                FLAT::Box::expand(mbr,epsilon * 1./2.);
                comparing.start();
                if (FLAT::Box::overlap(mbr, ancestorNode->mbrSelfD[0]))
                {
                        JOIN((*it), node->attachedObjs[0]);
                        JOIN((*it), node->attachedObjsAns[0]);
                }
                comparing.stop();

            }
        }
    }


}

FLAT::uint64 reTOUCH::mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr)
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
        (*ent)->mbrL[0].isEmpty = true;
        (*ent)->num[1] = mergingMbrB((*ent), (*ent)->mbrL[1]);
        numB += (*ent)->num[1];
        mbr = FLAT::Box::combineSafe((*ent)->mbrL[1],mbr);
        mbr = FLAT::Box::combineSafe((*ent)->mbrSelfD[1],mbr);
    }
    return numB;
}