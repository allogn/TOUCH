/* 
 * File:   CommonTOUCH.h
 * Author: Alvis Logins
 *
 * Intermediate level between TOUCH-like spatial algorithms and JoinAlgorithm.
 * Accumulates all common features of TOUCH-like algorithms.
 * 
 */

#ifndef CommonTOUCH_H
#define CommonTOUCH_H

#include <vector>
#include <queue>
#include <cmath>
#include <unistd.h> // check if log file exists

#include "JoinAlgorithm.h"
#include "TreeNode.h"
#include "TreeEntry.h"
#include "SpatialGridHash.h"

class CommonTOUCH : public JoinAlgorithm {
public:
    CommonTOUCH();
    virtual ~CommonTOUCH();
    
    void printTOUCH();
    
    int PartitioningType;	// Sorting algorithm
    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    int Levels;
    
    //for dTOUCH
    double maxLevelCoef;
    
    virtual void joinNodeToDesc(FLAT::uint64 ancestorNodeID);
    virtual void joinObjectToDesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID);
    void probe();
    
    void countSpatialGrid();
    void deduplicateSpatialGrid();
    
    void NL(FLAT::SpatialObject*& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }
    
    //Nested Loop join algorithm
    void NL(FLAT::SpatialObject*& A, TreeNode* node)
    {
        for(vector<TreeEntry*>::iterator itA = node->entries.begin(); itA != node->entries.end(); ++itA)
            if ( istouching(A , (*itA)->obj ) )
                resultPairs.addPair( A , (*itA)->obj );
    }
    
    //Nested Loop join algorithm
    void NL(TreeNode* node, SpatialObjectList& B)
    {
        for(vector<TreeEntry*>::iterator itA = node->entries.begin(); itA != node->entries.end(); ++itA)
            NL((*itA)->obj, B);
    }
    
    vector<FLAT::uint64> ItemPerLevel; int LVL; //for statistics
    TreeEntry* root;
protected:
    
    /*
    * Create new node according to set of TreeEntries. Entries can be of both types,
    * So create to entries that point to the new node of two types.
    * Create entry iff it is not empty
    */
    void writeNode(vector<TreeEntry*> objlist, int Level);
    void createTreeLevel(vector<TreeEntry*>& input, int Level);
    void createPartitions(std::vector<TreeEntry*> vds);
    
    std::vector<TreeNode*> tree;
    std::vector<TreeEntry*> nextInput;
};

#endif	/* CommonTOUCH_H */

