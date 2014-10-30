#include "DataFileReader.hpp"
#include "SpatialQuery.hpp"
//#include "HilbertRtreeIndex.hpp"
#include "Segment.hpp"
#include "hilbert.hpp"
#include <set>
#include <list>
#include <stack>
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>


#define algo_NL				0	//Nested Loop
#define algo_PS				1	//Plane-Sweeping
#define	algo_SGrid			2	//Spatial Grid Hash Join
#define	algo_S3				3	//Size Separation Spatial
#define	algo_TOUCH			4	//TOUCH:Spatial Hierarchical Hash Join
#define	algo_PBSM			5	//Partition Based Spatial-Merge Join
#define	algo_cTOUCH			6	//Partition Based Spatial-Merge Join
#define	algo_dTOUCH			7	//Partition Based Spatial-Merge Join

#define No_Sort					0
#define Hilbert_Sort			1
#define X_axis_Sort				2
#define STR_Sort				3

int base = 2; // the base for S3 and SH algorithms

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
int PartitioningType                     = Hilbert_Sort;	// Sorting algorithm
int runs				=  1;				// # of runs
double epsilon				=  1.5;				// the epsilon of the similarity join
int partitions				=  4;				// # of partitions: in S3 is # of levels; in SGrid is resolution

const int types				=  2;
const int max_assignment_level          =  3;			//for dTOUCH - maximum assignment level

string infile_dsA = "..//data//RandomData-100K.bin";
string infile_dsB = "..//data//RandomData-1600K.bin";

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

//The tree data structure for the hierarchical approaches.
//a TreeNode contains many TreeEntry
//The TreeEntry of a leaf level stores the object while the others store the MBR of their universe together with their pointer to their children TreeNode



vector<TreeEntry*> vdsAll;	//vector of the mixed Objects and their MBRs
vector<TreeEntry*> vdsA;	//vector of the Objects and their MBRs of the smaller dataset ??@todo smallest?
vector<TreeEntry*> vdsB;       

uint64 size_dsA,size_dsB;




// Comparator of the sort functions for the partitioning

// for testing
inline double distance(SpatialObject* sobj1, SpatialObject* sobj2)
{
	vector<Vertex> vertices;
	Box mbr = sobj2->getMBR();
	Box::getAllVertices(sobj1->getMBR(),vertices);
	double dist = 10000000;
	for (unsigned int i=0;i<vertices.size();++i)
		dist = min( mbr.pointDistance(vertices.at(i)) , dist);

	return dist;
}
// for testing
inline double objSize(SpatialObject* sobj)
{
	vector<Vertex> vertices;
	Box mbr = sobj->getMBR();
	return (mbr.high[0]-mbr.low[0])*(mbr.high[1]-mbr.low[1])*(mbr.high[2]-mbr.low[2]);
}

// for testestDSting
void testDS(SpatialObjectList& A, SpatialObjectList& B)
{
        double aveD,stdD;
	aveD = 0;
	stdD = 0;
	double sum=0,sqsum = 0,dist;
	uint64 count=0;
	ofstream lg; lg.open("DS.txt",ofstream::app);
	if(PartitioningType == 10)
	{
		//compute the average distance
	for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
		for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
		{
			count++;
			dist = distance(*itA , *itB);
			sum += dist;
			sqsum += dist*dist;
			if( count % 1000000 == 0 )
			{
				aveD = sum / (double)count;
				stdD = sqrt((sqsum/(double)count)-aveD*aveD);
				cout << count << ": ave=" << aveD << " std=" << stdD << endl;
				lg << count << ":" << aveD << "," << stdD << ";";
				if(count >= 100000000)
				{
					aveD = sum / (double)count;
					stdD = sqrt((sqsum/(double)count)-aveD*aveD);
					lg << "\n ***distance*** \n" << file_dsA << '\n' << file_dsB << '\n' << aveD << "," << stdD << ";" << endl;

					lg.close();
					return;
				}
			}

		}
	aveD = sum / (double)count;
	stdD = sqrt((sqsum/(double)count)-aveD*aveD);
	lg << "\n *** \n" << file_dsA << '\n' << file_dsB << '\n' << aveD << "," << stdD << ";" << endl;
	}
	else
	{
		//compute the average of size
		for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
		{
			count++;
			dist = objSize(*itA);
			sum += dist;
			sqsum += dist*dist;
			if( count % 1000 == 0 )
			{
				aveD = sum / (double)count;
				stdD = sqrt((sqsum/(double)count)-aveD*aveD);
				cout << count << ": ave=" << aveD << " std=" << stdD << endl;
				lg << count << ":" << aveD << "," << stdD << ";";
				if(count >= 10000)
				{
					aveD = sum / (double)count;
					stdD = sqrt((sqsum/(double)count)-aveD*aveD);
					lg << "\n ***SIZE*** \n" << file_dsA << '\n' << aveD << "," << stdD << ";" << endl;

					lg.close();
					return;
				}
			}
		}
		aveD = sum / (double)count;
		stdD = sqrt((sqsum/(double)count)-aveD*aveD);
		lg << "\n ***SIZE*** \n" << file_dsA << '\n' << aveD << "," << stdD << ";" << endl;
	}
	lg.close();
}


