/* 
 * File:   TreeEntry.h
 * Author: Alvis
 *
 */

#ifndef TREEENTRY_H
#define	TREEENTRY_H

#define TYPES 2

#include <Box.hpp>
#include <SpatialObject.hpp>

class TreeEntry
{
public:
        
	FLAT::Box mbrK[TYPES]; //combined black and light MBR of each type
	FLAT::Box mbr; //mbr used for sorting or old TOUCH versions

	FLAT::Box mbrSelfD[TYPES]; //mbr's of objects which were assigned to current node
	
	FLAT::Box mbrL[TYPES]; //light mbr
	FLAT::Box mbrD[TYPES]; //dark mbr
	
	FLAT::uint64 childIndex;
	FLAT::uint64 parentIndex;
	FLAT::SpatialObject* obj;
	
	// make a Leaf item
	TreeEntry(FLAT::SpatialObject* object)
	{
            obj = object;
            mbrL[obj->type] = obj->getMBR();
            mbr = obj->getMBR();
            FLAT::Box::expand(mbr,1.5); //@todo get epsilon here
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

