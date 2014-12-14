/* 
 * File:   JoinAlgorithm.h
 * 
 */

#ifndef JOINALGORITHM_H
#define	JOINALGORITHM_H

#include <string>
#include <limits>
#include "DataFileReader.hpp"
#include "ResultPairs.h"
#include "TreeNode.h"
#include "hilbert.hpp"
#include "Box.hpp"
#include "DataFileReader.hpp"

#define algo_NL				0	//Nested Loop
#define algo_PS				1	//Plane-Sweeping
#define	algo_SGrid			2	//Spatial Grid Hash Join
#define	algo_S3				3	//Size Separation Spatial
#define	algo_TOUCH			4	//TOUCH:Spatial Hierarchical Hash Join
#define	algo_PBSM			5	//Partition Based Spatial-Merge Join
#define	algo_cTOUCH			6	//cTOUCH: Complex Spatial Hierarchical Hash Join
#define	algo_dTOUCH			7	//dTOUCH: Double Spatial Hierarchical Hash Join
#define	algo_reTOUCH			8	//reTOUCH: Redoing Spatial Hierarchical Hash Join
#define	algo_rereTOUCH			9	//rereTOUCH: Double Redoing Spatial Hierarchical Hash Join

#define No_Sort				0
#define Hilbert_Sort                    1
#define X_axis_Sort			2
#define STR_Sort			3

typedef SpatialObjectList HashValue;
typedef pair<FLAT::uint64,HashValue*> ValuePair;
typedef boost::unordered_map <FLAT::uint64,HashValue*> HashTable;

class JoinAlgorithm {

public:
    FLAT::uint64 totalGridCells;
    std::vector<TreeNode*> tree;		// Append only structure can be replaced by a file "payload"
    std::vector<TreeEntry*> nextInput;
    TreeEntry* root;
    FLAT::Timer Ljoin;// The time for local join
    SpatialObjectList dsA, dsB;					//A is smaller than B
    int localPartitions;
    bool profilingEnable;
    
    //for dTOUCH
    double maxLevelCoef;
    
    int PartitioningType;	// Sorting algorithm

    
    int base; // the base for S3 and SH algorithms
    string logfilename;
    double epsilon;
    unsigned int numA, numB;		//number of elements to be read from datasets
    
    
    FLAT::Box universeA, universeB;
    FLAT::Timer dataLoad;	//The time for copying the data to memory

    
    ResultPairs resultPairs;
    bool verbose;		// Output everything or not?
    

    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    
    string file_dsA;
    string file_dsB;
    
    int algorithm;		// Choose the algorithm
    int localJoin;		// Choose the algorithm for joining the buckets, The local join
    int partitions;				// # of partitions: in S3 is # of levels; in SGrid is resolution
    
    string algoname;
    string basealgo;
      
    JoinAlgorithm();
    virtual ~JoinAlgorithm();
    
    virtual void writeNode(std::vector<TreeEntry*> objlist,int Level) {};
    virtual void createTreeLevel(std::vector<TreeEntry*>& input,int Level) {};
    virtual void probe() {};

    void readBinaryInput(string file_dsA, string file_dsB);

    std::vector<TreeEntry*> vdsAll;	//vector of the mixed Objects and their MBRs
    std::vector<TreeEntry*> vdsA;	//vector of the Objects and their MBRs of the smaller dataset ??@todo smallest?
    std::vector<TreeEntry*> vdsB;       

    FLAT::uint64 size_dsA,size_dsB;

    void print();
    
    
    
    
    void NL(FLAT::SpatialObject*& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
            if ( istouching(A , *itB) )
                resultPairs.addPair( A , *itB );
    }
    
    void NL(SpatialObjectList& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
            NL((*itA), B);
    }
    
//protected:
        FLAT::uint64 ItemsCompared;
        FLAT::uint64 ItemsMaxCompared;
	//FLAT::uint64 gridSize;

	FLAT::uint64 filtered[TYPES];
	FLAT::uint64 hashprobe;
	double repA;
	double repB;

	FLAT::uint64 footprint;
	FLAT::uint64 maxMappedObjects;
	double percentageEmpty;
	double avg;
	double std;
	FLAT::Timer sorting;

	FLAT::Timer building;
	FLAT::Timer probing;
	FLAT::Timer analyzing;
	FLAT::Timer initialize;
	FLAT::Timer total;
	FLAT::Timer comparing;
	FLAT::Timer partition;
    FLAT::Timer gridCalculate;
    