//Nested Loop join algorithm
void NL(SpatialObjectList& A, SpatialObjectList& B,  ResultPairs& results)
{
	for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
		for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
			if ( istouching(*itA , *itB) )
				results.addPair( *itA , *itB );
}

//Plane-sweeping join algorithm
void PS(SpatialObjectList& A, SpatialObjectList& B,  ResultPairs& results)
{

	//Sort the datasets based on their lower x coordinate
	stats.sorting.start();
	std::sort(A.begin(), A.end(), Comparator_Xaxis());
	//cout<<"Sort A Done."<<endl;
	std::sort(B.begin(), B.end(), Comparator_Xaxis());
	//cout<<"Sort B Done."<<endl;
	stats.sorting.stop();

	//sweep
	uint64 iA=0,iB=0;
	while(iA<A.size() && iB<B.size())
	{
		//cout<<iA<< " " <<iB<<endl;
		if(A.at(iA)->getMBR().low[0] < B.at(iB)->getMBR().low[0])
		{
			uint64 i = iB;
			spaceUnit border = A.at(iA)->getMBR().high[0]+epsilon;
			while(i<B.size() && B.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(i) , A.at(iA)) )
				{
					if (B.at(i)->type == 1)
						results.addPair( B.at(i),A.at(iA) );
					else
						results.addPair( A.at(iA), B.at(i));
				}
				i++;
			}
			iA++;
		}
		else
		{
			uint64 i = iA;
			spaceUnit border = B.at(iB)->getMBR().high[0]+epsilon;
			while(i<A.size() &&  A.at(i)->getMBR().low[0] < border)
				{
				if ( istouching(B.at(iB) , A.at(i)) )
				{
					if (B.at(iB)->type == 1)
						results.addPair( B.at(iB),A.at(i) );
					else
						results.addPair( A.at(i), B.at(iB));
				}
				i++;
			}
			iB++;
		}
	}
}

//Nested Loop join algorithm
void NL(TreeNode* node, SpatialObjectList& B,  ResultPairs& results)
{
	for(vector<TreeEntry*>::iterator itA = node->entries.begin(); itA != node->entries.end(); ++itA)
		for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
			if ( istouching((*itA)->obj , *itB) )
				results.addPair( (*itA)->obj , *itB );
}

//Plane-sweeping join algorithm
void PS(TreeNode* node, SpatialObjectList& B,  ResultPairs& results)
{
	//Sort the datasets based on their lower x coordinate
	stats.sorting.start();
	std::sort(node->entries.begin(), node->entries.end(), ComparatorTree_Xaxis());
	//cout<<"Sort A Done."<<endl;
	std::sort(B.begin(), B.end(), Comparator_Xaxis());
	//cout<<"Sort B Done."<<endl;
	stats.sorting.stop();

	//sweep
	uint64 iA=0,iB=0;
	while(iA<node->entries.size() && iB<B.size())
	{
		if(node->entries.at(iA)->mbr.low[0] < B.at(iB)->getMBR().low[0])
		{
			uint64 i = iB;
			spaceUnit border = node->entries.at(iA)->mbr.high[0];//+epsilon;
			while(i<B.size() && B.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(i) , node->entries.at(iA)->obj) )
					results.addPair( B.at(i),node->entries.at(iA)->obj );
				i++;
			}
			iA++;
		}
		else
		{
			uint64 i = iA;
			spaceUnit border = B.at(iB)->getMBR().high[0];//+epsilon;
			while(i<node->entries.size() &&  node->entries.at(i)->mbr.low[0] < border)
			{
				if ( istouching(B.at(iB) , node->entries.at(i)->obj) )
					results.addPair( B.at(iB),node->entries.at(i)->obj );
				i++;
			}
			iB++;
		}
	}
}


