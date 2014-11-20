/*
 * File: reTOUCH.h
 * 
 * reTOUCH modification of TOUCH algorithm
 * Do the spatial join by reconstructing tree on assigned objects
 * 
 */

#include "TOUCHlike.h"


class reTOUCH : public TOUCHlike
{
private:

    void NL(SpatialObjectList& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
            for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
                if ( istouching(*itA , *itB) )
                {
                    resultPairs.addPair(*itA , *itB);
                }
    }
    
    void PS(SpatialObjectList& A, SpatialObjectList& B)
    {
        
	//Sort the datasets based on their lower x coordinate
	sorting.start();
	std::sort(A.begin(), A.end(), Comparator_Xaxis());
	//cout<<"Sort A Done."<<endl;
	std::sort(B.begin(), B.end(), Comparator_Xaxis());
	//cout<<"Sort B Done."<<endl;
	sorting.stop();

	//sweep
	FLAT::uint64 iA=0,iB=0;
	while(iA<A.size() && iB<B.size())
	{
		//cout<<iA<< " " <<iB<<endl;
		if(A.at(iA)->getMBR().low[0] < B.at(iB)->getMBR().low[0])
		{
			FLAT::uint64 i = iB;
			FLAT::spaceUnit border = A.at(iA)->getMBR().high[0]+epsilon;
			while(i<B.size() && B.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(i) , A.at(iA)) )
				{
                                    resultPairs.addPair(B.at(i),A.at(iA));
				}

				i++;
			}
			iA++;
		}
		else
		{
			FLAT::uint64 i = iA;
			FLAT::spaceUnit border = B.at(iB)->getMBR().high[0]+epsilon;
			while(i<A.size() &&  A.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(iB) , A.at(i)) )
				{
                                    resultPairs.addPair(B.at(i),A.at(iA));
				}

				i++;
			}
			iB++;
		}
	}
    }

    //JOIN for reTOUCH <--------
    //The local join for joining two buckets, add the results to set in order to remove the duplicates
    void JOIN(SpatialObjectList& A, SpatialObjectList& B)
    {
            Ljoin.start();
            //cout<<"join " << A.size() << " and " <<B.size()<<endl;
            if(localJoin == algo_NL)
                    NL(A,B);
            else
                    PS(A,B);
            Ljoin.stop();
    }
//    //from cTOUCH @todo migrate all to JoinAlg
    void JOIN(FLAT::SpatialObject* obj, SpatialObjectList& B)
    {
        Ljoin.start();
        if(localJoin == algo_NL)
            NL(obj,B);
        else
            PS(obj,B);
        Ljoin.stop();
    }
    
        //Plane-sweeping join algorithm for cTOUCH <--------------
    void PS(FLAT::SpatialObject* A, SpatialObjectList& B)
    {
        //@todo sorting???
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , (*itB)) )
                resultPairs.addPair( A,(*itB) );
    }
    //Nested Loop join algorithm cTOUCH
    void NL(FLAT::SpatialObject* A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }

    /*
    Create new node according to set of TreeEntries. Entries can be of both types,
    So create to entries that point to the new node of two types.
    Create entry iff it is not empty
    */
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    void createTreeLevel(std::vector<TreeEntry*>& input,int Level);

public:

    reTOUCH()
    {
        algorithm = algo_reTOUCH; //@todo do it for all
        total.start(); // timing
    }

    ~reTOUCH()
    {
            delete &tree;
            total.stop();
    }
    void createPartitions()
    {
            partition.start();
            // Build PRtree levels from bottom up
            // Build PRtree levels from bottom up

            Levels = 0;
            totalnodes = 0;
            while(vdsA.size()>1)
            {
                    cout << "Tree Level: " << Levels << " treeNodes: " << tree.size() << " Remaining Input: " << vdsA.size() <<endl;
                    createTreeLevel(vdsA,Levels++);      // writes final nodes in tree and next level in nextInput
                    swap(vdsA,nextInput);					// swap input and nextInput list
                    nextInput.clear();
            }

            root = vdsA.front();

            cout << "Levels " << Levels << endl;


            partition.stop();
            cout << "creating the partitions time: " << partition << endl;

            // Destroy HTree
    }

    void assignmentA();
    void assignmentB();

    void joinInternalobjecttodesc(FLAT::SpatialObject* obj, TreeEntry* ancestorNode, bool isA);

    void joinIntenalnodetoleafs(TreeEntry* ancestorNode);

    void probe();

    void analyze();
    
    
    /*
     * Function for creating valid R-tree from B objects assigned to nodes
     * after A objects removal from leafs
     */
    FLAT::uint64 mergingMbrB(TreeEntry* startEntry, FLAT::Box &mbr);
    FLAT::uint64 mergingMbrA(TreeEntry* startEntry, FLAT::Box &mbr);
    void joinBtoDesA(FLAT::uint64 BID);
    void joinAtoDesB(FLAT::uint64 AID);
    
    std::vector<FLAT::uint64> ItemPerLevelA,ItemPerLevelB,ItemPerLevelAans;
};