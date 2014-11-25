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

#include <vector>
#include <queue>
#include <cmath>
#include <unistd.h> // check if log file exists

#include "JoinAlgorithm.h"
#include "TreeNode.h"
#include "TreeEntry.h"
#include "SpatialGridHash.h"

class TOUCHlike : public JoinAlgorithm {
public:
    TOUCHlike();
    virtual ~TOUCHlike();
    
    void printTOUCH();
    
    int PartitioningType;	// Sorting algorithm
    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    int Levels;
    
    
    void countSpatialGrid();
    void deduplicateSpatialGrid();
    
    void JOIN(TreeNode* node, SpatialObjectList& B)
    {
        Ljoin.start();
        if(localJoin == algo_NL)
            NL(node,B);
        else
            PS(node,B);
        Ljoin.stop();
    }
    //Nested Loop join algorithm
    void NL(TreeNode* node, SpatialObjectList& B)
    {
        for(vector<TreeEntry*>::iterator itA = node->entries.begin(); itA != node->entries.end(); ++itA)
            for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
                if ( istouching((*itA)->obj , *itB) )
                    resultPairs.addPair( (*itA)->obj , *itB );
    }

    //Plane-sweeping join algorithm
    void PS(TreeNode* node, SpatialObjectList& B)
    {
        //Sort the datasets based on their lower x coordinate
        sorting.start();
        std::sort(node->entries.begin(), node->entries.end(), ComparatorTree_Xaxis());
        //cout<<"Sort A Done."<<endl;
        std::sort(B.begin(), B.end(), Comparator_Xaxis());
        //cout<<"Sort B Done."<<endl;
        sorting.stop();

        //sweep
        FLAT::uint64 iA=0,iB=0;
        while(iA<node->entries.size() && iB<B.size())
        {
            //cout<<iA<< " " <<iB<<endl;
            if(node->entries.at(iA)->mbr.low[0] < B.at(iB)->getMBR().low[0])
            {
                FLAT::uint64 i = iB;
                FLAT::spaceUnit border = node->entries.at(iA)->mbr.high[0];//+epsilon;
                while(i<B.size() && B.at(i)->getMBR().low[0] < border)
                {
                    if ( istouching(B.at(i) , node->entries.at(iA)->obj) )
                        resultPairs.addPair( B.at(i),node->entries.at(iA)->obj );
                    i++;
                }
                iA++;
            }
            else
            {
                FLAT::uint64 i = iA;
                FLAT::spaceUnit border = B.at(iB)->getMBR().high[0];//+epsilon;
                while(i<node->entries.size() &&  node->entries.at(i)->mbr.low[0] < border)
                {
                    if ( istouching(B.at(iB) , node->entries.at(i)->obj) )
                        resultPairs.addPair( B.at(iB),node->entries.at(i)->obj );
                    i++;
                }
                iB++;
            }
        }
    }   
    
    vector<FLAT::uint64> ItemPerLevel; int LVL; //for statistics
protected:
    
    
    std::vector<TreeNode*> tree;
    std::vector<TreeEntry*> nextInput;
    TreeEntry* root;
};

#endif	/* TOUCHLIKEALGORITHM_H */

