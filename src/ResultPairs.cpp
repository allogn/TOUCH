/* 
 * File:   ResultPairs.cpp
 */

#include "ResultPairs.h"

void ResultPairs::deDuplicate()
{
        deDuplicateTime.start();

        std::set<std::pair<FLAT::SpatialObject*,FLAT::SpatialObject*> > uniqueResults;
        for (FLAT::uint64 i=0;i<objA.size();++i)
                uniqueResults.insert( std::pair<FLAT::SpatialObject*,FLAT::SpatialObject*>(objA.at(i),objB.at(i)) );

        duplicates = objA.size() - uniqueResults.size();
        objA.clear();
        objB.clear();
        std::set< std::pair<FLAT::SpatialObject*,FLAT::SpatialObject*> >::iterator it;
        for(it= uniqueResults.begin(); it!=uniqueResults.end(); it++)
                addPair(it->first,it->second);

        deDuplicateTime.stop();
}

void ResultPairs::addPair(FLAT::SpatialObject* sobjA, FLAT::SpatialObject* sobjB)
{
        results++;

        if (sobjA->type != 0)
        {
                //swap objects
                FLAT::SpatialObject* temp;
                temp = sobjA;
                sobjA = sobjB;
                sobjB = temp;
        }

        objA.push_back(sobjA);
        objB.push_back(sobjB);
}