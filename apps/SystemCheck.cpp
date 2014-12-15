/* 
 * File:   SystemCheck.cpp
 * Author: alvis
 *
 * Run all algorithms with random data
 */

#include "algoNL.h"
#include "algoPS.h"
#include "S3Hash.h"
#include "TOUCH.h"
#include "dTOUCH.h"
#include "cTOUCH.h"
#include "reTOUCH.h"
#include "rereTOUCH.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

std::string input_dsA = "../data/RandomData-100K.bin";
std::string input_dsB = "../data/RandomData-1600K.bin";

int PartitioningTypeMain                = Hilbert_Sort;     // Sorting algorithm
int localPartitions                     = 100;              //The local join resolution
bool verbose				=  false;           // Output everything or not?
int algorithm				=  algo_NL;         // Choose the algorithm
int localJoin				=  algo_SGrid;         // Choose the algorithm for joining the buckets, The local join
double epsilon				=  10;             // the epsilon of the similarity join
int leafsize				=  100;             // # of partitions: in S3 is # of levels; in SGrid is resolution. Leafnode size.
unsigned int numA = 2000 ,numB = 2000;                            //number of elements to be read from datasets
int nodesize                            = 2;                // number of children per node if not leaf
int maxLevelCoef                        = 500;              // coefficient in probability to assign object to first tree in dTOUCH

bool testRun()
{
    bool isError = false;
    vector<JoinAlgorithm*> allAlg(8);
    allAlg[0] = new algoNL();
    allAlg[1] = new TOUCH();
    allAlg[2] = new dTOUCH();
    allAlg[3] = new cTOUCH();
    allAlg[4] = new reTOUCH();
    allAlg[5] = new rereTOUCH();
    allAlg[6] = new algoPS();
    allAlg[7] = new S3Hash();
    
    for (int i = 0; i < allAlg.size(); i++)
    {
        allAlg[i]->PartitioningType = PartitioningTypeMain;
        allAlg[i]->nodesize         = nodesize;
        allAlg[i]->leafsize         = leafsize;
        allAlg[i]->localPartitions  = localPartitions;	
        allAlg[i]->verbose          = verbose;		
        allAlg[i]->localJoin        = localJoin;	
        allAlg[i]->epsilon          = epsilon;	
        allAlg[i]->numA             = numA;
        allAlg[i]->numB             = numB;
        allAlg[i]->maxLevelCoef     = maxLevelCoef;
        allAlg[i]->file_dsA         = input_dsA;
        allAlg[i]->file_dsB         = input_dsB;
        
        allAlg[i]->run();
    }

    int results = allAlg[0]->resultPairs.results;
    cout << "NL result: " << results << "; Check: ";
    for (int i = 1; i < allAlg.size(); i++)
    {
        cout << i << " - ";
        if (allAlg[i]->resultPairs.results != results)
        {
            cout << " ERROR! New value: " << allAlg[i]->resultPairs.results << ". ";
            cout << "Line for retrieving error:\n";
            cout << "./SpatialJoin -a " << allAlg[i]->algorithm << " -J 2 -n " << numA << " " << numB << " -e " << epsilon << "\n";
            //NOTE: default values must be the same here and in SpatialJoin
            isError = true;
        }
        else
        {
            cout << "OK. ";
        }
    }
    cout << endl;
    return isError;
}

void checkEqualResults()
{
    bool isError = false;
    
    std::cout << "Creating vector of algorithms" << endl;
    
    cout << "Variating epsilon: " << endl;
    for (int eps = 0; eps < 3; eps++)
    {
        epsilon = 0.5 + eps*15;
        cout << "Epsilon = " << epsilon << endl;
        if (testRun())
            isError = true;
    }
    epsilon = 15;
    
    cout << "Variating number of objects: " << endl;
    for (int num = 0; num < 3; num++)
    {
        numA = 500 + num*1000;
        numB = 500 + num*1000;
        cout << "Num = " << numA << endl;
        if (testRun())
            isError = true;
    }

    if (isError)
    {
        cout << "== Error! Something is wrong. ==" << endl;
    }
    else
    {
        cout << "-- Equal result check done. No mistakes. --" << endl;
    }
}

int main(int argc, char** argv) {
    
    checkEqualResults();
    
    return 0;
}

