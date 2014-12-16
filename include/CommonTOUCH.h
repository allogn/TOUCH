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
    
    void saveLog();
    
    unsigned int totalnodes;
    int Levels;
    int LevelsD;
    
    //for logging
    thrust::host_vector<int> levelAssigned[TYPES];
    thrust::host_vector<double> levelAvg[TYPES];
    thrust::host_vector<double> levelStd[TYPES];
    
    
    virtual void joinNodeToDesc(TreeNode* ancestorNode);
    virtual void joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode);
    void probe();
    
    void countSizeStatistics();
    
    void countSpatialGrid();
    void deduplicateSpatialGrid();
    
    void NL(TreeEntry*& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }
    
    TreeNode* root;
protected:
    
    /*
    * Create new node according to set of TreeEntries. Entries can be of both types,
    * So create to entries that point to the new node of two types.
    * Create entry iff it is not empty
    */
    void writeNode(SpatialObjectList& objlist);
    void writeNode(NodeList& objlist, int Level);
    void createTreeLevel(SpatialObjectList& input);
    void createTreeLevel(NodeList& input, int Level);
    void createPartitions(SpatialObjectList vds);
    
    void analyze();
    
    NodeList tree;
    NodeList nextInput;
};

#endif	/* CommonTOUCH_H */

