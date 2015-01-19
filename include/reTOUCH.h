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

    void joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode);

    void joinNodeToDesc(TreeNode* ancestorNode);
    
    void run();
    
    /*
     * Function for creating valid R-tree from B objects assigned to nodes
     * after A objects removal from leafs
     */
    FLAT::uint64 mergingMbrB(TreeNode* startNode, FLAT::Box &mbr);
    FLAT::uint64 mergingMbrA(TreeNode* startNode, FLAT::Box &mbr);
    
    thrust::host_vector<FLAT::uint64> ItemPerLevelA,ItemPerLevelB,ItemPerLevelAans;
};
