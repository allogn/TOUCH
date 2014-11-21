/* 
 * File:   TreeNode.h
 * Author: Alvis
 *
 */

#ifndef TREENODE_H
#define	TREENODE_H

#include "TreeEntry.h"
#include <vector>

class spatialGridHash();

class TreeNode
{
public:
	std::vector<TreeEntry*> entries;
	TreeEntry* parentEntry;
	bool leafnode;
	int level;
        SpatialGridHash* spatialGridHash[TYPES];    
	
	std::vector<FLAT::SpatialObject*> attachedObjs[TYPES];
        std::vector<FLAT::SpatialObject*> attachedObjsAns;
    
	TreeNode(int Level)
	{
		level = Level;
		if (Level==0) leafnode = true;
		else leafnode = false;
	}
};

#endif	/* TREENODE_H */

