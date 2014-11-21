/* 
 * File:   JoinAlgorithm.cpp
 * Author: Alvis
 * 
 * Created on 30.10.2014
 */

#include "JoinAlgorithm.h"

JoinAlgorithm::JoinAlgorithm() {
    hashprobe = 0;
    footprint=0;
    filtered[0]=0;
    filtered[1] = 0;
    maxMappedObjects=0;
    avg=0;
    std=0;
    ItemsCompared =0;
    gridSize = 0;
    percentageEmpty=0;
    repA=1;
    repB=1;
    algorithm				=  algo_NL;	//default algorithm
    localJoin				=  algo_NL;
    partitions				=  4;	
    
    verbose				=  false;
    base = 2; // the base for S3 and SH algorithms
    logfilename = "SJ.LOG"; //@todo add to parameters
    epsilon = 1.5;
    numA = 0, numB = 0;
}

JoinAlgorithm::~JoinAlgorithm() {
}

void JoinAlgorithm::readBinaryInput(string in_dsA, string in_dsB) {
    
    cout << "Start reading the datasets" << endl;

    file_dsA = in_dsA;
    file_dsB = in_dsB;

    FLAT::DataFileReader *inputA = new FLAT::DataFileReader(file_dsA);
    FLAT::DataFileReader *inputB = new FLAT::DataFileReader(file_dsB);

    TreeEntry* newEntry;

    dataLoad.start();

    size_dsA = (numA < inputA->objectCount && (numA != 0))?numA:inputA->objectCount;
    size_dsB = (numB < inputB->objectCount && (numB != 0))?numB:inputB->objectCount;
    cout << "size of A:" << size_dsA << "# from " << inputA->objectCount << "# " 
                    << size_dsA*sizeof(SpatialObjectList) / 1000.0 << "KB" << endl;
    cout << "size of B:" << size_dsB << "# from " << inputB->objectCount << "# " 
                    << size_dsB*sizeof(SpatialObjectList) / 1000.0 << "KB" << endl;
    
    FLAT::Box mbr;
    for (int i=0;i<DIMENSION;i++)
    {
            universeA.low.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::max();
            universeA.high.Vector[i]  = std::numeric_limits<FLAT::spaceUnit>::min();
            universeB.low.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::max();
            universeB.high.Vector[i]  = std::numeric_limits<FLAT::spaceUnit>::min();
    }
    FLAT::SpatialObject* sobj;

    dsA.reserve(size_dsA);
    numA = size_dsA;
    while(inputA->hasNext() && (numA-- != 0))
    {
            sobj = inputA->getNext();
            mbr = sobj->getMBR();
            mbr.isEmpty = false;
            for (int i=0;i<DIMENSION;i++)
            {
                    universeA.low.Vector[i] = min(universeA.low.Vector[i],mbr.low.Vector[i]);
                    universeA.high.Vector[i] = max(universeA.high.Vector[i],mbr.high.Vector[i]);
            }
            sobj->type = 0;
            sobj->cost = 0;

            newEntry = new TreeEntry(sobj);
            newEntry->expand(epsilon); //@todo in new touch when assigning - also must expand??
            vdsA.push_back(newEntry);
            dsA.push_back(sobj);
            vdsAll.push_back(newEntry);
            if  (verbose)
                cout << "A" << size_dsA - numA << ":" << mbr.low << " " << mbr.high << endl;
    }

    dsB.reserve(size_dsB);
    numB = size_dsB;
    while (inputB->hasNext() && (numB-- != 0))
    {
            sobj = inputB->getNext();
            mbr = sobj->getMBR();
            mbr.isEmpty = false;
            for (int i=0;i<DIMENSION;i++)
            {
                    universeB.low.Vector[i] = min(universeB.low.Vector[i],mbr.low.Vector[i]);
                    universeB.high.Vector[i] = max(universeB.high.Vector[i],mbr.high.Vector[i]);
            }
            sobj->type = 1;
            sobj->cost = 0;

            newEntry = new TreeEntry(sobj);
            newEntry->expand(epsilon); //@todo in new touch when assigning - also must expand??
            //vdsB.push_back(newEntry); @todo is it needed elsewhere? in dTOUCH it must be empty at the beginning
            dsB.push_back(sobj);
            vdsAll.push_back(newEntry);
            if(verbose)
                cout << "B" << size_dsB - numB << ":" << mbr.low << " " << mbr.high << endl;
    }

    dataLoad.stop();

    cout << "Reading Completed." << endl;

}

