/* 
 * File:   TOUCHlikeAlgorithm.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 13:45
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
    
    printTOUCH();
    
protected:
    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    int Levels;
    
    vector<uint64> ItemPerLevel; int LVL;
    
    std::vector<TreeNode*> tree;
    std::vector<TreeEntry*> nextInput;
    TreeEntry* root;
};

#endif	/* TOUCHLIKEALGORITHM_H */

