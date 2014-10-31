/* 
 * File:   TOUCH.h
 * Author: Alvis
 *
 * 
 */

#ifndef TOUCH_H
#define	TOUCH_H

#include <vector>

#include "TOUCHlike.h"
#include "ResultPairs.h"

class TOUCH : public TOUCHlike {
public:
    TOUCH(int buckets);
    virtual ~TOUCH();
    
    void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
    void createPartitions();
    void assignment(const SpatialObjectList& ds);
    void probe();
private:
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    void createTreeLevel(vector<TreeEntry*>& input,int Level);
    void joinIntenalnodetoleafs(uint64 ancestorNodeID);
};

#endif	/* TOUCH_H */

