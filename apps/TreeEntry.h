/* 
 * File:   TreeEntry.h
 * Author: Alvis
 *
 */

#ifndef TREEENTRY_H
#define	TREEENTRY_H

#include "Box.hpp"
#include "SpatialObject.hpp"

class TreeEntry
{
public:
	FLAT::Box mbrK[types]; //combined black and light MBR of each type
	FLAT::Box mbr; //mbr used for sorting or old TOUCH versions

	FLAT::Box mbrSelfD[types]; //mbr's of objects which were assigned to current node
	
	FLAT::Box mbrL[types]; //light mbr
	FLAT::Box mbrD[types]; //dark mbr
	
	FLAT::uint64 childIndex;
	FLAT::uint64 parentIndex;
	SpatialObject* obj;
	
	// make a Leaf item
	TreeEntry(SpatialObject* object)
	{
		obj = object;
		mbrL[obj->type] = obj->getMBR();
		FLAT::Box::expand(obj->getMBR(),epsilon);
		mbr = obj->getMBR();
	}

	//make an Internal item
	TreeEntry(const FLAT::Box& Mbr, FLAT::uint64 child) //left for compatibility
	{
		mbr = Mbr;
		childIndex = child;
		parentIndex = 0;
	}
	
	TreeEntry(const FLAT::Box& MbrA, const FLAT::Box& MbrB, FLAT::uint64 child)
	{
		mbrL[1] = MbrB;
		mbrL[0] = MbrA;
		parentIndex = 0;
		childIndex = child;
		FLAT::Box::combine(MbrB,MbrA,mbr); //for sorting
	}

};


#endif	/* TREEENTRY_H */

