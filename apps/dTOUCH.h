/* 
 * File:   dTOUCH.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 22:51
 */

#ifndef DTOUCH_H
#define	DTOUCH_H

#include "TOUCHlike.h"

using namespace std;
using namespace FLAT;

// The class for doing OUR Spatial Join, TOUCH: Spatial Hierarchical Hash Join
class dTOUCH : TOUCHlike
{

private:

	uint64 totalGridCells;
	vector<TreeNode*> treeA;		// Append only structure can be replaced by a file "payload"
    vector<TreeNode*> treeB;
	vector<TreeEntry*> nextInput;
	TreeEntry* rootA;
    TreeEntry* rootB;

    void writeNode(vector<TreeEntry*> objlist,int Level,vector<TreeNode*>& tree);
    void createTreeLevel(vector<TreeEntry*>& input,int Level,vector<TreeNode*>& tree);

public:
    
    uint64 maxAssignmentLevel = 3;

	dTOUCH(unsigned int buckets);
	~dTOUCH();
	void createPartitionsA();
    void createPartitionsB();
        
	void assignmentA(const SpatialObjectList& ds);
    void assignmentBrc(TreeEntry* targetEntry);
    void assignmentB();

	void joinIntenalnodetoleafs(uint64 ancestorNodeID, vector<TreeNode*>& tree, TreeEntry* root);
	void probe();
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
};


#endif	/* DTOUCH_H */

