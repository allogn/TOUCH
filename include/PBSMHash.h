/* 
 * File:   PBSMHash.h
 * Author: Alvis
 *
 * Created on 31 октября 2014 г., 17:31
 */

#ifndef PBSMHASH_H
#define	PBSMHASH_H

#include "JoinAlgorithm.h"

using namespace FLAT;

// The class for doing the Partition Based Spatial Merge Join
class PBSMHash : JoinAlgorithm
{

private:

	HashTable hashTableA, hashTableB;
	Box universe;
	Vertex universeWidth;
	int resolution;
	uint64 totalGridCells;

	uint64 gridLocation2Index(const int x,const int y,const int z)
	{
		return  x + (y*resolution) + (z*resolution*resolution);
	}

	void vertex2GridLocation(const Vertex& v,int& x,int& y,int &z)
	{
		x = (v[0] > universe.low[0])?(int)floor( (v[0] - universe.low[0]) / universeWidth[0]) : 0;
		y = (v[1] > universe.low[1])?(int)floor( (v[1] - universe.low[1]) / universeWidth[1]) : 0;
		z = (v[2] > universe.low[2])?(int)floor( (v[2] - universe.low[2]) / universeWidth[2]) : 0;

		// if cell not valid assign last corner cells
		if (x>=resolution) x=resolution-1;
		if (y>=resolution) y=resolution-1;
		if (z>=resolution) z=resolution-1;

	}

	bool vertex2GridLocation(const Vertex& v,int& x,int& y,int &z, bool islower)
	{
		x = (v[0] > universe.low[0])?(int)floor( (v[0] - universe.low[0]) / universeWidth[0]) : 0;
		y = (v[1] > universe.low[1])?(int)floor( (v[1] - universe.low[1]) / universeWidth[1]) : 0;
		z = (v[2] > universe.low[2])?(int)floor( (v[2] - universe.low[2]) / universeWidth[2]) : 0;

		// if cell not valid assign last corner cells
		if (x>=resolution) x=resolution-1;
		if (y>=resolution) y=resolution-1;
		if (z>=resolution) z=resolution-1;

		if ( (!islower) && ( x<0 || y<0 || z<0 ))
			return true;
		if( islower && ( x>=resolution || y>=resolution || z>=resolution))
			return true;
		return false;

	}

public:

	PBSMHash(const Box& universeExtent, uint64 partitionPerDim);
	~PBSMHash();

	void build(SpatialObjectList& a, SpatialObjectList& b);
    
	//join two given cells at the given level
	void joincells(const uint64 indexA, const uint64 indexB);
	void probe();
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
};


#endif	/* PBSMHASH_H */

