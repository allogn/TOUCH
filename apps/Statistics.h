/* 
 * File:   Statistics.h
 * Author: Alvis
 *
 * Created on 30 октября 2014 г., 14:22
 */

#ifndef STATISTICS_H
#define	STATISTICS_H

#include "Timer.hpp"

using namespace FLAT;

class Statistics
{
public:

	uint64 ItemsCompared;
	uint64 duplicates;
	uint64 gridSize;
	uint64 results;

	uint64 filtered;
	uint64 hashprobe;
	double repA;
	double repB;

	uint64 footprint;
	uint64 max;
	double percentageEmpty;
	double avg;
	double std;
	Timer sorting;

	Timer building;
	Timer probing;
	Timer deDuplicate;
	Timer analyzing;
	Timer initialize;
	Timer total;
	Timer comparing;
	Timer partition;
	Timer test;	//For testing purpose only

	Statistics()
	{
		hashprobe = 0;
		footprint=0;
		filtered=0;
		max=0;
		avg=0;
		std=0;
		ItemsCompared =0;
		duplicates =0;
		gridSize = 0;
		results=0;
		percentageEmpty=0;
		repA=1;
		repB=1;
	}
	~Statistics()
	{

	}
	void print()
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
		cout << "Max objects mapped to a grid cell: " << max <<endl;
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
		<< "size" << " SOlist "<< sizeof(SpatialObjectList) << " SO* "<< sizeof(SpatialObject *) << " Node* "<< sizeof(TreeNode*) << '\n'
		<< "EmptyCells(%) " << percentageEmpty	<< " MaxObj " << max << " AveObj " << avg << " StdObj " << std << '\n'

		<< "Compared # " << ItemsCompared << " % " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB) << '\n'
		<< "Duplicates " << duplicates << " Selectivity " << 100.0*(double)results/(double)(size_dsA*size_dsB) << '\n'
		<< "Results " << results << '\n'
		<< "filtered " << filtered	<< " repA " << repA	<< " repB " << repB << '\n'

		<< "Times: total " << total << '\n'
		<< " loading " << dataLoad << " init " << initialize	<< " build " << building << " probe " << probing << '\n'
		<< " comparing " << comparing << " partition " << partition	<< " join " << Ljoin	<< '\n'
		<< " deDuplicating " << deDuplicate	<< " analyzing " << analyzing << " sorting " << sorting << '\n'
		<< "Partitions " << partitions << " epsilon " << epsilon << " Fanout " << base << '\n'
		<< "\n================================\n"
		<< "\ndatasets\n" << file_dsA << '\n' << file_dsB << '\n';


cout<<"Logging to file ";
		ofstream fout(logfilename.c_str(),ios_base::app);
		fout << algoname << " using " << basealgo << " gridSize " << gridSize
		<< " sortType " <<	PartitioningType
		<< " epsilon " << epsilon
		<< " memFP(MB) " << (footprint+0.0)/(1024.0*1024.0)
		<< " #A " << size_dsA
		<< " #B " << size_dsB
		<< " sizeObj(B) " << sizeof(SpatialObjectList)
		<< " EmptyCells(%) " << percentageEmpty
		<< " MaxObj " << max
		<< " AveObj " << avg
		<< " StdObj " << std

		<< " Compared # " << ItemsCompared
		<< " % " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB)
		<< " Duplicates " << duplicates
		<< " Results " << results
		<< " Selectivity " << 100.0*(double)results/(double)(size_dsA*size_dsB)
		<< " filtered " << filtered
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
		<< " deDuplicating " << deDuplicate
		<< " analyzing " << analyzing
		<< " sorting " << sorting

		<< " partitions " << partitions
		<< " Fanout " << base
		<< " datasets " << file_dsA << " " << file_dsB;


/*
 * Debugging and Testing purpose
 * For measuring the effectivness of the hierarchical approach
*/
if(algorithm == algo_TOUCH)
{
		uint64 leafsize = ceil((double)size_dsA / (double)partitions);
		uint64 comparisons = 0;
		for(int i = 0 ; i<LVL ; i++)
			comparisons += pow(base,i)*(leafsize*ItemPerLevel[i]);
		//ofstream lg; lg.open("tree.txt",ofstream::app);
		fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
		100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)" <<
		" Filtered " <<	filtered << "(#) = " << 100*(double)filtered/(double)size_dsB << "(%) Level";

		for(int i = 0 ; i<LVL ; i++)
			fout<< " " << i << " : " << ItemPerLevel[i] << "(#) = " <<  100*(double)ItemPerLevel[i]/(double)size_dsB << "(%)";
//		lg<<endl;
//		lg.close();
}

if(algorithm == algo_cTOUCH)
{
		uint64 leafsize = ceil((double)size_dsA / (double)partitions);
		uint64 comparisons = 0;
		for(int i = 0 ; i<LVL ; i++)
			comparisons += pow(base,i)*(leafsize*ItemPerLevel[i]);
		//ofstream lg; lg.open("tree.txt",ofstream::app);
		fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
		100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)" <<
		" Filtered " <<	filtered << "(#) = " << 100*(double)filtered/(double)size_dsB << "(%) Level";

		for(int i = 0 ; i<LVL ; i++)
			fout<< " " << i << " : " << ItemPerLevel[i] << "(#) = " <<  100*(double)ItemPerLevel[i]/(double)size_dsB << "(%)";
//		lg<<endl;
//		lg.close();
}

if(algorithm == algo_dTOUCH)
{
		uint64 leafsize = ceil((double)size_dsA / (double)partitions);
		uint64 comparisons = 0;
		for(int i = 0 ; i<LVL ; i++)
			comparisons += pow(base,i)*(leafsize*ItemPerLevel[i]);
		//ofstream lg; lg.open("tree.txt",ofstream::app);
		fout << " LocalJoinResolution " << localPartitions <<  " ExpectedComparisons " << comparisons << "(#) = " <<
		100*((double)comparisons/(double)(size_dsA*size_dsB)) << "(%)" <<
		" Filtered " <<	filtered << "(#) = " << 100*(double)filtered/(double)size_dsB << "(%) Level";

		for(int i = 0 ; i<LVL ; i++)
			fout<< " " << i << " : " << ItemPerLevel[i] << "(#) = " <<  100*(double)ItemPerLevel[i]/(double)size_dsB << "(%)";
//		lg<<endl;
//		lg.close();
}
                
/*
 * End
 */


fout<< endl;
fout.close();



cout<<"Done."<<endl;

	}
};


#endif	/* STATISTICS_H */

