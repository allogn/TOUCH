/* 
 * File:   dTOUCH.h
 * Author: Alvis
 *
 */

#ifndef DTOUCH_H
#define	DTOUCH_H

#include "TOUCHlike.h"
#include "SpatialGridHash.h"

class dTOUCH : public TOUCHlike
{

private:

    vector<TreeNode*> treeA;
    vector<TreeNode*> treeB;
    TreeEntry* rootA;
    TreeEntry* rootB;
    int LevelsA;
    int LevelsB;
    int LVLA; // for output statistics
    int LVLB;
    vector<FLAT::uint64> ItemPerLevelA;
    vector<FLAT::uint64> ItemPerLevelB;

    void writeNode(vector<TreeEntry*> objlist,int Level,vector<TreeNode*>& tree);
    void createTreeLevel(vector<TreeEntry*>& input,int Level,vector<TreeNode*>& tree);

public:
    
    int maxAssignmentLevel; //@todo get from input params

    dTOUCH();
    ~dTOUCH();
    void createPartitionsA();
    void createPartitionsB();
        
    void assignmentA(); // assignment of objects B to tree A
    void assignmentB();

    void joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID, vector<TreeNode*>& tree, TreeEntry* root, FLAT::Box universe);
    void probe();
    void analyze();
    
};


#endif	/* DTOUCH_H */

