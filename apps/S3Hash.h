/* 
 * File:   S3Hash.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 19:08
 */

#ifndef S3HASH_H
#define	S3HASH_H

#include "JoinAlgorithm.h"

//The class for doing the Size Separation Spatial Join
class S3Hash : public JoinAlgorithm
{

private:

	HashTable hashTableA, hashTableB;
	Box universe;
	Vertex* universeWidth;
	int levels;
	int* resolution;
	uint64* indexOffset;	// Starting index of every level in the hash tables
	uint64 totalGridCells;

	uint64 gridLocation2Index(const int x,const int y,const int z, const int level)
	{
		return  indexOffset[level]+(x + (y*resolution[level]) + (z*resolution[level]*resolution[level]));
	}
	void vertex2GridLocation(const Vertex& v,int& x,int& y,int &z, const int level)
	{
		x = (v[0] > universe.low[0])?(int)floor( (v[0] - universe.low[0]) / universeWidth[level][0]):0;
		y = (v[1] > universe.low[1])?(int)floor( (v[1] - universe.low[1]) / universeWidth[level][1]):0;
		z = (v[2] > universe.low[2])?(int)floor( (v[2] - universe.low[2]) / universeWidth[level][2]):0;

		// if cell not valid assign last corner cells
		if (x>=resolution[level]) x=resolution[level]-1;
		if (y>=resolution[level]) y=resolution[level]-1;
		if (z>=resolution[level]) z=resolution[level]-1;
	}

public:

	S3Hash(const Box& universeExtent, int level);

	~S3Hash();

	void build(SpatialObjectList& a, SpatialObjectList& b);
    
	//join two given cells at the given level
	void joincells(const uint64 indexA, const uint64 indexB)
	{
		// join objects in the cells A and B
		SpatialObjectList A,B;

		HashTable::iterator hA = hashTableA.find(indexA);
		if (hA==hashTableA.end()) return;
		A = *( hA->second );

		HashTable::iterator hB = hashTableB.find(indexB);
		if (hB==hashTableB.end()) return;
		B = *( hB->second );
		JOIN(A,B);
	}
    
	void probe();
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
};


#endif	/* S3HASH_H */

