/* 
 * File:   SpatialGridHash.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 23:21
 */

#ifndef SPATIALGRIDHASH_H
#define	SPATIALGRIDHASH_H

#include "JoinAlgorithm.h"

using namespace std;
using namespace FLAT;

// The class for doing the Spatial Grid Join
class SpatialGridHash : public JoinAlgorithm
{

private:

	HashTable gridHashTable;
	FLAT::Box universe;
	FLAT::Vertex universeWidth;
	FLAT::int64 resolution;

	FLAT::int64 gridLocation2Index(const int x,const int y,const int z)
	{
		return (x + (y*resolution) + (z*resolution*resolution));
	}

	void vertex2GridLocation(const FLAT::Vertex& v,int& x,int& y,int &z)
	{
		x = (int)floor( (v[0] - universe.low[0]) / universeWidth[0]);
		y = (int)floor( (v[1] - universe.low[1]) / universeWidth[1]);
		z = (int)floor( (v[2] - universe.low[2]) / universeWidth[2]);

		if (x<0) x=0; if (x>=resolution) x=resolution-1;
		if (y<0) y=0; if (y>=resolution) y=resolution-1;
		if (z<0) z=0; if (z>=resolution) z=resolution-1;
	}
	bool vertex2GridLocation(const FLAT::Vertex& v,int& x,int& y,int &z, bool islower)
	{
		x = (int)floor( (v[0] - universe.low[0]) / universeWidth[0]);
		y = (int)floor( (v[1] - universe.low[1]) / universeWidth[1]);
		z = (int)floor( (v[2] - universe.low[2]) / universeWidth[2]);

		if ( (!islower) && ( x<0 || y<0 || z<0 ))
			return true;
		if( islower && ( x>=resolution || y>=resolution || z>=resolution))
			return true;

		// if cell not valid assign last corner cells
		if (x<0) x=0; if (x>=resolution) x=resolution-1;
		if (y<0) y=0; if (y>=resolution) y=resolution-1;
		if (z<0) z=0; if (z>=resolution) z=resolution-1;

		return false;

	}

	void getOverlappingCells(SpatialObject* sobj,vector<FLAT::uint64>& cells)
	{
		Box mbr = sobj->getMBR();
		//cout<<mbr.low << " " << mbr.high << " " << epsilon<< endl;
		if(algorithm == algo_SGrid)
		{
			Box::expand(mbr,epsilon);
		}
		//cout<<mbr.low << " " << mbr.high << " " << epsilon<< endl;

		int xMin,yMin,zMin;
		vertex2GridLocation(mbr.low,xMin,yMin,zMin);

		int xMax,yMax,zMax;
		vertex2GridLocation(mbr.high,xMax,yMax,zMax);

		for (int i=xMin;i<=xMax;++i)
			for (int j=yMin;j<=yMax;++j)
				for (int k=zMin;k<=zMax;++k)
					cells.push_back( gridLocation2Index(i,j,k) );

	}

	void getCellMbr(Box& extent,const int x,const int y,const int z)
	{
		extent.low[0] = x*universeWidth[0]; extent.high[0] = (x+1)*universeWidth[0];
		extent.low[1] = y*universeWidth[1]; extent.high[1] = (y+1)*universeWidth[1];
		extent.low[2] = z*universeWidth[2]; extent.high[2] = (z+1)*universeWidth[2];

		extent.low = extent.low + universe.low;
		extent.high = extent.high + universe.low;
	}

	bool getProjectedCells(SpatialObject* sobj,vector<uint64>& cells)
	{
		Box mbr = sobj->getMBR();

		if (!Box::overlap(mbr,universe)) return false;

		int xMin,xMax,yMin,yMax,zMin,zMax;
		vertex2GridLocation(mbr.low,xMin,yMin,zMin);
		vertex2GridLocation(mbr.high,xMax,yMax,zMax);

		for (int i=xMin;i<=xMax;++i)
			for (int j=yMin;j<=yMax;++j)
				for (int k=zMin;k<=zMax;++k)
				{
					cells.push_back( gridLocation2Index(i,j,k) );
				}
		return true;
	}
	bool getProjectedCells(Box& mbr,vector<uint64>& cells)
	{

		if (!Box::overlap(mbr,universe)) return false;

		int xMin,xMax,yMin,yMax,zMin,zMax;
		vertex2GridLocation(mbr.low,xMin,yMin,zMin);
		vertex2GridLocation(mbr.high,xMax,yMax,zMax);

		for (int i=xMin;i<=xMax;++i)
			for (int j=yMin;j<=yMax;++j)
				for (int k=zMin;k<=zMax;++k)
				{
					cells.push_back( gridLocation2Index(i,j,k) );
				}
		return true;
	}

public:

	SpatialGridHash(const Box& universeExtent,const int gridResolutionPerDimension);
	~SpatialGridHash();
    
	void build(SpatialObjectList& dsA);
	void clear();
	void probe(const SpatialObjectList& dsB);
	void probe(TreeNode* leaf);
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
};

#endif	/* SPATIALGRIDHASH_H */

