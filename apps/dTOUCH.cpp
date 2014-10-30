#include "DataFileReader.hpp"
#include "SpatialQuery.hpp"
#include "Segment.hpp"
#include "hilbert.hpp"
#include <set>
#include <list>
#include <stack>
#include <queue>
#include <algorithm>
#include <limits>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#define MAX_ASSIGNMENT_LEVEL 3

using namespace FLAT;
using namespace std;

typedef boost::unordered_set<SpatialObject*> SpatialObjectSet;
typedef vector<SpatialObject*> SpatialObjectList;
typedef SpatialObjectList HashValue;
typedef pair<uint64,HashValue*> ValuePair;
typedef boost::unordered_map <uint64,HashValue*> HashTable;
typedef pair<SpatialObject*,SpatialObject*> ResultPair;
typedef boost::unordered_set< ResultPair > ResultList;

//Global variables
int localPartitions = 100;					//The local join resolution
bool verbose				=  false;		// Output everything or not?
int algorithm				=  algo_TOUCH;		// Choose the algorithm
int localJoin				=  algo_NL;		// Choose the algorithm for joining the buckets, The local join
int PartitioningType = 1;				//0:No Sort; 1:Hilbert Sort; 2:X axis; 3:STR
int runs					=  1;				// # of runs
double epsilon				=  1.5;				// the epsilon of the similarity join
int partitions				=  4;				// # of partitions: in S3 is # of levels; in SGrid is resolution
//string file_dsA = "c:\\Project\\Brain\\data\\RawData-segment-object-10000.bin";
//string file_dsB = "c:\\Project\\Brain\\data\\RawData-segment-object-10000.bin";
//string file_dsA = "..//data//RawData-segment-object-1M-Axons.bin";
//string file_dsB = "..//data//RawData-segment-object-1M-Dendrites.bin";
string file_dsA = "..//data//RandomData-100K.bin";
string file_dsB = "..//data//RandomData-1600K.bin";

SpatialObjectList dsA, dsB;					//A is smaller than B

DataFileReader *inputA, *inputB;
string logfilename = "SJ.LOG";
unsigned int numA = 0 ,numB = 0;		//number of elements to be read from datasets
Box universeA, universeB;
Timer dataLoad;	//The time for copying the data to memory
Timer Ljoin;// The time for local join
/*
 *Only for testing and debugging purpose
 */
vector<uint64> ItemPerLevel; int LVL;
/*
 * End
 */

#define PAGE_SIZE 4096 // 4096
#define OBJECT_SIZE 48
#define NODE_FANOUT PAGE_SIZE/(OBJECT_SIZE+8)
#define LEAF_FANOUT PAGE_SIZE/OBJECT_SIZE


vector<TreeEntry*> vdsA;	//vector of the Objects and their MBRs of the smaller dataset
vector<TreeEntry*> vdsB;
uint64 size_dsA,size_dsB;

// The class for doing OUR Spatial Join, TOUCH: Spatial Hierarchical Hash Join
class dTOUCHJoin
{

private:

	uint64 totalGridCells;
	vector<TreeNode*> treeA;		// Append only structure can be replaced by a file "payload"
    vector<TreeNode*> treeB;
	vector<TreeEntry*> nextInput;
	TreeEntry* rootA;
    TreeEntry* rootB;

	unsigned int leafsize, nodesize;
	unsigned int totalnodes;
	int Levels;


