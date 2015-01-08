/* 
 * File:   PBSMHash.cpp
 * Author: Alvis
 * 
 * Created on 31 октября 2014 г., 17:31
 */

#include "PBSMHash.h"

void PBSMHash::init(FLAT::uint64 partitionPerDim)
{
    initialize.start();
    resolution = partitionPerDim;
    totalGridCells = pow(resolution,3);
    universe = FLAT::Box::combineSafe(universeA,universeB);
    FLAT::Vertex difference;
    FLAT::Vertex::differenceVector(universe.high,universe.low,difference);

    localPartitions = totalGridCells;

    for (int i=0;i<DIMENSION;++i)
            universeWidth[i] = ceil( (double)difference[i]/(double)resolution );

    initialize.stop();
}

PBSMHash::~PBSMHash() {
    for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
            delete it->second;
    for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
            delete it->second;
}

void PBSMHash::analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
{
        analyzing.start();
        footprint += dsA.capacity()*(sizeof(FLAT::SpatialObject*));
        footprint += dsB.capacity()*(sizeof(FLAT::SpatialObject*));
        FLAT::uint64 sum=0,sqsum=0,sumA,sumB;
        for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
        {
                FLAT::uint64 ptrs=((SpatialObjectList*)(it->second))->size();
                sumA += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;
        }

        for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
        {
                FLAT::uint64 ptrs=((SpatialObjectList*)(it->second))->size();
                sumB += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;
        }
        sum = sumA+sumB;
        footprint += sum*sizeof(FLAT::SpatialObject*) + 2*sizeof(HashValue)*localPartitions;
        avg = (sum+0.0) / (2*localPartitions+0.0);
        repA = (double)(sumA)/(double)size_dsA;
        repB = (double)(sumB)/(double)size_dsB;
        percentageEmpty = (double)(2*localPartitions - hashTableA.size()- hashTableB.size()) / (double)(2*localPartitions)*100.0;
        double differenceSquared=0;
        differenceSquared = ((double)sqsum/(double)(2*localPartitions))-avg*avg;
        std = sqrt(differenceSquared);
        analyzing.stop();
}

void PBSMHash::probe()
{
    //For every cell of A join it with its corresponding cell of B
    probing.start();
    for(int i = 0 ; i < resolution ; i++)
            for(int j = 0 ; j < resolution ; j++)
                    for(int k = 0 ; k < resolution ; k++)
                    {
                            FLAT::uint64 index = gridLocation2Index(i,j,k);
                            // Join indexA of the two hash tables A and B
                            joincells(index, index);
                    }

    probing.stop();
}

//join two given cells at the given level
void PBSMHash::joincells(const FLAT::uint64 indexA, const FLAT::uint64 indexB)
{
        // join objects in the cells A and B
        SpatialObjectList A,B;

        HashTable::iterator hA = hashTableA.find(indexA);
        if (hA==hashTableA.end()) return;
        A = *( hA->second );

        HashTable::iterator hB = hashTableB.find(indexB);
        if (hB==hashTableB.end()) return;
        B = *( hB->second );
        NL(A,B);
}

void PBSMHash::build(SpatialObjectList& a, SpatialObjectList& b)
{
        building.start();
        double exp = epsilon * 0.5;
        for(SpatialObjectList::iterator A=a.begin(); A!=a.end(); ++A)
        {
                FLAT::Box mbr = (*A)->getMBR();

                FLAT::Box::expand(mbr,exp);

                int xMin,yMin,zMin;
                int xMax,yMax,zMax;
                vertex2GridLocation(mbr.low,xMin,yMin,zMin);
                vertex2GridLocation(mbr.high,xMax,yMax,zMax);

                for(int x=xMin; x<=xMax; x++)
                        for(int y=yMin; y<=yMax; y++)
                                for(int z=zMin; z<=zMax; z++)
                                {
                                        FLAT::uint64 index = gridLocation2Index(x,y,z);
                                        HashTable::iterator it= hashTableA.find(index);
                                        if (it==hashTableA.end())
                                        {
                                                HashValue* soList = new HashValue();
                                                soList->push_back(*A);
                                                hashTableA.insert( ValuePair(index,soList) );
                                        }
                                        else
                                        {
                                                it->second->push_back(*A);
                                        }
                                }

        }
        for(SpatialObjectList::iterator B=b.begin(); B!=b.end(); ++B)
        {
                FLAT::Box mbr = (*B)->getMBR();

                FLAT::Box::expand(mbr,exp);
                if (!FLAT::Box::overlap(mbr,universe))
                {
                        filtered[0]++;
                        continue;
                }

                int xMin,yMin,zMin;
                int xMax,yMax,zMax;
                bool filtered = false;
                filtered |= vertex2GridLocation(mbr.low,xMin,yMin,zMin,true);
                filtered |= vertex2GridLocation(mbr.high,xMax,yMax,zMax,false);

                if(filtered)
                        filtered ++;
                else
                {
                for(int x=xMin; x<=xMax; x++)
                        for(int y=yMin; y<=yMax; y++)
                                for(int z=zMin; z<=zMax; z++)
                                {
                                        FLAT::uint64 index = gridLocation2Index(x,y,z);
                                        HashTable::iterator it= hashTableB.find(index);
                                        if (it==hashTableB.end())
                                        {
                                                HashValue* soList = new HashValue();
                                                soList->push_back(*B);
                                                hashTableB.insert( ValuePair(index,soList) );
                                        }
                                        else
                                        {
                                                it->second->push_back(*B);
                                        }
                                }
                }
        }
        building.stop();
}
