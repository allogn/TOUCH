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
    TOUCH();
    TOUCH(const TOUCH& orig);
    virtual ~TOUCH();
    
    void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB);
    void createPartitions();
    void assignment(const SpatialObjectList& ds);
    void probe(ResultPairs& results);
private:
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    void createTreeLevel(vector<TreeEntry*>& input,int Level);
    void joinIntenalnodetoleafs(uint64 ancestorNodeID, ResultPairs& results);
};

#endif	/* TOUCH_H */