		void writeNode(vector<TreeEntry*> objlist,int Level,vector<TreeNode*>& tree)
		{
			TreeNode* prNode = new TreeNode(Level);
			Box mbr;
			uint64 childIndex;
			totalnodes ++;
			int count =0;
			for (vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
			{
				prNode->entries.push_back(*it);
				if (count==0) mbr = (*it)->mbr;
				else Box::combine((*it)->mbr,mbr,mbr);
				count++;
			}
			childIndex = tree.size();

			tree.push_back(prNode);
			nextInput.push_back(new TreeEntry(mbr,childIndex));
		}

		void createTreeLevel(vector<TreeEntry*>& input,int Level,vector<TreeNode*>& tree)
		{
			unsigned int nodeSize;
			if (Level==0) nodeSize = leafsize;
			else nodeSize = nodesize;

			if(PartitioningType != No_Sort)
			{
				statsTOUCH.sorting.start();
				std::sort(input.begin(),input.end(),Comparator());
				statsTOUCH.sorting.stop();
			}

			if(PartitioningType == STR_Sort)
			{
				//First sort by X then assigns the objects to partitions then sort by Y each partition separately then again assigns the objects to partitions then sort by Z objects of each partition separately
				statsTOUCH.sorting.start();
				cout<< "Node size " << nodeSize << endl;
				uint64 itemsD1 = nodeSize * nodeSize;
				uint64 itemsD2 = nodeSize;
				std::sort(input.begin(),input.end(),Comparator());
				uint64 i=0;
				while(true)
				{
					if((i+1)*itemsD1 < input.size())
						std::sort(input.begin()+i*itemsD1, input.begin()+(i+1)*itemsD1 ,ComparatorY());
					else
					{
						std::sort(input.begin()+i*itemsD1, input.end() ,ComparatorY());
						break;
					}
					i++;
				}
				i=0;
				while(true)
				{
					if((i+1)*itemsD2 < input.size())
						std::sort(input.begin()+i*itemsD2, input.begin()+(i+1)*itemsD2 ,ComparatorZ());
					else
					{
						std::sort(input.begin()+i*itemsD2, input.end() ,ComparatorZ());
						break;
					}
					i++;
				}
				statsTOUCH.sorting.stop();
			}

			cout << "Sort "<< input.size()<< " items in " << statsTOUCH.sorting << endl;


			vector<TreeEntry*> entries;
			for (vector<TreeEntry*>::iterator it=input.begin();it!=input.end();++it)
			{
				if (entries.size()<nodeSize)
				{
					entries.push_back(*it);
					if (entries.size()>=nodeSize)
					{
						writeNode(entries,Level,tree);
						entries.clear();
					}
				}
			}
			if (!entries.empty())
				writeNode(entries,Level,tree);
		}

public:

	TOUCHJoin(unsigned int buckets)
	{
		statsTOUCH.initialize.start();

		statsTOUCH.gridSize = buckets;

		leafsize = ceil((double)vdsA.size()/(double)buckets);
		nodesize = base;

		statsTOUCH.initialize.stop();
	}

	~TOUCHJoin()
	{
		delete &treeA;
		delete &treeB;
	}
	void createPartitionsA()
	{
		statsTOUCH.partition.start();
		// Build PRtree levels from bottom up
		Levels = 0;
		totalnodes = 0;
		while(vdsA.size()>1)
		{
			cout << "Tree A Level: " << Levels << " treeNodes: " << treeA.size() << " Remaining Input: " << vdsA.size() <<endl;
			createTreeLevel(vdsA,Levels++,treeA);      // writes final nodes in tree and next level in nextInput
			swap(vdsA,nextInput);					// swap input and nextInput list
			nextInput.clear();
		}

		rootA = vdsA.front();

		cout << "Levels " << Levels << endl;

		statsTOUCH.partition.stop();
		cout << "creating the partitions time: " << statsTOUCH.partition << endl;

		// Destroy HTree
	}
    void createPartitionsB()
	{
		statsTOUCH.partition.start();
		// Build PRtree levels from bottom up
		Levels = 0;
		totalnodes = 0;
		while(vdsB.size()>1)
		{
			cout << "Tree B Level: " << Levels << " treeNodes: " << treeB.size() << " Remaining Input: " << vdsB.size() <<endl;
			createTreeLevel(vdsB,Levels++,treeB);      // writes final nodes in tree and next level in nextInput
			swap(vdsB,nextInput);					// swap input and nextInput list
			nextInput.clear();
		}

		rootB = vdsB.front();

		cout << "Levels " << Levels << endl;

		statsTOUCH.partition.stop();
		cout << "creating the partitions time: " << statsTOUCH.partition << endl;

		// Destroy HTree
	}
        
	void assignmentA(const SpatialObjectList& ds)
	{
		statsTOUCH.building.start();
		int overlaps;
                int assigned_level;
		bool assigned;
		for (unsigned int i=0;i<ds.size();++i)
		{
			//if(i%1000==0)cout<<i<<endl;
			SpatialObject* obj = ds.at(i);
			Box objMBR = obj->getMBR();
			//objMBR.low = objMBR.low - epsilon;
			//objMBR.high = objMBR.high + epsilon;

			TreeEntry* nextNode;
			TreeNode* ptr = treeA.at(rootA->childIndex);

			nextNode = NULL;

			while(true)
			{
				overlaps = 0;
				assigned = false;
				for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
					if ( Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) )
					{
						if(overlaps == 0)
						{
							nextNode = ptr->entries.at(cChild);
						}
						else
						{
							//should be assigned to this level
							if(ptr->level > MAX_ASSIGNMENT_LEVEL)
							{
								vdsB.push_back(new TreeEntry(obj));
							}
							else
							{
								ptr->attachedObjs.push_back(obj);
							}
							assigned = true;
                                                        assigned_level = ptr->level;
							break;
						}
						overlaps++;
					}

				if(assigned)
                                {
                                        obj->cost = overlaps*((pow(NODE_FANOUT,assigned_level+1)-1)/(NODE_FANOUT - 1) - 1);
					break;
				}
                                if(!overlaps)
				{
					//filtered
					statsTOUCH.filtered ++;
					break;
				}
				ptr = treeA.at(nextNode->childIndex);
				if(ptr->leafnode /*|| ptr->level < 2*/)
				{
                                    //intersects only with one entry in the leaf
					ptr->attachedObjs.push_back(obj);
                                        obj->cost = 1; // one object only.
					break;
				}
			}
		}

		statsTOUCH.building.stop();
	}
        
