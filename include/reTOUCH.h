/*
 * File: reTOUCH.h
 * 
 * reTOUCH modification of TOUCH algorithm
 * Do the spatial join by reconstructing tree on assigned objects
 * 
 */

#include "CommonTOUCH.h"


class reTOUCH : public CommonTOUCH
{
public:

    reTOUCH()
    {
        algorithm = algo_reTOUCH;
    }

    ~reTOUCH() {}

    void assignmentA();
    void assignmentB();

    void joinObjectToDesc(TreeNode* obj, TreeNode* ancestorNode);

    void joinNodeToDesc(FLAT::uint64 ancestorNodeID);
    
    void run();
    
    /*
     * Function for creating valid R-tree from B objects assigned to nodes
     * after A objects removal from leafs
     */
    FLAT::uint64 mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr);
    FLAT::uint64 mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr);
    
    std::vector<FLAT::uint64> ItemPerLevelA,ItemPerLevelB,ItemPerLevelAans;
};
