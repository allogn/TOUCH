#ifndef N_TREE_
#define N_TREE_

#include <iostream>
#include <algorithm>
#include "Vertex.cpp"
#include "Box.cpp"
#include "Query.cpp"
#include "Data.cpp"
#include "Analyzer.cpp"

#define SPLITS 64 // 3x3x3
#define SPLIT_PER_DIMENSION 4
#define HEIRISTIC false
#define MAXDATA 64

using namespace std;
/*
 * Ntree Node
 * Based on Octree
 * with N splits on demand
 * each Leaf is a bucket of Data with MAX DATA size
 *
 * Author: Farhan Tauheed
 */

class NtreeNode
	{
	public:
	Box extent;
	int totalLeaves;
	int minDepth;
	int leafCount;
	bool isLeaf;
	int Level; // 1 is root
	NtreeNode* children[SPLITS];
	Data* leaves[MAXDATA]; //Data pointers in all Nodes not good

	NtreeNode(const Box &box, int NodeLevel);

	~NtreeNode();

	void Insert(const Data &data);

	Data* Search(Query &query);

	void SearchAll(Query &query);

	void SetProperties();

	void Analyze(Analyzer &stats);

	NtreeNode* SearchData(const Vertex & p);

	void equiSplit(const Box &b,Box splits[]);

	friend std::ostream & operator << (std::ostream & lhs,const NtreeNode & rhs);

	void PrintFullSummary();

	};

NtreeNode::NtreeNode(const Box &box, int NodeLevel)
	{
	extent = box;
	totalLeaves=0;
	leafCount=0;
	Level=NodeLevel;
	minDepth=0;
	isLeaf=true;

	for (int i=0;i<SPLITS;i++)
		children[i]=NULL;

	for (int i=0;i<MAXDATA;i++)
		leaves[i]=NULL;
	}

NtreeNode::~NtreeNode()
{
	if (isLeaf==true)
		{
		for (int j=0;j<MAXDATA;j++)
			if (leaves[j]!=NULL)
				{
				delete leaves[j];
				leaves[j]=NULL;
				}
		}
	else
		{
		for (int i=0;i<SPLITS;i++)
			if (children[i]!=NULL)
				{
				children[i]->~NtreeNode();
				children[i]=NULL;
				}
		}
}

void NtreeNode::Insert(const Data &data)
{
	if (isLeaf==true)
		{
		if (leafCount==MAXDATA)
			{
			Box spilts[SPLITS];
			equiSplit(this->extent,spilts);
			isLeaf = false;
			leafCount=0;
			totalLeaves++;
			for (int i=0;i<SPLITS;i++)
				children[i] = new NtreeNode(spilts[i],Level+1);

			for (int i=0;i<MAXDATA;i++)
				{
				NtreeNode* temp = SearchData(leaves[i]->midPoint);
				temp->Insert ( *leaves[i] );
				delete leaves[i];
				leaves[i]=NULL;
				}

			NtreeNode* temp = SearchData(data.midPoint);
			temp->Insert ( data );
			}
		else
			{
			leaves[leafCount] = new Data(data.midPoint,data.id);
			leafCount++;
			totalLeaves++;
			}
		}
	else
		{
		for (int i=0;i<SPLITS;i++)
			if (children[i]!=NULL)
				if (Box::enclose(children[i]->extent,data.midPoint))
					{
					totalLeaves++;
					children[i]->Insert(data);
					}
		}
}

Data* NtreeNode::Search(Query &query)
	{
	query.VisitNode();
	if (isLeaf==true)
		{
		for (int i=0;i<MAXDATA;i++)
			if (leaves[i]!=NULL)
				if (Box::enclose(query.box,leaves[i]->midPoint))
				{
					query.VisitLeaf(*leaves[i]);
					return leaves[i];
				}
		return NULL;
		}
	else
		{
		if (HEIRISTIC==true)
		{
		for (int i=0;i<SPLITS;i++)
			if (children[i]!=NULL)
				if (Box::enclose(query.box,children[i]->extent) && children[i]->totalLeaves>0)
					{
					//cout << "E";
					Data* temp = children[i]->Search(query);
					if (temp!=NULL) return temp;
					}
		}
		for (int i=0;i<SPLITS;i++)
			if (children[i]!=NULL)
				if (Box::overlap(query.box,children[i]->extent) && children[i]->totalLeaves>0)
					{
					//cout << "O";
					Data* temp = children[i]->Search(query);
					if (temp!=NULL) return temp;
					}
		return NULL;
		}
	}

