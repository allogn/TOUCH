/* 
 * File:   SpatialGridHash.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 23:21
 */

#include "SpatialGridHash.h"

SpatialGridHash::SpatialGridHash(const FLAT::Box& universeExtent,const int gridResolutionPerDimension)	
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
        footprint += dsA.capacity()*(sizeof(FLAT::SpatialObject*));
        footprint += dsB.capacity()*(sizeof(FLAT::SpatialObject*));
        cout << "Cell Width: " << universeWidth  << endl;
//		cout << "Bucket Count: " << gridHashTable.bucket_count() <<endl;
//		cout << "Max Bucket Count: " << gridHashTable.max_bucket_count() << endl;
//		cout << "The average number of elements per bucket: " << gridHashTable.load_factor() << endl;
//		cout << "Max Load Factor: " << gridHashTable.max_load_factor() << endl;
//		cout << "size(" << gridHashTable.size() << ") of the largest possible container: " << gridHashTable.max_size();

        FLAT::uint64 sum=0,sqsum=0;
        for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
        {
                FLAT::uint64 ptrs=((SpatialObjectList*)(it->second))->size();
                sum += ptrs;
                sqsum += ptrs*ptrs;
                if (maxMappedObjects<ptrs) maxMappedObjects = ptrs;
        }
        footprint += sum*sizeof(FLAT::SpatialObject*) +  sizeof(HashValue)*localPartitions;
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
            //if ((*i)->id == 5579) cout << "trying with 5579" << endl;
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
                                //if ((*i)->id == 5579) cout << "new cell with 5579 " << gridHashTable.size() << endl;
                        }
                        else
                        {
                                it->second->push_back(*i);
                                //if ((*i)->id == 5579) cout << "old cell with 5579 " << it->first << endl;
                        }
                }
        }
        building.stop();
        //cout<< "Number of assigned object " << assigned << " from " << dsA.size()<<endl;
}

void SpatialGridHash::clear()
{
        gridHashTable.clear();
        //for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
        // delete	it->second;
}

void SpatialGridHash::probe(FLAT::SpatialObject*& obj)
{
        probing.start();
        
        //set to zero variables to use same grid many times
//        filtered[obj->type] = 0;
//        hashprobe = 0;
//        resultPairs = ResultPairs();
        
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
 
//        filtered[0] = 0;
//        filtered[1] = 0;
//        hashprobe = 0;
//        resultPairs = ResultPairs();
        
        
        for(SpatialObjectList::const_iterator i=dsB.begin(); i!=dsB.end(); ++i)
        {
          // if ((*i)->id == 5579) cout << "----------------- ============== probe 5579" << endl;
           // if ((*i)->id == 2572) cout << "----------------- ============== probe 2572" << endl;
                vector<FLAT::uint64> cells;
                if (!getProjectedCells( *i , cells ))
                {
                        filtered[(*i)->type]++;
                        continue;
                }
                
          //  if ((*i)->id == 2572) cout << "----------------- ============== probe 2572 test 2" << endl;
                ///// Get Unique Objects from Grid Hash in Vicinity
                hashprobe += cells.size();
                
                if ((*i)->id == 2572) 
                for (vector<FLAT::uint64>::const_iterator j = cells.begin(); j!=cells.end(); ++j)
                {
                        HashTable::iterator it = gridHashTable.find(*j);
                        if (it==gridHashTable.end()) continue;
                        HashValue* soList = it->second;
//                        for (HashValue::const_iterator k=soList->begin(); k!=soList->end(); ++k)
//                        {
//                            cout << "list of objects of " << it->first << " " << (*k)->id << endl;
//                        }
                }
                
                
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

void SpatialGridHash::probe(TreeNode* leaf)
{
        probing.start();
        for (unsigned int child = 0; child < leaf->entries.size(); ++child)
        {
                vector<FLAT::uint64> cells;
                if (!getProjectedCells( leaf->entries.at(child)->mbr  , cells ))
                {
                        filtered[0]++;
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
                                if ( istouching( leaf->entries.at(child)->obj , *k) )
                                {
                                    resultPairs.addPair(leaf->entries.at(child)->obj , *k);
                                }
                        }
                }

        }
        probing.stop();
}
