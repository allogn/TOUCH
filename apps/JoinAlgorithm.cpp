/* 
 * File:   JoinAlgorithm.cpp
 * Author: Alvis
 * 
 * Created on 30.10.2014
 */

#include "JoinAlgorithm.h"

JoinAlgorithm::JoinAlgorithm() {
}

JoinAlgorithm::JoinAlgorithm(const JoinAlgorithm& orig) {
}

JoinAlgorithm::~JoinAlgorithm() {
}

void JoinAlgorithm::readBinaryInput(string file_dsA, string file_dsB) {

	cout << "Start reading the datasets" << endl;
	///////////////////////// Load Data //////////////////////////////

	inputA = new DataFileReader(file_dsA);
	inputB = new DataFileReader(file_dsB);

	//////////////////////// Error Checking //////////////////////////
	//if (inputA->objectType!=SEGMENT) {cout << "Only works for Segment type objects"; exit(0);}
	//if (inputB->objectType!=SEGMENT) {cout << "Only works for Segment type objects"; exit(0);}
	//////////////////////////////////////////////////////////////////

	dataLoad.start();

	if(numA > 0 && numB > 0)
	{
		size_dsA = numA;
		size_dsB = numB;
		cout << "size of A:" << size_dsA << "# from " << inputA->objectCount << "# " << size_dsA*sizeof(SpatialObjectList) / 1000.0 << "KB" << endl;
		cout << "size of B:" << size_dsB << "# from " << inputB->objectCount << "# " << size_dsB*sizeof(SpatialObjectList) / 1000.0 << "KB" << endl;
		Box mbr;
		for (int i=0;i<DIMENSION;i++)
		{
			universeA.low.Vector[i] = numeric_limits<spaceUnit>::max();
			universeA.high.Vector[i]  = numeric_limits<spaceUnit>::min();
			universeB.low.Vector[i] = numeric_limits<spaceUnit>::max();
			universeB.high.Vector[i]  = numeric_limits<spaceUnit>::min();
		}
		SpatialObject* sobj;
                
                dsA.reserve(size_dsA);
                while(inputA->hasNext() && numA-- != 0)
                {
                        sobj = inputA->getNext();
                        mbr = sobj->getMBR();
                        for (int i=0;i<DIMENSION;i++)
                        {
                                universeA.low.Vector[i] = min(universeA.low.Vector[i],mbr.low.Vector[i]);
                                universeA.high.Vector[i] = max(universeA.high.Vector[i],mbr.high.Vector[i]);
                        }
                        sobj->type = 0;
                        sobj->cost = 0;
                        vdsA.push_back(new TreeEntry(sobj));
                        dsA.push_back(sobj);
                        vdsAll.push_back(sobj);
                        if  (verbose)
                            cout << "A" << size_dsA - numA << ":" << mbr.low << " " << mbr.high << endl;
                }
                        
		dsB.reserve(size_dsB);
		while (inputB->hasNext() && numB-- != 0)
		{
			sobj = inputB->getNext();
			mbr = sobj->getMBR();
			for (int i=0;i<DIMENSION;i++)
			{
				universeB.low.Vector[i] = min(universeB.low.Vector[i],mbr.low.Vector[i]);
				universeB.high.Vector[i] = max(universeB.high.Vector[i],mbr.high.Vector[i]);
			}
			sobj->type = 1;
			sobj->cost = 0;
			dsB.push_back(sobj);
			if(verbose)
                            cout << "B" << size_dsB - numB << ":" << mbr.low << " " << mbr.high << endl;
		}


	}
	else
	{
	if(inputA->objectCount > inputB->objectCount)
		swap(inputA,inputB);

	size_dsA = inputA->objectCount;
	size_dsB = inputB->objectCount;
	cout << "size of A:" << size_dsA << "# " << size_dsA*sizeof(SpatialObjectList) / 1000.0 << "KB" << endl;
	cout << "size of B:" << size_dsB << "# " << size_dsB*sizeof(SpatialObjectList) / 1000.0 << "KB" << endl;

	if(algorithm == algo_TOUCH)
	{
		SpatialObject* sobj;
		while(inputA->hasNext())
		{
			sobj = inputA->getNext();
			sobj->type = 0;
			sobj->cost = 0;
			vdsA.push_back(new TreeEntry(sobj));
			//delete sobj;
		}
	}
	else
	{
		dsA.reserve(size_dsA);
		while (inputA->hasNext())
		{
			dsA.push_back(inputA->getNext());
			dsA.back()->type = 0;
			dsA.back()->cost = 0;
		}
	}
	dsB.reserve(size_dsB);
	while (inputB->hasNext())
		dsB.push_back(inputB->getNext());
		dsB.back()->type = 1;
		dsB.back()->cost = 0;
	}

	dataLoad.stop();

    cout << "Reading Completed." << endl;

}

