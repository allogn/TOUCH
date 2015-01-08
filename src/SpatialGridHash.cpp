/* 
 * File:   SpatialGridHash.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 23:21
 */

#include "SpatialGridHash.h"

void SpatialGridHash::init(const FLAT::Box& universeExtent,const int gridResolutionPerDimension)	
{
        initialize.start();
        resolution = gridResolutionPerDimension;
        universe = universeExtent;
        FLAT::Vertex difference;
        FLAT::Vertex::differenceVector(universe.high,universe.low,difference);
        for (int i=0;i<DIMENSION;++i)
                universeWidth[i] = difference[i]/resolution;
        localPartitions = (FLAT::uint64)(resolution * resolution) * resolution;

        initialize.stop();
}

SpatialGridHash::~SpatialGridHash() {
    for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
            delete it->second;
}

void SpatialGridHash::analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
{

        analyzing.start();
        footprint += dsA.capacity()*(sizeof(TreeEntry*));
        footprint += dsB.capacity()*(sizeof(TreeEntry*));
        cout << "Cell Width: " << universeWidth  << endl;
        
        FLAT::uint64 sum=0,sqsum=0;
        for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
        {
                FLAT::uint64 ptrs=((SpatialObjectList*)(it->second))->size();
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;
        }
        footprint += sum*sizeof(TreeEntry*) +  sizeof(HashValue)*localPartitions;
        avg = (sum+0.0) / (localPartitions+0.0);
        percentageEmpty = (double)(localPartitions - gridHashTable.size()) / (double)(localPartitions)*100.0;
        repA = (double)(sum)/(double)size_dsA;
        double differenceSquared=0;
        differenceSquared = ((double)sqsum/(double)localPartitions)-avg*avg;
        std = sqrt(differenceSquared);
        analyzing.stop();
}

void SpatialGridHash::build(SpatialObjectList& dsA)
{
        building.start();
        gridHashTable.clear();
        for(SpatialObjectList::iterator i=dsA.begin(); i!=dsA.end(); ++i)
        {
                vector<FLAT::uint64> cells;
                getOverlappingCells(*i,cells);
                for (vector<FLAT::uint64>::iterator j = cells.begin(); j!=cells.end(); ++j)
                {
                        HashTable::iterator it= gridHashTable.find(*j);
                        if (it==gridHashTable.end())
                        {
                                HashValue* soList = new HashValue();
                                soList->push_back(*i);
                                gridHashTable.insert( ValuePair(*j,soList) );
                        }
                        else
                        {
                                it->second->push_back(*i);
                        }
                }
        }
        building.stop();
}

void SpatialGridHash::clear()
{
        gridHashTable.clear();
        //for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
        // delete	it->second;
}

void SpatialGridHash::probe(TreeEntry*& obj)
{
        probing.start();
        
        vector<FLAT::uint64> cells;
        if (!getProjectedCells( obj , cells ))
        {
                filtered[obj->type]++;
                return;
        }
        ///// Get Unique Objects from Grid Hash in Vicinity
        hashprobe += cells.size();
        for (vector<FLAT::uint64>::const_iterator j = cells.begin(); j!=cells.end(); ++j)
        {
                HashTable::iterator it = gridHashTable.find(*j);
                if (it==gridHashTable.end()) continue;
                HashValue* soList = it->second;
                for (HashValue::const_iterator k=soList->begin(); k!=soList->end(); ++k)
                        if ( istouching( obj , *k) )
                        {
                            resultPairs.addPair(obj , *k);
                        }
        }

        probing.stop();
}


void SpatialGridHash::probe(const SpatialObjectList& dsB)
{
    probing.start();

    for(SpatialObjectList::const_iterator i=dsB.begin(); i!=dsB.end(); ++i)
    {
        vector<FLAT::uint64> cells;
        if (!getProjectedCells( *i , cells ))
        {
            filtered[(*i)->type]++;
            continue;
        }
        ///// Get Unique Objects from Grid Hash in Vicinity
        hashprobe += cells.size();

        for (vector<FLAT::uint64>::const_iterator j = cells.begin(); j!=cells.end(); ++j)
        {
            HashTable::iterator it = gridHashTable.find(*j);
            if (it==gridHashTable.end()) continue;
            HashValue* soList = it->second;
            for (HashValue::const_iterator k=soList->begin(); k!=soList->end(); ++k)
            {
                if ( istouching( *i , *k) )
                {
                    resultPairs.addPair(*i , *k);
                }
            }
        }
    }

    probing.stop();
}

void SpatialGridHash::transferInfo(SpatialGridHash* sgh, JoinAlgorithm* alg)
{
    alg->ItemsCompared += sgh->ItemsCompared;
    alg->resultPairs.results += sgh->resultPairs.results;
    alg->resultPairs.duplicates += sgh->resultPairs.duplicates;
    alg->repA += sgh->repA;
    alg->repB += sgh->repB;
    alg->resultPairs.deDuplicateTime.add(sgh->resultPairs.deDuplicateTime);
    alg->initialize.add(sgh->initialize);
}