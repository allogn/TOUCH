/* 
 * File:   TOUCH.h
 *
 * Original TOUCH algorithm
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
private:
    void joinNodeToDesc(FLAT::uint64 ancestorNodeID);
    void assignment();
};

#endif	/* TOUCH_H */