//Plane-sweeping join algorithm for cTOUCH <--------------
void PS(SpatialObject* A, SpatialObjectList& B,  ResultPairs& results)
{
	//@todo sorting???
    for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
		if ( istouching(A , (*itB)) )
			results.addPair( A,(*itB) );
}


//Nested Loop join algorithm cTOUCH
void NL(SpatialObject* A, SpatialObjectList& B,  ResultPairs& results)
{
	for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
		if ( istouching(A , *itB) )
			results.addPair( A , *itB );
}

//Nested Loop join algorithm
void NL(SpatialObjectList& A, SpatialObjectList& B,  ResultList& results)
{
	for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
		for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
			if ( istouching(*itA , *itB) )
			{
				stats.deDuplicate.start();
				uint64 temp = results.size();
				if(PartitioningType == 4)// To have some clue!
					stats.results++;
				else{
				results.insert(ResultPair(*itA , *itB) );
				if(temp == results.size())
					stats.duplicates++;
				else
					stats.results++;
				}
				stats.deDuplicate.stop();
			}
}

//Plane-sweeping join algorithm
void PS(SpatialObjectList& A, SpatialObjectList& B,  ResultList& results)
{

	//Sort the datasets based on their lower x coordinate
	stats.sorting.start();
	std::sort(A.begin(), A.end(), Comparator_Xaxis());
	//cout<<"Sort A Done."<<endl;
	std::sort(B.begin(), B.end(), Comparator_Xaxis());
	//cout<<"Sort B Done."<<endl;
	stats.sorting.stop();

	//sweep
	uint64 iA=0,iB=0;
	while(iA<A.size() && iB<B.size())
	{
		//cout<<iA<< " " <<iB<<endl;
		if(A.at(iA)->getMBR().low[0] < B.at(iB)->getMBR().low[0])
		{
			uint64 i = iB;
			spaceUnit border = A.at(iA)->getMBR().high[0]+epsilon;
			while(i<B.size() && B.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(i) , A.at(iA)) )
				{
					stats.deDuplicate.start();
					uint64 temp = results.size();
					if(PartitioningType == 4)
						stats.results++;
					else{
							results.insert(ResultPair( B.at(i),A.at(iA)) );
							
						if(temp == results.size())
							stats.duplicates++;
						else
							stats.results++;
					}
					stats.deDuplicate.stop();
				}

				i++;
			}
			iA++;
		}
		else
		{
			uint64 i = iA;
			spaceUnit border = B.at(iB)->getMBR().high[0]+epsilon;
			while(i<A.size() &&  A.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(iB) , A.at(i)) )
				{
					stats.deDuplicate.start();
					uint64 temp = results.size();
					if(PartitioningType == 4)
						stats.results++;
					else{
							results.insert(ResultPair( B.at(i),A.at(iA)) );
					if(temp == results.size())
						stats.duplicates++;
					else
						stats.results++;
					}
					stats.deDuplicate.stop();
				}

				i++;
			}
			iB++;
		}
	}
}

//The local join for joining two buckets
void JOIN(SpatialObjectList& A, SpatialObjectList& B,  ResultPairs& results) //type is for result array ordering
{
	Ljoin.start();
	//cout<<"join " << A.size() << " and " <<B.size()<<endl;
	if(localJoin == algo_NL)
		NL(A,B,results);
	else
		PS(A,B,results);
	Ljoin.stop();
}

void JOIN(SpatialObjectList& A, SpatialObjectList& B,  ResultList& results)
{
	Ljoin.start();
	//cout<<"join " << A.size() << " and " <<B.size()<<endl;
	if(localJoin == algo_NL)
		NL(A,B,results);
	else
		PS(A,B,results);
	Ljoin.stop();
}

