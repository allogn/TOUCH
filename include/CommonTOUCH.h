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
#include "LocalSpatialGridHash.h"
#include "FlexLocalSpatialGridHash.h"
#include "SpatialGridHash.h"

class CommonTOUCH : public JoinAlgorithm {
public:
    CommonTOUCH();
    virtual ~CommonTOUCH();
    
    unsigned int totalnodes;
    
    NodeList probingList;
    
    void probeDownUp();
    void probeUpDown();
    void probeUpDownFilter();
    void pathWayJoin(TreeNode* node);
    void pathWayJoinDown(TreeNode* node);
    void JoinDownR(TreeNode* node, TreeNode* nodeObj);
    void pathWayJoinFilter(TreeNode* node);
    void pathWayJoinDownFilter(TreeNode* node);
    void JoinDownRFilter(TreeNode* node, TreeNode* nodeObj);
    void JOIN(TreeNode* node, TreeNode* nodeObj);
    void JOINdown(TreeNode* node, TreeNode* nodeObj);
    
    virtual void joinNodeToDesc(TreeNode* ancestorNode);
    virtual void joinObjectToDesc(TreeEntry* obj, TreeNode* ancestorNode);
    void probe();
    
    void countSizeStatistics();
    
    void countSpatialGrid();
    void countSpatialGrid(TreeNode* node);
    void deduplicateSpatialGrid();
    void deduplicateSpatialGrid(TreeNode* node);
    
    unsigned int countObjBelow(TreeNode* node, int type);
    void countObjBelowStart();
    
    virtual void NL(TreeEntry*& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }
    
    virtual void NL(SpatialObjectList& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
            NL((*itA),B);
    }
    
    TreeNode* root;
protected:
    
    /*
    * Create new node according to set of TreeEntries. Entries can be of both types,
    * So create to entries that point to the new node of two types.
    * Create entry iff it is not empty
    */
    virtual void writeNode(SpatialObjectList& objlist);
    virtual void writeNode(NodeList& objlist, int Level);
    void createTreeLevel(SpatialObjectList& input);
    void createTreeLevel(NodeList& input, int Level);
    void createPartitions(SpatialObjectList& vds);
    
    void analyze();
    
    NodeList tree;
    NodeList nextInput;
};

#endif	/* CommonTOUCH_H */