void JoinAlgorithm::print()
{
        string algoname="";
        switch(algorithm)
        {
                case algo_NL:
                        algoname = "NL";
                break;
                case algo_PS:
                        algoname = "PS";
                break;
                case algo_TOUCH:
                        algoname = "TOUCH";
                break;
                case algo_cTOUCH:
                        algoname = "cTOUCH";
                break;
                case algo_reTOUCH:
                        algoname = "reTOUCH";
                break;
                case algo_dTOUCH:
                        algoname = "dTOUCH";
                break;
                case algo_SGrid:
                        algoname = "SGrid";
                break;
                case algo_S3:
                        algoname = "S3";
                break;
                case algo_PBSM:
                        algoname = "PBSM";
                break;
        }
        string basealgo="";
        switch(localJoin)
        {
                case algo_NL:
                        basealgo = "NL";
                break;
                case algo_PS:
                        basealgo = "PS";
                break;
                case algo_TOUCH:
                        basealgo = "TOUCH";
                break;
                case algo_cTOUCH:
                        basealgo = "cTOUCH";
                break;
                case algo_dTOUCH:
                        basealgo = "dTOUCH";
                break;
                case algo_reTOUCH:
                        basealgo = "reTOUCH";
                break;
                case algo_SGrid:
                        basealgo = "SGrid";
                break;
                case algo_S3:
                        basealgo = "S3";
                break;
                case algo_PBSM:
                        basealgo = "PBSM";
                break;
        }
/*
        cout << "\nAlgorithm " << algoname << " Base " << basealgo << "\nTotal Cells in Grid: " << gridSize <<endl;
        cout << "Total memory Footprint in MB: " << (footprint+0.0)/(1024.0*1024.0) <<endl;
        cout << "Percentage Cells Empty: " << percentageEmpty << endl;
        cout << "Max objects mapped to a grid cell: " << maxMappedObjects <<endl;
        cout << "Average objects mapped to grid cells: " << avg <<endl;
        cout << "Std Deviation of objects mapped to grid cell: " << std <<endl;
        cout << "Hash Prob: " << hashprobe << endl;

        cout << "Total Items Compared: " << ItemsCompared << "# " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB) << "%" <<endl;
        cout << "Total Duplciates Found: " << duplicates <<endl;
        cout << "Total Unique Results: " << results <<endl;
        cout << "Filtered repA repB: " << filtered << " " << repA << "X " << repB << "X"<< endl;

        cout << "Time taken for loading Data: " << dataLoad << endl;
        cout << "Time taken for sorting Data: " << sorting << endl;
        cout << "Time taken for comparing Data: " << comparing << endl;
        cout << "Time taken to initialize Hash Table: " << initialize << endl;
        cout << "Time taken for building Hash Table: " << building << endl;
        cout << "Time taken for probing Hash Table: " << probing << endl;
        cout << "Time taken for deDuplicating results: " << deDuplicate << endl;
        cout << "Time taken for analyzing these metrics: " << analyzing << endl;
        cout << "Total Time taken: " << total << endl;
        */
        cout<< "\n================================\n";
        cout << algoname << " using " << basealgo << " gridSize " << gridSize << '\n'
        << "memFP(MB) " << (footprint+0.0)/(1024.0*1024.0) << " #A " << size_dsA << " #B " << size_dsB << '\n'
        << "size" << " SOlist "<< sizeof(SpatialObjectList) << " SO* "<< sizeof(FLAT::SpatialObject *) << " Node* "<< sizeof(TreeNode*) << '\n'
        << "EmptyCells(%) " << percentageEmpty	<< " MaxObj " << maxMappedObjects << " AveObj " << avg << " StdObj " << std << '\n'

        << "Compared # " << ItemsCompared << " % " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB) << '\n'
        << "Duplicates " << resultPairs.duplicates << " Selectivity " << 100.0*(double)resultPairs.results/(double)(size_dsA*size_dsB) << '\n'
        << "Results " << resultPairs.results << '\n'
        << "filtered A " << filtered[0]	<< " B " << filtered[1] << " repA " << repA	<< " repB " << repB << '\n'

        << "Times: total " << total << '\n'
        << " loading " << dataLoad << " init " << initialize	<< " build " << building << " probe " << probing << '\n'
        << " comparing " << comparing << " partition " << partition	<< " join " << Ljoin	<< '\n'
        << " deDuplicating " << resultPairs.deDuplicateTime	<< " analyzing " << analyzing << " sorting " << sorting << '\n'
        << "Partitions " << partitions << " epsilon " << epsilon << " Fanout " << base << '\n'
        << "\n================================\n"
        << "\ndatasets\n" << file_dsA << '\n' << file_dsB << '\n';


        cout<<"Logging to file ";
        ofstream fout(logfilename.c_str(),ios_base::app);
        fout << algoname << " using " << basealgo << " gridSize " << gridSize
        << " epsilon " << epsilon
        << " memFP(MB) " << (footprint+0.0)/(1024.0*1024.0)
        << " #A " << size_dsA
        << " #B " << size_dsB
        << " sizeObj(B) " << sizeof(SpatialObjectList)
        << " EmptyCells(%) " << percentageEmpty
        << " MaxObj " << maxMappedObjects
        << " AveObj " << avg
        << " StdObj " << std

        << " Compared # " << ItemsCompared
        << " % " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB)
        << " Duplicates " << resultPairs.duplicates
        << " Results " << resultPairs.results
        << " Selectivity " << 100.0*(double)resultPairs.results/(double)(size_dsA*size_dsB)
        << " filtered A:" << filtered[0] << " B " << filtered[1]
        << " repA " << repA
        << " repB " << repB

        << " Time "
        << " loading " << dataLoad
        << " init " << initialize
        << " build " << building
        << " probe " << probing
        << " comparing " << comparing
        << " partition " << partition
        << " join " << Ljoin
        << " total " << total
        << " deDuplicating " << resultPairs.deDuplicateTime
        << " analyzing " << analyzing
        << " sorting " << sorting

        << " partitions " << partitions
        << " Fanout " << base
        << " datasets " << file_dsA << " " << file_dsB;


    fout<< endl;
    fout.close();



    cout<<"Done."<<endl;

}

