/* 
 * File:   S3Hash.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 19:08
 */

#include "S3Hash.h"

S3Hash::S3Hash() {
    algorithm = algo_S3;
}

void S3Hash::init(int level)
{
    initialize.start();
    levels = level;
    resolution = (int*)malloc(sizeof(int)*levels);
    indexOffset = (FLAT::uint64*)malloc(sizeof(FLAT::uint64)*levels);
    totalGridCells = 1;
    universeWidth = (FLAT::Vertex*)malloc(sizeof(FLAT::Vertex)*levels);

    universe = FLAT::Box::combineSafe(universeA,universeB);
    resolution[0] = 1;
    indexOffset[0] = 0;
    FLAT::Vertex difference;
    FLAT::Vertex::differenceVector(universe.high,universe.low,difference);

    for(int l = 1 ; l < levels ; l++)
    {
            resolution[l] = resolution[l-1]*base;
            indexOffset[l] = totalGridCells;
            totalGridCells += pow(resolution[l],DIMENSION);
    }
    if (verbose) cout << "Total cells: " << totalGridCells << endl;
    localPartitions = totalGridCells;

    for(int l = 0 ; l < levels ; l++)
    {
            for (int i=0;i<DIMENSION;++i)
                    universeWidth[l][i] = ceil( (double)difference[i]/(double)resolution[l] );
    }

    initialize.stop();
}

S3Hash::~S3Hash() {
    for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
            delete it->second;
    for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
            delete it->second;

    delete indexOffset;
    delete resolution;
}

void S3Hash::build(SpatialObjectList& a, SpatialObjectList& b)
{
        building.start();
        for(SpatialObjectList::iterator A=a.begin(); A!=a.end(); ++A)
        {
                FLAT::Box mbr = (*A)->getMBR();

                int xMin,yMin,zMin;
                int xMax,yMax,zMax;
                int level;
                for(level = levels - 1; level >= 0 ; level --)
                {
                        vertex2GridLocation(mbr.low,xMin,yMin,zMin,level);
                        vertex2GridLocation(mbr.high,xMax,yMax,zMax,level);
                        if(xMin==xMax && yMin==yMax && zMin == zMax )
                                break;
                }
                FLAT::uint64 index = gridLocation2Index(xMin,yMin,zMin,level);
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
        for(SpatialObjectList::iterator B=b.begin(); B!=b.end(); ++B)
        {
                FLAT::Box mbr = (*B)->getMBR();

                int xMin,yMin,zMin;
                int xMax,yMax,zMax;
                int level;
                for(level = levels - 1; level >= 0 ; level --)
                {
                        vertex2GridLocation(mbr.low,xMin,yMin,zMin,level);
                        vertex2GridLocation(mbr.high,xMax,yMax,zMax,level);
                        if(xMin==xMax && yMin==yMax && zMin == zMax )
                                break;
                }
                FLAT::uint64 index = gridLocation2Index(xMin,yMin,zMin,level);
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
        building.stop();
}

void S3Hash::probe()
{
        //For every cell in level i of A join it with cells touching it from level 0 to L of B
        probing.start();
        for(int levelA = levels-1; levelA >= 0 ; levelA--)
        {
                for(int Ai = 0 ; Ai < resolution[levelA] ; Ai++)
                        for(int Aj = 0 ; Aj < resolution[levelA] ; Aj++)
                                for(int Ak = 0 ; Ak < resolution[levelA] ; Ak++)
                                {
                                        FLAT::uint64 indexA = gridLocation2Index(Ai,Aj,Ak,levelA);
                                        int xMin,yMin,zMin;
                                        //int xMax,yMax,zMax;
                                        xMin = Ai*universeWidth[levelA][0];
                                        yMin = Aj*universeWidth[levelA][1];
                                        zMin = Ak*universeWidth[levelA][2];

                                        //xMax = (Ai+1)*universeWidth[levelA][0];
                                        //yMax = (Aj+1)*universeWidth[levelA][1];
                                        //zMax = (Ak+1)*universeWidth[levelA][2];


                                        for(int levelB = 0; levelB < levelA ; levelB++)
                                        {
                                                int Bi = floor((double)xMin/universeWidth[levelB][0]);
                                                int Bj = floor((double)yMin/universeWidth[levelB][1]);
                                                int Bk = floor((double)zMin/universeWidth[levelB][2]);
                                                FLAT::uint64 indexB = gridLocation2Index(Bi,Bj,Bk,levelB);
                                                // Join indexA of the hash tables A with indexB of Hash Table B
                                                joincells(indexA, indexB);
                                        }
                                        // Join indexA of the two hash tables A and B
                                        joincells(indexA, indexA);

                                        for(int levelB = levelA+1; levelB < levels ; levelB++)
                                        {
                                                int bucketPerDim = pow(base,levelB-levelA);//res[levelB]/res[levelA]
                                                for(int Bi=0 ; Bi<bucketPerDim ; Bi++)
                                                        for(int Bj=0 ; Bj<bucketPerDim ; Bj++)
                                                                for(int Bk=0 ; Bk<bucketPerDim ; Bk++)
                                                                {
                                                                        FLAT::uint64 indexB = gridLocation2Index(Ai*bucketPerDim+Bi,Aj*bucketPerDim+Bj,Ak*bucketPerDim+Bk,levelB);
                                                                        // Join indexA of the A hash tables A with indexB of Hash Table B
                                                                        joincells(indexA, indexB);
                                                                }
                                        }
                                }
        }
        probing.stop();
}

void S3Hash::analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
{
        analyzing.start();
        footprint += dsA.capacity()*(sizeof(FLAT::SpatialObject*));
        footprint += dsB.capacity()*(sizeof(FLAT::SpatialObject*));
        FLAT::uint64 sum=0,sqsum=0;
        for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
        {
                FLAT::uint64 ptrs=((SpatialObjectList*)(it->second))->size();
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;
        }
        for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
        {
                FLAT::uint64 ptrs=((SpatialObjectList*)(it->second))->size();
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;
        }
        footprint += sum*sizeof(SpatialObjectList) +  sizeof(HashValue)*localPartitions;
        avg = (sum+0.0) / (localPartitions+0.0);
        percentageEmpty = (double)(localPartitions - hashTableA.size()- hashTableB.size()) / (double)(localPartitions)*100.0;
        double differenceSquared=0;
        differenceSquared = ((double)sqsum/(double)localPartitions)-avg*avg;
        std = sqrt(differenceSquared);
        analyzing.stop();
}
