/* 
 * File:   algoPS.h
 * Author: alvis
 *
 * Created on December 14, 2014, 9:41 PM
 */

#ifndef ALGOPS_H
#define	ALGOPS_H

#include "JoinAlgorithm.h"

class algoPS : public JoinAlgorithm {
public:
    algoPS()
    {
        algorithm = algo_PS;
    }
    ~algoPS() {}
    
    void run()
    {
        totalTimeStart();
        readBinaryInput(file_dsA, file_dsB);
        PS(dsA,dsB);
        totalTimeStop();
    }
    
    void PS(SpatialObjectList& A, SpatialObjectList& B)
    {

	//Sort the datasets based on their lower x coordinate
	sorting.start();
	std::sort(A.begin(), A.end(), Comparator_Xaxis());
	std::sort(B.begin(), B.end(), Comparator_Xaxis());
	sorting.stop();

	//sweep
	FLAT::uint64 iA=0,iB=0;
	while(iA<A.size() && iB<B.size())
	{
            if(A.at(iA)->getMBR().low[0] < B.at(iB)->getMBR().low[0])
            {
                FLAT::uint64 i = iB;
                FLAT::spaceUnit border = A.at(iA)->getMBR().high[0]+epsilon;
                while(i<B.size() && B.at(i)->getMBR().low[0] <= border)
                {
                    if (istouching(B.at(i) , A.at(iA)))
                        resultPairs.addPair( B.at(i) , A.at(iA) );
                    i++;
                }
                iA++;
            }
            else
            {
                FLAT::uint64 i = iA;
                FLAT::spaceUnit border = B.at(iB)->getMBR().high[0]+epsilon;
                while(i<A.size() &&  A.at(i)->getMBR().low[0] <= border)
                {
                    if ( istouching(B.at(iB) , A.at(i)) )
                    {
                        resultPairs.addPair( B.at(i) , A.at(iA) );
                    }
                    i++;
                }
                iB++;
            }
	}
        //resultPairs.deDuplicateTime.start();
        resultPairs.deDuplicate();
        //resultPairs.deDuplicateTime.stop();
    }
};

#endif	/* ALGOPS_H */
