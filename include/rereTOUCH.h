/*
 * File: rereTOUCH.h
 * 
 * rereTOUCH modification of TOUCH algorithm
 * Do the spatial join by reconstructing tree on assigned objects
 * 
 */

#include "CommonTOUCH.h"


class rereTOUCH : public CommonTOUCH
{
public:

    rereTOUCH()
    {
        algorithm = algo_rereTOUCH; 
    }

    ~rereTOUCH() {}

    void assignmentA();
    void assignmentB();
    void reassignmentB();

    void joinObjectToDesc(FLAT::SpatialObject* obj, TreeEntry* ancestorNode);

    void joinNodeToDesc(TreeEntry* ancestorNode);
    
    /*
     * Function for creating valid R-tree from B objects assigned to nodes
     * after A objects removal from leafs
     */
    FLAT::uint64 mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr, bool clearA);
    FLAT::uint64 mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr);
    
    void run();
    
    std::vector<FLAT::uint64> ItemPerLevelA,ItemPerLevelB,ItemPerLevelAans,ItemPerLevelBans;
};
