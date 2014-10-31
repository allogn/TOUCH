/* 
 * File:   ResultPairs.cpp
 * Author: Alvis
 * 
 * Created on 30 октября 2014 г., 14:49
 */

#include "ResultPairs.h"

void ResultPairs::deDuplicate()
{
        deDuplicate.start();

        set<pair<SpatialObject*,SpatialObject*> > uniqueResults;
        for (uint64 i=0;i<objA.size();++i)
                uniqueResults.insert( pair<SpatialObject*,SpatialObject*>(objA.at(i),objB.at(i)) );

        duplicates = objA.size() - uniqueResults.size();
        objA.clear();
        objB.clear();
        set< pair<SpatialObject*,SpatialObject*> >::iterator it;
        for(it= uniqueResults.begin(); it!=uniqueResults.end(); it++)
                addPair(it->first,it->second);

        deDuplicate.stop();
}

void ResultPairs::addPair(SpatialObject* sobjA, SpatialObject* sobjB)
{
        results++;

        if (sobjA->type != 0)
        {
                //swap objects
                SpatialObject* temp;
                temp = sobjA;
                sobjA = sobjB;
                sobjB = temp;
        }

        objA.push_back(sobjA);
        objB.push_back(sobjB);
}