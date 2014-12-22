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
    void writeNode(SpatialObjectList& objlist);
    void writeNode(NodeList& nodelist, int Level);

    void assign(TreeNode* ptr, TreeEntry* obj); //update parents

public:

    cTOUCH()
    {
        algorithm = algo_cTOUCH;
    }

    ~cTOUCH() {
    
    }

    void assignment();

    void joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode);

    void run();
};
