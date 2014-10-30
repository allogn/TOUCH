/* 
 * File:   ResultPairs.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 14:49
 */

#ifndef RESULTPAIRS_H
#define	RESULTPAIRS_H

//The class for storing the result's pairs
class ResultPairs
{
public:
	SpatialObjectList objA;
	SpatialObjectList objB;
	ResultPairs()
	{
	}
	~ResultPairs()
	{
		objA.clear();
		objB.clear();
	}
	void addPair(SpatialObject* sobjA, SpatialObject* sobjB)
	{
		stats.results++;
		statsTOUCH.results++;
		return;

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

	void deDuplicate()
	{
		stats.deDuplicate.start();

		set<pair<SpatialObject*,SpatialObject*> > uniqueResults;
		for (uint64 i=0;i<objA.size();++i)
			uniqueResults.insert( pair<SpatialObject*,SpatialObject*>(objA.at(i),objB.at(i)) );

		stats.duplicates = objA.size() - uniqueResults.size();
		objA.clear();
		objB.clear();
		set< pair<SpatialObject*,SpatialObject*> >::iterator it;
		for(it= uniqueResults.begin(); it!=uniqueResults.end(); it++)
			addPair(it->first,it->second);
		stats.deDuplicate.stop();
	}

};


#endif	/* RESULTPAIRS_H */

