/* 
 * File:   TOUCH.h
 * Author: Alvis
 *
 * 
 */

#ifndef TOUCH_H
#define	TOUCH_H

#include <vector>
#include <queue>

#include "TOUCHlike.h"
#include "SpatialGridHash.h"

class TOUCH : public TOUCHlike {
public:
    TOUCH(int buckets);
    virtual ~TOUCH();
    
    void analyze();
    void createPartitions();
    void assignment();
    void probe();
    
    
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
private:
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    void createTreeLevel(vector<TreeEntry*>& input,int Level);
    void joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID);
};

#endif	/* TOUCH_H */

