/* 
 * File:   ResultPairs.h
 *
 * Class for saving the set of intersecting pairs of objects.
 * Adding new objects does not check the uniqueness
 * deDuplicate function removes duplicate objects (noting the time)
 * Using boost::set for deDuplication
 * 
 * Objects are saved in pairs <object type 0, object type 1>
 * Swapped automatically if needed
 */

#ifndef RESULTPAIRS_H
#define	RESULTPAIRS_H

#include "TreeEntry.h"
#include "TreeNode.h"
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>

#include "Box.hpp"
#include "Timer.hpp"

typedef thrust::host_vector<TreeNode*> NodeList;
typedef thrust::host_vector<TreeEntry*> SpatialObjectList;

typedef std::pair<TreeEntry*,TreeEntry*> ResultPair; //no boost set for thrust
typedef boost::unordered_set< ResultPair > ResultList; // storing unique results

class ResultPairs
{
public:
    SpatialObjectList objA;
    SpatialObjectList objB;

    FLAT::uint64 results;

    FLAT::Timer deDuplicateTime;
    FLAT::uint64 duplicates;

    ResultPairs()
    {
        results = 0;
        duplicates = 0;
    }
    ~ResultPairs()
    {
            objA.clear();
            objB.clear();
    }
    void addPair(TreeEntry* sobjA, TreeEntry* sobjB);
    void deDuplicate();
    void printAllResults()
    {
        FLAT::Box b1; 
        FLAT::Box b2; 
        for (int i = 0; i < objA.size(); i++)
        {
            b1 = objA[i]->getMBR();
            b2 = objB[i]->getMBR();
            std::cout << objA[i]->id << "(" << objA[i]->type << ") " << objB[i]->id << "(" << objB[i]->type << ")" << " [ " << b1 << " ; " << b2 << " ]\n";
        }
    }
};


#endif	/* RESULTPAIRS_H */