    struct Comparator : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    FLAT::Vertex r1p,r2p;
                    r1p = r1->mbr.getCenter();
                    r2p = r2->mbr.getCenter();
                    if(r1p[0]<r2p[0])return true;
                    if(r1p[0]==r2p[0]&&r1p[1]<r2p[1])return true;
                    if(r1p[0]==r2p[0]&&r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                    return false;
            }
    };
    
    struct ComparatorHilbert : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    FLAT::Vertex r1p,r2p;
                    r1p = r1->mbr.getCenter();
                    r2p = r2->mbr.getCenter();
                    double d1[3],d2[3];
                    d1[0]=r1p[0];d1[1]=r1p[1];d1[2]=r1p[2];
                    d2[0]=r2p[0];d2[1]=r2p[1];d2[2]=r2p[2];
                    if (hilbert_ieee_cmp(3,d1,d2)>=0)  return true;
                    else return false;
            }
    };

    //	Comparing based on the Y axis iff two items are in the same parition
    struct ComparatorY : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    FLAT::Vertex r1p,r2p;
                    r1p = r1->mbr.getCenter();
                    r2p = r2->mbr.getCenter();

                    if(r1p[1]<r2p[1])return true;
                    if(r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                    return false;

            }
    };
    //	Comparing based on the Z axis iff two items are in the same parition
    struct ComparatorZ : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {
                    FLAT::Vertex r1p,r2p;
                    r1p = r1->mbr.getCenter();
                    r2p = r2->mbr.getCenter();

                    if(r1p[2]<r2p[2])return true;
                    return false;

            }
    };

    std::set<int> s;
    // Returns true if touch and false if not by comparing The corners of the MBRs
    inline bool istouchingV(FLAT::SpatialObject* sobj1, FLAT::SpatialObject* sobj2)
    {
            vector<FLAT::Vertex> vertices1;
            vector<FLAT::Vertex> vertices2;
            FLAT::Box mbr1 = sobj1->getMBR();
            FLAT::Box mbr2 = sobj2->getMBR();
            FLAT::Box::getAllVertices(sobj1->getMBR(),vertices1);
            FLAT::Box::getAllVertices(sobj2->getMBR(),vertices2);
            ItemsCompared++;
            
            for (unsigned int i=0;i<vertices1.size();++i)
            {
                if (FLAT::Box::enclose(mbr2,vertices1.at(i)))
                {
                    return true;
                }
            }
            
            for (unsigned int i=0;i<vertices2.size();++i)
            {
                if (FLAT::Box::enclose(mbr1,vertices2.at(i)))
                {
                    return true;
                }
            }
            
            for (unsigned int i=0;i<vertices1.size();++i)
            {
                if (mbr2.pointDistance(vertices1.at(i)) < epsilon)
                {
                    return true;
                }
            }
            for (unsigned int i=0;i<vertices2.size();++i)
            {
                if (mbr1.pointDistance(vertices2.at(i)) < epsilon)
                {
                    return true;
                }
            }
            return false;
    }
    // Returns true if touch and false if not by comparing only the centers
    inline bool istouching(FLAT::SpatialObject* sobj1, FLAT::SpatialObject* sobj2)
    {
            return istouchingV(sobj1,sobj2);

            ItemsCompared++;
            
            FLAT::Box mbr1 = sobj1->getMBR();
            FLAT::Box mbr2 = sobj2->getMBR();
            mbr1.isEmpty = false;
            mbr2.isEmpty = false;
            FLAT::Box::expand(mbr1, epsilon);
            
            return FLAT::Box::overlap(mbr1, mbr2);
    }

    struct ComparatorTree_Xaxis : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    FLAT::Vertex r1p,r2p;
                    r1p = r1->mbr.low;
                    r2p = r2->mbr.low;

                            if(r1p[0]<r2p[0])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]<r2p[1])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                            return false;
            }
    };
    struct Comparator_Xaxis : public std::binary_function<FLAT::SpatialObject* const, FLAT::SpatialObject* const, bool>
    {
            bool operator()(FLAT::SpatialObject* const r1, FLAT::SpatialObject* const r2)
            {

                    FLAT::Vertex r1p,r2p;
                    r1p = r1->getMBR().low;
                    r2p = r2->getMBR().low;

                            if(r1p[0]<r2p[0])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]<r2p[1])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                            return false;
            }
    };
    
    void totalTimeStart() { total.start(); };
    void totalTimeStop() { total.stop(); };
    
    virtual void run() {};
    
};

#endif	/* JOINALGORITHM_H */