void NtreeNode::SearchAll(Query &query)
	{
	query.VisitRangeNode();
	if (isLeaf==true)
		{
		for (int i=0;i<MAXDATA;i++)
			if (leaves[i]!=NULL)
				if (Box::enclose(query.box,leaves[i]->midPoint))
					query.VisitRangeLeaf(*leaves[i]);
		}
	else
		{
		for (int i=0;i<SPLITS;i++)
			if (children[i]!=NULL)
				if (Box::overlap(query.box,children[i]->extent))
					children[i]->SearchAll(query);
		}
	}

NtreeNode* NtreeNode::SearchData(const Vertex & p)
	{
	if (isLeaf==true)
		return this;
	else
	for (int i=0;i<SPLITS;i++)			// Search All Nodes .. this can be formulated in an equation
		if (children[i]!=NULL)
			if (Box::enclose(children[i]->extent,p))
				return children[i]->SearchData(p);
	cout << "Should not reach this point!";
	return NULL;
	}

void NtreeNode::PrintFullSummary()
	{
	cout << *this << "\n";
	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			children[i]->PrintFullSummary();
	}

std::ostream & operator << (std::ostream & lhs,const NtreeNode & rhs)
	{
	for (int i=0;i<rhs.Level-1;i++) lhs << "\t";
	if (rhs.isLeaf==true) lhs << "Leaf:";
	else lhs << "NonLeaf:";
	lhs << " Level=" << rhs.Level << " Data=" << rhs.totalLeaves << " Depth=" << rhs.minDepth;
	lhs <<"  " << rhs.extent;
	return lhs;
	}

void NtreeNode::equiSplit(const Box &b,Box splits[])
	{
	Vertex width;
	Vertex::differenceVector(b.low,b.high,width);

	for (int i=0;i<DIMENSION;i++)
		width[i] = width[i] / SPLIT_PER_DIMENSION;

	int c=0;
	for (spaceUnit i=0;i<SPLIT_PER_DIMENSION;i++)
		for (spaceUnit j=0;j<SPLIT_PER_DIMENSION;j++)
			for (spaceUnit k=0;k<SPLIT_PER_DIMENSION;k++)
				{
				Vertex begin = Vertex(k+0,j+0,i+0);
				Vertex end   = Vertex(k+1,j+1,i+1);
				splits[c++]  = Box( b.low+(begin*width) , b.low+(end*width) );
				}
	}

void NtreeNode::Analyze(Analyzer &stats)
	{
	// Visit
	if (Level>stats.maxLevel) {stats.maxLevel=Level;}
	if (totalLeaves>stats.totalData) stats.totalData = totalLeaves;

	stats.totalNodes++;

	for (int i=0;i<MAXDATA;i++)
		if (this->leaves[i]!=NULL) stats.totalData++;

	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			children[i]->Analyze(stats);
	}

bool compare(const NtreeNode* x, const NtreeNode* y)
	{
	return x->minDepth < y->minDepth;
	}

void NtreeNode::SetProperties()
	{
	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			children[i]->SetProperties();

	if (this->isLeaf)
		{
		int count=0;
		for (int i=0;i<MAXDATA;i++)
			if (this->leaves[i]!=NULL) count++;
		if (count>0) this->minDepth=1;
		this->totalLeaves=count;
		}
	else
		{
		int count=0;
		int min=-1;
		for (int i=0;i<SPLITS;i++)
			if (this->children[i]!=NULL && this->children[i]->totalLeaves>0)
				{
				if (min==-1) min =  this->children[i]->minDepth;
				if (min>this->children[i]->minDepth) min = this->children[i]->minDepth;
				count+=this->children[i]->totalLeaves;
				}
		this->minDepth=min+1;
		this->totalLeaves=count;
		sort(this->children,this->children+SPLITS,compare);
		}
	}

/*
 * NTree Algorithm OnDemand Split
 */
class Ntree
	{
	public:
	Box worldCoordinate;
	NtreeNode* root;

	Ntree(const Box &worldExtent)
		{
		this->worldCoordinate = worldExtent;
		this->root = new NtreeNode(worldCoordinate,1);
		}

	~Ntree()
	{
		root->~NtreeNode();
		delete root;
	}

	void Insert(const Data &data)
		{
		root->Insert (data);
		}

	void Optimize()
		{
		root->SetProperties();
		}

	void Analyze()
	{
		Analyzer* stats = new Analyzer();
		root->Analyze(*stats);
		cout << *stats;
	}

	void PrintSummary()
		{
		root->PrintFullSummary();
		}

	void Seed(Query &query)
		{
		//query->executionTime.start();
		root->Search(query);
		//query->executionTime.stop();
		//root->SearchAll(query);
		}

	void RangeSearch(Query &query)
		{
		//query->executionTime.start();
		root->SearchAll(query);
		//query->executionTime.stop();
		}
	};

#endif
