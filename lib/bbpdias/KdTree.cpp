#ifndef KD_TREE_
#define KD_TREE_

#include <iostream>
#include <algorithm>
#include <vector>
#include <float.h>
#include <math.h>
#include "Vertex.cpp"
#include "Box.cpp"
#include "Query.cpp"
#include "Data.cpp"
#include "Analyzer.cpp"

#define SPLITS 64 // Minimum 2
#define LONGEST_DIMENSION_SPLIT true

using namespace std;
/*
 * Kdtree Node
 * Based on Median Split
 * with N splits on demand
 * each Leaf is a bucket of Data with MAX DATA size
 *
 * Author: Farhan Tauheed
 */

class KdtreeNode
	{
	public:
	int Level; // 1 is root
	int totalLeaves;
	bigSpaceUnit overlapRatio;
	int splitDimension;
	Box extent;

	KdtreeNode* children[SPLITS];
	Data* leaves[SPLITS-1]; //Data pointers in all Nodes not good

	KdtreeNode(int NodeLevel);

	~KdtreeNode();

	void KdtreeNode::Insert(vector<Data>* bulkData ,int start,int end,int dimension);

	Data* Search(Query &query);

	void SearchAll(Query &query);

	void Analyze(Analyzer &stats);

	friend std::ostream & operator << (std::ostream & lhs,const KdtreeNode & rhs);

	void PrintFullSummary();

	void SetProperties();
	};

KdtreeNode::KdtreeNode(int NodeLevel)
	{
	Level=NodeLevel;
	splitDimension =0;
	overlapRatio =0;
	totalLeaves=0;
	for (int i=0;i<DIMENSION;i++)
		{
		extent.low[i] = FLT_MAX;
		extent.high[i] = FLT_MIN;
		}
	for (int i=0;i<SPLITS;i++)
		children[i]=NULL;

	for (int i=0;i<SPLITS-1;i++)
		leaves[i]=NULL;
	}

KdtreeNode::~KdtreeNode()
	{
	for (int j=0;j<SPLITS-1;j++)
		if (leaves[j]!=NULL)
			{
			delete leaves[j];
			leaves[j]=NULL;
			}

	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			{
			children[i]->~KdtreeNode();
			delete children[i];
			children[i]=NULL;
			}
	}

bool xCompare(const Data &a, const Data &b)
	{
	return a.midPoint[0] < b.midPoint[0];
	}

bool yCompare(const Data &a, const Data &b)
	{
	return a.midPoint[1] < b.midPoint[1];
	}

bool zCompare(const Data &a, const Data &b)
	{
	return a.midPoint[2] < b.midPoint[2];
	}

bool compare(const KdtreeNode* x, const KdtreeNode* y)
	{
	if (x==NULL) return false;
	if (y==NULL) return true;
	return  x->overlapRatio < y->overlapRatio;
	}

void KdtreeNode::Insert(vector<Data>* bulkData,int start,int end,int dimension)
	{
	extent.high = bulkData->at(start).midPoint;
	extent.low  = bulkData->at(start).midPoint;

	for (std::vector<Data>::iterator i = bulkData->begin()+start; i != bulkData->begin()+end; ++i)
		for (int j=0;j<DIMENSION;j++)
			{
			if (extent.high[j]<i->midPoint[j]) extent.high[j]=i->midPoint[j];
			if (extent.low[j] >i->midPoint[j]) extent.low[j] =i->midPoint[j];
			}

	int max = 0;
	for (int i=0;i<DIMENSION;i++)
		if ((extent.high[i]-extent.low[i])>(extent.high[max]-extent.low[max])) max = i;
	splitDimension = max;       // Longest Dimension Split
	if (LONGEST_DIMENSION_SPLIT==false) splitDimension = dimension; // Cycling Split

	if (splitDimension==0)
		sort (bulkData->begin()+start,bulkData->begin()+end,xCompare); // TO optimize random access
	if (splitDimension==1)
		sort (bulkData->begin()+start,bulkData->begin()+end,yCompare); // TO optimize random access
	if (splitDimension==2)
		sort (bulkData->begin()+start,bulkData->begin()+end,zCompare); // TO optimize random access

	int indexEnd=0,indexStart=0;
	float splitWidth = (end - start+0.0) / (SPLITS+0.0);
	//cout << "Start: " << start << " End: " << end << " Dimension: "<< dimension << " Level: " <<this->Level << " splitWidth: "<< splitWidth <<"  " ;
	//for (int i=start;i<end;i++) cout << bulkData->at(i).id << "-"; cout << "\n";

	if (end-start<=SPLITS-1)
		{
		for (int i=0;i<end-start;i++)
			this->leaves[i] = new Data( bulkData->at(start+i).midPoint, bulkData->at(start+i).id);
		}
	else
		{
		indexStart = start;
		indexEnd = start-1;
		for (int i=0;i<SPLITS;i++)
			if (i==SPLITS-1)
				{
				indexStart = indexEnd+1;
				indexEnd = end;
				if (indexEnd-indexStart > 0)
					{
					this->children[i] = new KdtreeNode(Level+1);
					this->children[i]->Insert(bulkData,indexStart,indexEnd,(splitDimension+1)%3);
					}
				}
			else
				{
				indexStart = indexEnd+1;
				indexEnd   = start+(int)((i+1)*splitWidth);
				if (indexEnd-indexStart > 0)
					{
					this->children[i] = new KdtreeNode(Level+1);
					this->children[i]->Insert(bulkData,indexStart,indexEnd,(splitDimension+1)%3);
					}
				if (indexStart<=indexEnd)
					{
					this->leaves[i] = new Data( bulkData->at(indexEnd).midPoint, bulkData->at(indexEnd).id);
					}
				}
		}
	}

