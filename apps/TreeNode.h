/* 
 * File:   TreeNode.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 14:14
 */

#ifndef TREENODE_H
#define	TREENODE_H

class TreeNode
{
public:
	std::vector<TreeEntry*> entries;
	TreeEntry* parentEntry;
	bool leafnode;
	int level;
	
	SpatialObjectList attachedObjs[types]; //for cTOUCH. ONLY DARK TREE
	// make a Leaf node
	TreeNode(int Level)
	{
		level = Level;
		if (Level==0) leafnode = true;
		else leafnode = false;
		parentEntry = 0; //for cTOUCH
	}
};

#endif	/* TREENODE_H */