void JOIN(TreeNode* node, SpatialObjectList& B,  ResultPairs& results)
{
	Ljoin.start();
	//cout<<"join " << A.size() << " and " <<B.size()<<endl;
	if(localJoin == algo_NL)
		NL(node,B,results);
	else
		PS(node,B,results);
	Ljoin.stop();
}


//JOIN for cTOUCH <--------
void JOIN(SpatialObject* obj, SpatialObjectList& B,  ResultPairs& results)
{
	Ljoin.start();
	if(localJoin == algo_NL)
		NL(obj,B,results);
	else
		PS(obj,B,results);
	Ljoin.stop();
}

void usage(const char *program_name) {

    printf("   Usage: %s [-v] [-h] [-a <int>] [-i <path> <path>]\n", program_name);
    printf("   -h               Print this help menu.\n");
    printf("   -a               Algorithms\n");
    printf("      0:Nested Loop\n");
    printf("      1:Plane-Sweeping\n");
    printf("      2:Spatial Grid Hash\n");
    printf("      3:Size Separation Spatial\n");
    printf("      4:TOUCH:Spatial Hierarchical Hash\n");
    printf("      5:Partition Based Spatial-Merge Join\n");
    printf("      6:cTOUCH:Spatial Hierarchical Has\n");
    printf("      7:dTOUCH:Spatial Hierarchical Has\n");
    printf("\n");
    printf("   -J               Algorithm for joining the buckets\n");
    printf("   -p               # of partitions\n");
    printf("   -b               the number of cells to be merged in the hierarchy\n");
    printf("   -e               Epsilon of the similarity join\n");
    printf("   -r               # of runs\n");
    printf("   -i <path> <path>  Dataset A followed by B\n");
    printf("   -n #A #B  number of element to be read\n");
    printf("   -v verbose\n");
    printf("\n-a 1 -i 'c:\\data\\A.dat' 'c:\\data\\B.dat'\n");

}

//Parsing Arguments
void parse_args(int argc, const char* argv[]) {

    int x;
    if(argc<2)
    {
    	usage(argv[0]);
        exit(1);
    }
    for ( x= 1; x < argc; ++x)
    {
        switch (argv[x][1])
        {
        case 'h':
            usage(argv[0]);
            exit(1);
            break;
		case 'i':
            if (++x < argc)
            {
                file_dsA=argv[x];
                file_dsB=argv[++x];
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument, %s", argv[x-1]);
                usage(argv[0]);
            }
            break;
		case 'a':
			sscanf(argv[++x], "%u", &algorithm);
            break;
		case 'n':
			sscanf(argv[++x], "%u", &numA);
			sscanf(argv[++x], "%u", &numB);
            break;
		case 'J':
			sscanf(argv[++x], "%u", &localJoin);
			break;
		case 'r':       /* # of runs */
			sscanf(argv[++x], "%u", &runs);
            break;
		case 't':       /* Testing */
			sscanf(argv[++x], "%u", &PartitioningType);
            break;
		case 'e':       /* epsilon */
			sscanf(argv[++x], "%lf", &epsilon);
            break;
		case 'p':       /* # of partitions */
			sscanf(argv[++x], "%u", &partitions);
            break;
		case 'b':       /* base for the number of components to merge in every level of the hierarchy */
			sscanf(argv[++x], "%u", &base);
            break;
		case 'v':       /* verbose */
			verbose = true;
            break;
        default:
            fprintf(stderr, "Error: Invalid command line parameter, %c\n", argv[x][1]);
            usage(argv[0]);
        }
    }
}



// The following functions are for creating the corresponding object of their specified join method

//Spatial Grid Hash Join algorithm
void SGrid()
{
	Box universe;
	//Box::combine(inputA->universe,inputB->universe,universe);
	if(numA > 0 && numB > 0)
	{
		universe = universeA;
	}
	else
	{
		universe = inputA->universe;
	}

	Box::expand(universe,epsilon);
	cout<< "Universe: " << universe.low << " " << universe.high <<endl;
	SpatialGridHash* spatialGridHash = new SpatialGridHash(universe,partitions);

	cout << "Buidling ";
	spatialGridHash->build(dsA);
	cout << "Done\nAnalysis " ;
	spatialGridHash->analyze(dsA,dsB);
	cout << "Done\nProbing ";
	ResultList SGresults;
	spatialGridHash->probe(dsB , SGresults);
	cout << "\nDone." << endl;
	//stats.results = SGresults.size();
	delete spatialGridHash;
}