    void assignmentBrc(TreeEntry* targetEntry)
	{
		bool overlaps;
		bool assigned;
		TreeNode* targetNode = treeA.at(targetEntry->childIndex);
		if (targetNode->leafnode)
		{
			//if(i%1000==0)cout<<i<<endl;
			for (vector<TreeEntry*>::iterator it = targetNode->entries.begin(); it != targetNode->entries.end(); it++)
			{
				SpatialObject* obj = (*it)->obj;
				Box objMBR = obj->getMBR();
				//objMBR.low = objMBR.low - epsilon;
				//objMBR.high = objMBR.high + epsilon;

				TreeEntry* nextNode;
				TreeNode* ptr = treeB.at(rootB->childIndex);
				nextNode = NULL;

				while(true)
				{
					overlaps = false;
					assigned = false;
					for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
					{
						if ( Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) )
						{
							if(!overlaps)
							{
								overlaps = true;
								nextNode = ptr->entries.at(cChild);
							}
							else
							{
								//should be assigned to this level
								ptr->attachedObjs.push_back(obj);
								//delete object from the first tree
								//put a flag
								assigned = true;
								break;
							}
						}
					}

					if(assigned)
						break;
					if(!overlaps)
					{
						//filtered
						statsTOUCH.filtered ++;
						break;
					}
					ptr = treeB.at(nextNode->childIndex);
					if(ptr->leafnode /*|| ptr->level < 2*/)
					{
						ptr->attachedObjs.push_back(obj);
						break;
					}
					
				}
			}
			
		}
		else
		{
			for(vector<TreeEntry*>::iterator it = targetNode->entries.begin();
					it != targetNode->entries.end(); it++)
			{
				assignmentBrc((*it));
			}
		}
	}
        
    void assignmentB() //using A tree, that was already built
	{
		statsTOUCH.building.start();
                assignmentBrc(rootA);
		statsTOUCH.building.stop();
	}

	//Joins spatial objects of the given vector against spatial objects in the leaf node
	void joinLeafVector(TreeNode* leaf, SpatialObjectList& vect)//What is wrong here ???????
	{

		//statsTOUCH.ItemsCompared += leaf->entries.size()*vect.size();
		//return;


		SpatialObject * obj;
		//cout<<"(" <<leaf->entries.size() << " to " << vect.size();
		for (unsigned int child = 0; child < leaf->entries.size(); ++child)
		{
			obj = leaf->entries.at(child)->obj;
			for(uint64 i=0; i<vect.size(); i++)
			if ( istouching(vect.at(i),obj) )
				//results.addPair( vect.at(i),obj );
				statsTOUCH.results ++;
		}
		//cout<<").";


	}
	void joinIntenalnodetoleafs(uint64 ancestorNodeID, ResultPairs& results, vector<TreeNode*>& tree, TreeEntry* root)
	{
		SpatialGridHash* spatialGridHash;
		queue<uint64> leaves;
		TreeNode* leaf, *ancestorNode;
		ancestorNode = tree.at(ancestorNodeID);
		if( localJoin == algo_SGrid )// && localPartitions < internalObjsCount)// && internal->level >0)
		{
			//constructing the grid for the current internal node that we want to join it with all its desendet leaf nodes
			//spatialGridHash = new SpatialGridHash(localUniverse,localPartitions);
			statsTOUCH.deDuplicate.start();
			//spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
			spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
			spatialGridHash->build(ancestorNode->attachedObjs);
			statsTOUCH.deDuplicate.stop();
		}

		leaves.push(ancestorNodeID);
		while(leaves.size()>0)
		{
			leaf = tree.at(leaves.front());
			leaves.pop();
			if(leaf->leafnode)
			{
				statsTOUCH.comparing.start();
				// join leaf->entries and vect

				if(localJoin == algo_SGrid)// && localPartitions < internalObjsCount)// && internal->level >0)
				{
					ResultList temp;
					spatialGridHash->probe(leaf , temp);
					statsTOUCH.duplicates -= temp.size();
					//The following or statsTOUCH.results += temp.size();
					for (ResultList::iterator it=temp.begin(); it!=temp.end(); ++it)
					{
						results.addPair(it->first,it->second);
					}
				}
				else
					JOIN(leaf,ancestorNode->attachedObjs,results);

				statsTOUCH.comparing.stop();
				//else if(PartitioningType == 2)
					//joinVectorLeaf(leaf,hb,results);
			}
			else
			{
				for (uint64 child = 0; child < leaf->entries.size(); ++child)
				{
					leaves.push(leaf->entries.at(child)->childIndex);
				}
			}
		}
	}
	//Optimized probing function
	void Oprobe(ResultPairs& results)
	{
		//For every cell of A join it with its corresponding cell of B
		statsTOUCH.probing.start();
                
                //one tree
		queue<uint64> Qnodes;

		TreeNode* currentNode;
                Box localUniverse;

                Qnodes.push(rootA->childIndex);
		//uni.push(root->mbr);

		int lvl = Levels;
		// A BFS on the tree then for each find all its leaf nodes by another BFS
		while(Qnodes.size()>0)
		{
			uint64 currentNodeID = Qnodes.front();
			currentNode = treeA.at(currentNodeID);
			Qnodes.pop();
			//BFS on the tree using Qnodes as the queue for memorizing the future nodes to be traversed
			if(!currentNode->leafnode)
			for (uint64 child = 0; child < currentNode->entries.size(); ++child)
			{
				Qnodes.push(currentNode->entries.at(child)->childIndex);
			}
			//join the internal node with all *its* leaves

			// If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
			if(currentNode->attachedObjs.size()==0)
				continue;

			// just to display the level of the BFS traversal
			if(lvl!=currentNode->level)
			{
				lvl = currentNode->level;
				cout << "\n### Level " << lvl <<endl;
			}

			joinIntenalnodetoleafs(currentNodeID, results, treeA, rootA);

			//if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
		}
                
		//second tree
		//Qnodes is empty

		Qnodes.push(rootB->childIndex);

		lvl = Levels;
		// A BFS on the tree then for each find all its leaf nodes by another BFS
		while(Qnodes.size()>0)
		{
			uint64 currentNodeID = Qnodes.front();
			currentNode = treeB.at(currentNodeID);
			Qnodes.pop();
			//BFS on the tree using Qnodes as the queue for memorizing the future nodes to be traversed
			if(!currentNode->leafnode)
			for (uint64 child = 0; child < currentNode->entries.size(); ++child)
			{
				Qnodes.push(currentNode->entries.at(child)->childIndex);
			}
			//join the internal node with all *its* leaves

			// If the current node has no objects assigned to it, no join is needed for the current node to the leaf nodes.
			if(currentNode->attachedObjs.size()==0)
				continue;

			// just to display the level of the BFS traversal
			if(lvl!=currentNode->level)
			{
				lvl = currentNode->level;
				cout << "\n### Level " << lvl <<endl;
			}

			joinIntenalnodetoleafs(currentNodeID, results, treeB, rootB);

			//if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
			//	delete spatialGridHash;
		}
                
		statsTOUCH.probing.stop();

	}
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
	{

		statsTOUCH.analyzing.start();
		uint64 emptyCells=0;
		uint64 sum=0,sqsum=0;
		double differenceSquared=0;
		statsTOUCH.footprint += vdsA.size()*(sizeof(TreeEntry*));
		statsTOUCH.footprint += dsB.size()*(sizeof(SpatialObject*));
		LVL = Levels;
		//vector<uint64> ItemPerLevel;
                
                //tree A
                cout << "== TREE A (B assigned) ==" << endl;
		ItemPerLevel.reserve(Levels);
		for(int i = 0 ; i<Levels ; i++)
			ItemPerLevel.push_back(0);
		for(unsigned int ni = 0; ni<treeA.size() ; ni++)
		{
			SpatialObjectList objs = treeA.at(ni)->attachedObjs;
			uint64 ptrs = objs.size();
			if(objs.size()==0)emptyCells++;
			ItemPerLevel[treeA.at(ni)->level]+=ptrs;
			sum += ptrs;
			sqsum += ptrs*ptrs;
			if (statsTOUCH.max<ptrs) statsTOUCH.max = ptrs;

		}
		for(int i = 0 ; i<Levels ; i++)
			cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;
                
                cout << "== TREE B (A assigned) ==" << endl; //@todo
		ItemPerLevel.reserve(Levels);
		for(int i = 0 ; i<Levels ; i++)
			ItemPerLevel.push_back(0);
		for(unsigned int ni = 0; ni<treeB.size() ; ni++)
		{
			SpatialObjectList objs = treeB.at(ni)->attachedObjs;
			uint64 ptrs = objs.size();
			if(objs.size()==0)emptyCells++;
			ItemPerLevel[treeB.at(ni)->level]+=ptrs;
			sum += ptrs;
			sqsum += ptrs*ptrs;
			if (statsTOUCH.max<ptrs) statsTOUCH.max = ptrs;

		}
		for(int i = 0 ; i<Levels ; i++)
			cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;
                
		stats.footprint += sum*sizeof(SpatialObject*) + treeB.size()*(sizeof(TreeNode*)) + treeA.size()*(sizeof(TreeNode*));
		statsTOUCH.avg = (sum+0.0) / (treeB.size() + treeA.size());
		statsTOUCH.percentageEmpty = (emptyCells+0.0) / (treeB.size() + treeA.size())*100.0;
		differenceSquared = ((double)sqsum/((double)treeB.size() + (double)treeA.size()))-statsTOUCH.avg*statsTOUCH.avg;
		statsTOUCH.std = sqrt(differenceSquared);
                
                
		statsTOUCH.analyzing.stop();
	}
};
