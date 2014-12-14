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
        {
            if ((*itB)->id == 9715 && A->id == 8147) cout << "check with 9715" << endl;
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
        }
    }

    /*
    Create new node according to set of TreeEntries. Entries can be of both types,
    So create to entries that point to the new node of two types.
    Create entry iff it is not empty
    */
    void writeNode(std::vector<TreeEntry*> objlist,int Level);

public:

    reTOUCH()
    {
        algorithm = algo_reTOUCH;
    }

    ~reTOUCH() {}

    void assignmentA();
    void assignmentB();

    void joinObjectToDesc(FLAT::SpatialObject* obj, TreeEntry* ancestorNode);

    void joinNodeToDesc(FLAT::uint64 ancestorNodeID);

    void probe();
    void analyze();
    
    void run();
    
    /*
     * Function for creating valid R-tree from B objects assigned to nodes
     * after A objects removal from leafs
     */
    FLAT::uint64 mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr);
    FLAT::uint64 mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr);
    
    std::vector<FLAT::uint64> ItemPerLevelA,ItemPerLevelB,ItemPerLevelAans;
};