//Size Separation Spatial join algorithm
void S3(ResultPairs& S3Results)
{
	Box universe;
	if(numA > 0 && numB > 0)
	{
		Box::combine(universeA,universeB,universe);
	}
	else
	{
		Box::combine(inputA->universe,inputB->universe,universe);
	}

	Box::expand(universe,epsilon);
	cout<< "Universe: " << universe.low << " " << universe.high <<endl;
	Box::combine(inputA->universe,inputB->universe,universe);
	S3Hash* s3Hash = new S3Hash(universe,partitions);

	cout << "Building Started" << endl;
	s3Hash->build(dsA, dsB);
	cout << "Building Done" << endl;
	s3Hash->analyze(dsA,dsB);
	cout << "Analysis Done" << endl;
	s3Hash->probe(S3Results);

	cout << "Probing Done" << endl;
	delete s3Hash;
}

//Partition Based Spatial-Merge join algorithm
void PBSM()
{
	ResultList PBSMResults;
	Box universe;
	if(numA > 0 && numB > 0)
	{
		universe = universeA;
	}
	else
	{
		universe = inputA->universe;
	}

	Box::expand(universe,epsilon);
	cout<< "Universe: " << universe.low << " " << universe.high <<endl;
	PBSMHash* pbsmHash = new PBSMHash(universe,partitions);

	cout << "Building Started" << endl;
	pbsmHash->build(dsA, dsB);
	cout << "Building Done" << endl;
	pbsmHash->analyze(dsA,dsB);
	cout << "Analysis Done" << endl;
	pbsmHash->probe(PBSMResults);
	cout << "Probing Done" << endl;
	delete pbsmHash;
}

void cTOUCH(ResultPairs& cTOUCHResults)
{

	TOUCHJoin* touch = new TOUCHJoin(partitions);
	cout << "Forming the partitions" << endl;
	touch->createPartitions();
	cout << "Assigning the objects of B" << endl;
	touch->assignment();
	cout << "Assigning Done." << endl;
	touch->analyze(dsA,dsB);
	cout << "Analysis Done" << endl;
	cout << "Probing, doing the join" << endl;
	touch->probe();
	cout << "Done." << endl;
}

void TOUCH(ResultPairs& TOUCHResults)
{

	TOUCHJoin* touch = new TOUCHJoin(partitions);
	cout << "Forming the partitions" << endl;
	touch->createPartitions();
	cout << "Assigning the objects of B" << endl;
	touch->assignment(dsB);
	cout << "Assigning Done." << endl;
	touch->analyze(dsA,dsB);
	cout << "Analysis Done" << endl;
	cout << "Probing, doing the join" << endl;
	touch->probe();
	cout << "Done." << endl;
}

int main(int argc, const char* argv[])
{
	//Parsing the arguments
	parse_args(argc, argv);
	//Reading the datasets and allocating memory
	readBinaryInput();

	if(PartitioningType >= 10)
	{
		testDS(dsA,dsB);
		return 1;
	}

	ResultPairs results;
	stats.total.start();

	switch(algorithm)
	{
		case algo_NL:
			NL(dsA,dsB,results);
		break;
		case algo_PS:
			cout << "Unsupported in this version" << endl;
			//PS(dsA,dsB,results);
		break;
		case algo_TOUCH:
			TOUCH(results);
		break;
		case algo_cTOUCH:
			cTOUCH(results);
		break;
		case algo_dTOUCH:
			dTOUCH(results);
		break;
		case algo_SGrid:
			SGrid();
		break;
		case algo_S3:
			S3(results);
		break;
		case algo_PBSM:
			PBSM();
		break;
		default:
			cout<<"No such an algorithm!"<<endl;
			exit(0);
		break;
	}

	stats.total.stop();
	//Reporting
	stats.print();

	cout<<"Terminated."<<endl;
	return 1;
}
