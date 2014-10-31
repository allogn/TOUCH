/* 
 * File:   ResultPairs.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 14:49
 */

#ifndef RESULTPAIRS_H
#define	RESULTPAIRS_H

#include "Segment.hpp"
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>


using namespace FLAT;

typedef std::vector<SpatialObject*> SpatialObjectList;
typedef pair<SpatialObject*,SpatialObject*> ResultPair;
typedef boost::unordered_set< ResultPair > ResultList; // storing unique results

//The class for storing the result's pairs
class ResultPairs
{
public:
	SpatialObjectList objA;
	SpatialObjectList objB;
    
	uint64 results;
    
	Timer deDuplicate;
	uint64 duplicates;
    
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
	void addPair(SpatialObject* sobjA, SpatialObject* sobjB);
	void deDuplicate();

};


#endif	/* RESULTPAIRS_H */

