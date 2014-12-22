/* 
 * File:   ResultPairs.cpp
 */

#include "ResultPairs.h"

void ResultPairs::deDuplicate()
{
        deDuplicateTime.start();
        results = 0;
        ResultList uniqueResults;
        for (FLAT::uint64 i=0;i<objA.size();++i)
                uniqueResults.insert( ResultPair(objA[i],objB[i]) );

        duplicates = objA.size() - uniqueResults.size();
        objA.clear();
        objB.clear();
        ResultList::iterator it;
        for(it= uniqueResults.begin(); it!=uniqueResults.end(); it++)
                addPair(it->first,it->second);

        deDuplicateTime.stop();
}

void ResultPairs::addPair(TreeEntry* sobjA, TreeEntry* sobjB)
{
        results++;

        if (sobjA->type != 0)
        {
                //swap objects
                TreeEntry* temp;
                temp = sobjA;
                sobjA = sobjB;
                sobjB = temp;
        }

        objA.push_back(sobjA);
        objB.push_back(sobjB);
}