Data* KdtreeNode::Search(Query &query)
	{
	query.VisitNode();
	//cout  << "Visting: " << *this << " Dim:" << splitDimension << "\n";

	for (int i=0;i<SPLITS-1;i++)
		if (this->leaves[i]!=NULL)
			if (Box::enclose(query.box,this->leaves[i]->midPoint))
			{
			//cout << "Leaf found: " << *leaves[i] << "\n";
			query.VisitLeaf(*leaves[i]);
			return leaves[i];
			}

/*
	for (int i=0;i<SPLITS;i++)
		if (this->children[i]!=NULL)
		{
			bigSpaceUnit overlapVolume = Box::overlapVolume(query.box,this->children[i]->extent);
			if (overlapVolume<=0) this->children[i]->overlapRatio=0;
			else this->children[i]->overlapRatio =  Box::volume(this->children[i]->extent);
		}

	sort(this->children,this->children+SPLITS,compare);
*/
	for (int i=0;i<SPLITS;i++)
		if (this->children[i]!=NULL)
			{
			//cout << "Checking Overlap: " << *this->children[i] << "\n";
			if (Box::overlap(query.box,this->children[i]->extent))
				{
				//cout << "Found Overlap: " << *this->children[i] << "\n";
				Data* temp = this->children[i]->Search(query);
				if (temp!=NULL) return temp;
				}
			}
	//cout << "Returning NULL\n";
	return NULL;
	}

void KdtreeNode::SearchAll(Query &query)
	{
	query.VisitRangeNode();
	for (int i=0;i<SPLITS-1;i++)
		if (leaves[i]!=NULL)
			if (Box::enclose(query.box,leaves[i]->midPoint))
				query.VisitRangeLeaf(*leaves[i]);

	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			if (Box::overlap(query.box,children[i]->extent))
				children[i]->SearchAll(query);
	}

void KdtreeNode::PrintFullSummary()
	{
	cout << *this << "\n";
	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			children[i]->PrintFullSummary();
	}

std::ostream & operator << (std::ostream & lhs,const KdtreeNode & rhs)
	{
	for (int i=0;i<rhs.Level-1;i++) lhs << "\t";
	//if (rhs.isLeaf==true) lhs << "Leaf:";
	//else

	lhs << "Node:";
	lhs << " Level=" << rhs.Level << " Extent= " << rhs.extent;

	//for (int i=0;i<SPLITS-1;i++)
	//	if (rhs.leaves[i]!=NULL)
	//		lhs << *(rhs.leaves[i]) << "  ";
	return lhs;
	}

void KdtreeNode::Analyze(Analyzer &stats)
	{
	// Visit

	stats.totalNodes++;
	for (int i=0;i<SPLITS-1;i++)
		if (this->leaves[i]!=NULL) stats.totalData++;

	int c=0;
	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			children[i]->Analyze(stats);
		else c++;

	if (Level>stats.maxLevel) stats.maxLevel=Level;
	if (c==SPLITS && Level<stats.minLevel) stats.minLevel=Level;
	}

void KdtreeNode::SetProperties()
	{
	for (int i=0;i<SPLITS;i++)
		if (children[i]!=NULL)
			children[i]->SetProperties();

	for (int i=0;i<SPLITS;i++)
		if (this->children[i]!=NULL)
			totalLeaves++;

	if (totalLeaves>0)
		sort(this->children,this->children+totalLeaves,compare);

	for (int i=0;i<SPLITS-1;i++)
		if (this->leaves[i]!=NULL)
			totalLeaves++;
	}
/*
 * NTree Algorithm OnDemand Split
 */
class Kdtree
	{
	public:
	KdtreeNode* root;
	vector<Data>* bulkData;
	int countData;

	Kdtree()
		{
		this->root = new KdtreeNode(1);
		bulkData = new vector<Data>();
		bulkData->reserve(4600000);
		countData=0;
		}

	~Kdtree()
		{
		if (bulkData!=NULL)
			delete bulkData;
		root->~KdtreeNode();
		}

	void Insert(const Data &data)
		{
		bulkData->push_back(data);
		countData++;
		}

	void Build()
		{
		root->Insert (bulkData,0,countData,0);
		cout << "build Complete";
		if (bulkData!=NULL)
			delete bulkData;
		bulkData = NULL;
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
		}

	void Optimize()
		{
		root->SetProperties();
		}

	void RangeSearch(Query &query)
		{
		//query->executionTime.start();
		root->SearchAll(query);
		//query->executionTime.stop();
		}
	};

#endif
