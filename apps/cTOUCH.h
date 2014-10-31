// cTOUCH: Spatial Hierarchical Hash Join

#include <cmath>
#include "TOUCHlike.h"

using namespace std;
using namespace FLAT;


class cTOUCH : public TOUCHlike
{

private:

	uint64 totalGridCells;
	

    //Plane-sweeping join algorithm for cTOUCH <--------------
    void PS(SpatialObject* A, SpatialObjectList& B)
    {
        //@todo sorting???
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , (*itB)) )
                resultPairs.addPair( A,(*itB) );
    }
    //Nested Loop join algorithm cTOUCH
    void NL(SpatialObject* A, SpatialObjectList& B,  ResultPairs& results)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }
    

    //JOIN for cTOUCH <--------
    void JOIN(SpatialObject* obj, SpatialObjectList& B)
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

public:

	TOUCHJoin(unsigned int buckets)
	{
		initialize.start();

		gridSize = buckets;

		leafsize = ceil((double)vdsAll.size()/(double)buckets);
		nodesize = base;

		initialize.stop();
	}

	~TOUCHJoin()
	{
		delete &tree;
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
        
	void joinInternalobjecttodesc(SpatialObject* obj, uint64 ancestorNodeID);
        
	void joinIntenalnodetoleafs(uint64 ancestorNodeID);
        
	void probe();
        
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
};
