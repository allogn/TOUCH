/* 
 * File:   TOUCH.h
 *
 * 
 */

#ifndef TOUCH_H
#define	TOUCH_H

#include "CommonTOUCH.h"

class TOUCH : public CommonTOUCH {
public:
    TOUCH();
    virtual ~TOUCH();
    
    void run();
    
    void analyze();
    void assignment();
    void probe();
private:
    void writeNode(std::vector<TreeEntry*> objlist,int Level);
    void joinIntenalnodetoleafs(FLAT::uint64 ancestorNodeID);
};

#endif	/* TOUCH_H */

