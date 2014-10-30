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
#include "Statistics.h"

class TOUCHlike : public JoinAlgorithm {
public:
    TOUCHlikeAlgorithm();
    TOUCHlikeAlgorithm(const TOUCHlikeAlgorithm& orig);
    virtual ~TOUCHlikeAlgorithm();
    
protected:
    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    int Levels;
    
    std::vector<TreeNode*> tree;
    std::vector<TreeEntry*> nextInput;
    TreeEntry* root;
        
private:
    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    int Levels;
};

#endif	/* TOUCHLIKEALGORITHM_H */

