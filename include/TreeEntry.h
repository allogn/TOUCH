/* 
 * File:   TreeEntry.h
 * Author: Alvis
 *
 */

#ifndef TREEENTRY_H
#define	TREEENTRY_H

#define TYPES 2

#include <Box.hpp>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>

class TreeEntry
{
public:
    FLAT::Box mbr;
    FLAT::SpatialObject* obj;

    int cost;
    int id;
    int type;

    // make a Leaf item
    TreeEntry(FLAT::SpatialObject* object, int ntype, int nid, double epsilon)
    {
        obj = object;
        mbr = obj->getMBR();
        mbr.isEmpty = false;
        FLAT::Box::expand(mbr, (double)epsilon/2.);
    }
    
    FLAT::Box& getMBR()
    {
        return mbr;
    }
};


#endif	/* TREEENTRY_H */

