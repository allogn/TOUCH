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
private:

    void NL(SpatialObjectList& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
            for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
                if ( istouching(*itA , *itB) )
                {
                    resultPairs.addPair(*itA , *itB);
                }
    }
    
    //Nested Loop join algorithm cTOUCH
    void NL(FLAT::SpatialObject* A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }

    /*
    Create new node according to set of TreeEntries. Entries can be of both types,
    So create to entries that point to the new node of two types.
    Create entry iff it is not empty
    */
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    //void createTreeLevel(std::vector<TreeEntry*>& input,int Level);

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

    void probe();
    void analyze();
    
    
    /*
     * Function for creating valid R-tree from B objects assigned to nodes
     * after A objects removal from leafs
     */
    FLAT::uint64 mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr, bool clearA);
    FLAT::uint64 mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr);
    
    bool verifyMBR(TreeEntry* ent);
    
    void run();
    
    std::vector<FLAT::uint64> ItemPerLevelA,ItemPerLevelB,ItemPerLevelAans,ItemPerLevelBans;
};
