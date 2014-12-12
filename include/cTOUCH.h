/*
 * File: cTOUCH.h
 * Author: Alvis Logins
 * 
 * Complex TOUCH modification of TOUCH algorithm
 * Do the spatial join by creating colorful R-Tree
 * 
 */

#include "CommonTOUCH.h"


class cTOUCH : public CommonTOUCH
{
private:

    /*
    Create new node according to set of TreeEntries. Entries can be of both types,
    So create to entries that point to the new node of two types.
    Create entry iff it is not empty
    */
    void writeNode(vector<TreeEntry*> objlist,int Level);

    void assign(TreeNode* ptr, FLAT::SpatialObject* obj); //update parents

public:

    cTOUCH()
    {
        algorithm = algo_cTOUCH;
    }

    ~cTOUCH() {}

    void assignment();

    void joinObjectToDesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID);
    void probe();

    void run();
    void analyze();
};
