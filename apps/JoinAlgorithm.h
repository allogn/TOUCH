/* 
 * File:   JoinAlgorithm.h
 * Author: Alvis
 *
 * Pure virtual class for every Spatial Join algorithm used in the project
 */

#ifndef JOINALGORITHM_H
#define	JOINALGORITHM_H

#include <string>
#include "DataFileReader.hpp"
#include "ResultPairs.h"

#define algo_NL				0	//Nested Loop
#define algo_PS				1	//Plane-Sweeping
#define	algo_SGrid			2	//Spatial Grid Hash Join
#define	algo_S3				3	//Size Separation Spatial
#define	algo_TOUCH			4	//TOUCH:Spatial Hierarchical Hash Join
#define	algo_PBSM			5	//Partition Based Spatial-Merge Join
#define	algo_cTOUCH			6	//Partition Based Spatial-Merge Join
#define	algo_dTOUCH			7	//Partition Based Spatial-Merge Join

#define No_Sort				0
#define Hilbert_Sort		1
#define X_axis_Sort			2
#define STR_Sort			3

typedef SpatialObjectList HashValue;
typedef pair<uint64,HashValue*> ValuePair;
typedef boost::unordered_map <uint64,HashValue*> HashTable;

class JoinAlgorithm {
protected:
    uint64 ItemsCompared;
	uint64 gridSize;

	uint64 filtered;
	uint64 hashprobe;
	double repA;
	double repB;

	uint64 footprint;
	uint64 max;
	double percentageEmpty;
	double avg;
	double std;
	Timer sorting;

	Timer building;
	Timer probing;
	Timer analyzing;
	Timer initialize;
	Timer total;
	Timer comparing;
	Timer partition;
    
    
    struct Comparator : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    Vertex r1p,r2p;
                    r1p = r1->mbr.getCenter();
                    r2p = r2->mbr.getCenter();

                    if(PartitioningType == Hilbert_Sort)
                    {
                            double d1[3],d2[3];
                            d1[0]=r1p[0];d1[1]=r1p[1];d1[2]=r1p[2];
                            d2[0]=r2p[0];d2[1]=r2p[1];d2[2]=r2p[2];
                            if (hilbert_ieee_cmp(3,d1,d2)>=0)  return true;
                            else return false;
                    }
                    else
                    {
                            if(r1p[0]<r2p[0])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]<r2p[1])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                            return false;
                    }
            }
    };

    //	Comparing based on the Y axis iff two items are in the same parition
    struct ComparatorY : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    Vertex r1p,r2p;
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
                    Vertex r1p,r2p;
                    r1p = r1->mbr.getCenter();
                    r2p = r2->mbr.getCenter();

                    if(r1p[2]<r2p[2])return true;
                    return false;

            }
    };

    // Returns true if touch and false if not by comparing The corners of the MBRs
    inline bool istouchingV(SpatialObject* sobj1, SpatialObject* sobj2)
    {
            vector<Vertex> vertices;
            Box mbr = sobj2->getMBR();
            //if(algorithm == algo_NL || algorithm == algo_PS)
            //	Box::expand(mbr,epsilon);
            Box::getAllVertices(sobj1->getMBR(),vertices);
            if(algorithm == algo_TOUCH)
                    ItemsCompared++;
            else
                    ItemsCompared++;
            for (unsigned int i=0;i<vertices.size();++i)
                    if (mbr.pointDistance(vertices.at(i)) < epsilon)
                            return true;
            return false;
    }
    // Returns true if touch and false if not by comparing only the centers
    inline bool istouching(SpatialObject* sobj1, SpatialObject* sobj2)
    {
            return istouchingV(sobj1,sobj2);

            if(algorithm == algo_TOUCH)
                    ItemsCompared++;
            else
                    ItemsCompared++;
            return (Vertex::distance(sobj1->getCenter(),sobj2->getCenter()) < epsilon );
    }

    struct ComparatorTree_Xaxis : public std::binary_function<TreeEntry* const, TreeEntry* const, bool>
    {
            bool operator()(TreeEntry* const r1, TreeEntry* const r2)
            {

                    Vertex r1p,r2p;
                    r1p = r1->mbr.low;
                    r2p = r2->mbr.low;

                            if(r1p[0]<r2p[0])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]<r2p[1])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                            return false;
            }
    };
    struct Comparator_Xaxis : public std::binary_function<SpatialObject* const, SpatialObject* const, bool>
    {
            bool operator()(SpatialObject* const r1, SpatialObject* const r2)
            {

                    Vertex r1p,r2p;
                    r1p = r1->getMBR().low;
                    r2p = r2->getMBR().low;

                            if(r1p[0]<r2p[0])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]<r2p[1])return true;
                            if(r1p[0]==r2p[0]&&r1p[1]==r2p[1]&&r1p[2]<r2p[2])return true;
                            return false;
            }
    };
    
    
    
    void JOIN(SpatialObjectList& A, SpatialObjectList& B)
    {
        Ljoin.start();
        if(localJoin == algo_NL)
            NL(A,B);
        else
            PS(A,B);
        Ljoin.stop();
    }
    

public:
    uint64 totalGridCells;
    vector<TreeNode*> tree;		// Append only structure can be replaced by a file "payload"
    vector<TreeEntry*> nextInput;
    TreeEntry* root;
    
    ResultPairs resultPairs;
    
    string logfilename = "SJ.LOG";

    unsigned int leafsize, nodesize;
    unsigned int totalnodes;
    
    string file_dsA;
    string file_dsB;
    
    JoinAlgorithm();
    virtual ~JoinAlgorithm();
    
    virtual void writeNode(vector<TreeEntry*> objlist,int Level);
    virtual void createTreeLevel(vector<TreeEntry*>& input,int Level);
    virtual void probe();

    void readBinaryInput(string file_dsA, string file_dsB);

    vector<TreeEntry*> vdsAll;	//vector of the mixed Objects and their MBRs
    vector<TreeEntry*> vdsA;	//vector of the Objects and their MBRs of the smaller dataset ??@todo smallest?
    vector<TreeEntry*> vdsB;       

    uint64 size_dsA,size_dsB;

    void print();
    
    
    //Nested Loop join algorithm
    void NL(SpatialObjectList& A, SpatialObjectList& B)
    {
        for(SpatialObjectList::iterator itA = A.begin(); itA != A.end(); ++itA)
            for(SpatialObjectList::iterator itB = B.begin(); itB != B.end(); ++itB)
                if ( istouching(*itA , *itB) )
                {
                    resultPairs.addPair(*itA , *itB);
                }
    }

    //Plane-sweeping join algorithm
    void PS(SpatialObjectList& A, SpatialObjectList& B);
    
private:
    
};

#endif	/* JOINALGORITHM_H */

