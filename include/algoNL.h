/* 
 * File:   algoNL.h
 * Author: alvis
 *
 * Created on December 14, 2014, 8:36 PM
 */

#ifndef ALGONL_H
#define	ALGONL_H

#include "JoinAlgorithm.h"

class algoNL : public JoinAlgorithm {
public:
    algoNL()
    {
        algorithm = algo_NL;
    }
    ~algoNL() {}
    
    void run()
    {
        totalTimeStart();
        readBinaryInput(file_dsA, file_dsB);
        NL(dsA,dsB);
        totalTimeStop();
    }
};

#endif	/* ALGONL_H */

