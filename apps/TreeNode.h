/* 
 * File:   TreeNode.h
 * Author: Alvis
 *
 */

#ifndef TREENODE_H
#define	TREENODE_H

#include "TreeEntry.h"
#include <vector>

class TreeNode
{
public:
	std::vector<TreeEntry*> entries;
	TreeEntry* parentEntry;
	bool leafnode;
	int level;
	
	vector<SpatialObject*> attachedObjs[types];
    
	TreeNode(int Level)
	{
		level = Level;
		if (Level==0) leafnode = true;
		else leafnode = false;
	}
};

#endif	/* TREENODE_H */

