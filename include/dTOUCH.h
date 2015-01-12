/* 
 * File:   dTOUCH.h
 * Author: Alvis Logins
 *
 * dTOUCH runs TOUCH with probability to assign objects to high levels
 * Skipped objects are used for second iteration of TOUCH with dataset roles swapped
 */

#ifndef DTOUCH_H
#define	DTOUCH_H

#include "CommonTOUCH.h"
#include "LocalSpatialGridHash.h"

class dTOUCH : public CommonTOUCH
{

private:
    
    void assignment(SpatialObjectList& ds);
public:

    dTOUCH();
    ~dTOUCH();
    
    void run();
    //void joinNodeToDesc(TreeNode* ancestorNode);
};


#endif	/* DTOUCH_H */

