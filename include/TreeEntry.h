/* 
 * File:   TreeEntry.h
 * Author: Alvis
 *
 */

#ifndef TREEENTRY_H
#define	TREEENTRY_H

#define TYPES 2

#include <Box.hpp>

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
        
        
        FLAT::uint64 num[TYPES];// number of objects assigned below
	
	// make a Leaf item
	TreeEntry(FLAT::SpatialObject* object)
	{
            obj = object;
            mbr = obj->getMBR();
            mbr.isEmpty = false; //@todo all mbr's are empty at the beginning!!!
            mbrL[obj->type] = mbr;
	}

	//make an Internal item
	TreeEntry(const FLAT::Box& Mbr, FLAT::uint64 child) //left for compatibility
	{
                mbrL[0] = Mbr;
		mbr = Mbr;
                mbrL[0] = Mbr;
		childIndex = child;
		parentIndex = 0;
	}
	
	TreeEntry(const FLAT::Box& MbrA, const FLAT::Box& MbrB, FLAT::uint64 child)
	{
		mbrL[1] = MbrB;
		mbrL[0] = MbrA;
                mbrK[0] = MbrA;
                mbrK[1] = MbrB;
		parentIndex = 0;
		childIndex = child;
		mbr = FLAT::Box::combineSafe(MbrB,MbrA); // sorting if cTOUCH
	}

        void expand(double epsilon)
        {
            FLAT::Box::expand(mbr,epsilon * 1./2.); // all objects must also be expanded on epsilon/2
            
            if (!mbrL[0].isEmpty)
                FLAT::Box::expand(mbrL[0],epsilon * 1./2.);
            if (!mbrL[1].isEmpty)
                FLAT::Box::expand(mbrL[1],epsilon * 1./2.);
        }
};


#endif	/* TREEENTRY_H */

