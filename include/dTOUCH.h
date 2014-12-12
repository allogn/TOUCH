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
#include "SpatialGridHash.h"

class dTOUCH : public CommonTOUCH
{

private:
    
    void assignment(SpatialObjectList& ds);
    void joinObjectToDesc(FLAT::SpatialObject* obj, FLAT::uint64 ancestorNodeID);

public:

    dTOUCH();
    ~dTOUCH();
    
    void analyze();
    void run();
    
};


#endif	/* DTOUCH_H */