void JoinAlgorithm::PS(SpatialObjectList& A, SpatialObjectList& B)
{

	//Sort the datasets based on their lower x coordinate
	sorting.start();
	std::sort(A.begin(), A.end(), Comparator_Xaxis());
	//cout<<"Sort A Done."<<endl;
	std::sort(B.begin(), B.end(), Comparator_Xaxis());
	//cout<<"Sort B Done."<<endl;
	sorting.stop();

	//sweep
	FLAT::uint64 iA=0,iB=0;
	while(iA<A.size() && iB<B.size())
	{
		//cout<<iA<< " " <<iB<<endl;
		if(A.at(iA)->getMBR().low[0] < B.at(iB)->getMBR().low[0])
		{
			FLAT::uint64 i = iB;
			FLAT::spaceUnit border = A.at(iA)->getMBR().high[0]+epsilon;
			while(i<B.size() && B.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(i) , A.at(iA)) )
				{
                                    resultPairs.addPair(B.at(i),A.at(iA));
				}

				i++;
			}
			iA++;
		}
		else
		{
			FLAT::uint64 i = iA;
			FLAT::spaceUnit border = B.at(iB)->getMBR().high[0]+epsilon;
			while(i<A.size() &&  A.at(i)->getMBR().low[0] < border)
			{
				if ( istouching(B.at(iB) , A.at(i)) )
				{
                                    resultPairs.addPair(B.at(i),A.at(iA));
				}

				i++;
			}
			iB++;
		}
	}
}


void JoinAlgorithm::JOIN(SpatialObjectList& A, SpatialObjectList& B)
{
        Ljoin.start();
        if(localJoin == algo_NL)
            NL(A,B);
        else
            PS(A,B);
        Ljoin.stop();
}