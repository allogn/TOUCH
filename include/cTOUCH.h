/*
 * File: cTOUCH.h
 * Author: Alvis Logins
 * 
 * Complex TOUCH modification of TOUCH algorithm
 * Do the spatial join by creating colorful R-Tree
 * 
 */

#include "TOUCHlike.h"


class cTOUCH : public TOUCHlike
{
private:

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
    

    //JOIN for cTOUCH <--------
    void JOIN(FLAT::SpatialObject* obj, SpatialObjectList& B)
    {
        Ljoin.start();
        if(localJoin == algo_NL)
            NL(obj,B);
        else
            PS(obj,B);
        Ljoin.stop();
    }

		/*
		Create new node according to set of TreeEntries. Entries can be of both types,
		So create to entries that point to the new node of two types.
		Create entry iff it is not empty
		*/
		void writeNode(vector<TreeEntry*> objlist,int Level);
		void createTreeLevel(vector<TreeEntry*>& input,int Level);
                
                void assign(TreeNode* ptr, FLAT::SpatialObject* obj); //update parents

public:

	cTOUCH()
	{
	    algorithm = algo_cTOUCH; //@todo do it for all
            total.start(); // timing
	}

	~cTOUCH()
	{
		delete &tree;
                total.stop();
	}
	void createPartitions()
	{
		partition.start();
		// Build PRtree levels from bottom up
		
		Levels = 0;
		totalnodes = 0;
		while(vdsAll.size()>1)
		{
			cout << "Tree Level: " << Levels << " treeNodes: " << tree.size() << " Remaining Input: " << vdsAll.size() <<endl;
			createTreeLevel(vdsAll,Levels++);      // writes final nodes in tree and next level in nextInput
			swap(vdsAll,nextInput);					// swap input and nextInput list
			nextInput.clear();
		}

		root = vdsAll.front();
				
		cout << "Levels " << Levels << endl;

		partition.stop();
		cout << "creating the partitions time: " << partition << endl;

		// Destroy HTree
	}

	void assignment();
        
	void joinInternalobjecttodesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID);
        
	void joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID);
        
	void probe();
        
	void analyze();
};
