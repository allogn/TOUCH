/* 
 * File:   TOUCH.h
 *
 * 
 */

#ifndef TOUCH_H
#define	TOUCH_H

#include "TOUCHlike.h"

class TOUCH : public TOUCHlike {
public:
    TOUCH();
    virtual ~TOUCH();
    
    void analyze();
    void createPartitions();
    void assignment();
    void probe();
private:
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    void createTreeLevel(vector<TreeEntry*>& input,int Level);
    void joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID);
};

#endif	/* TOUCH_H */

