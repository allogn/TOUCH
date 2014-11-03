/* 
 * File:   TOUCHlikeAlgorithm.h
 * Author: Alvis Logins
 *
 * Intermediate level between TOUCH-like spatial algorithms and JoinAlgorithm.
 * Accumulates all common features of TOUCH-like algorithms.
 * 
 */

#ifndef TOUCHLIKEALGORITHM_H
#define	TOUCHLIKEALGORITHM_H

#include "JoinAlgorithm.h"
#include "TreeNode.h"
#include "TreeEntry.h"

class TOUCHlike : public JoinAlgorithm {
public:
    TOUCHlike();
    virtual ~TOUCHlike();
    
    void printTOUCH();
    
    int PartitioningType;	// Sorting algorithm
    int localPartitions;	//The local join resolution
    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    int Levels;
    
protected:
    
    vector<FLAT::uint64> ItemPerLevel; int LVL; //for statistics
    
    std::vector<TreeNode*> tree;
    std::vector<TreeEntry*> nextInput;
    TreeEntry* root;
};

#endif	/* TOUCHLIKEALGORITHM_H */

