#include "TOUCH.h"

int PartitioningTypeMain = Hilbert_Sort;	// Sorting algorithm
int localPartitions = 100;					//The local join resolution
bool verbose				=  false;		// Output everything or not?
int algorithm				=  algo_NL;		// Choose the algorithm
int localJoin				=  algo_NL;		// Choose the algorithm for joining the buckets, The local join
int runs					=  1;				// # of runs
double epsilon				=  1.5;				// the epsilon of the similarity join
int partitions				=  4;				// # of partitions: in S3 is # of levels; in SGrid is resolution
unsigned int numA = 0 ,numB = 0;		//number of elements to be read from datasets
int base = 2; // the base for S3 and SH algorithms

string input_dsA = "..//data//RandomData-100K.bin";
string input_dsB = "..//data//RandomData-1600K.bin";

void usage(const char *program_name) {

    printf("   Usage: %s [-v] [-h] [-a <int>] [-i <path> <path>]\n", program_name);
    printf("   -h               Print this help menu.\n");
    printf("   -a               Algorithms\n");
    printf("      0:Nested Loop\n");
    printf("      1:Plane-Sweeping\n");
    printf("      2:Spatial Grid Hash\n");
    printf("      3:Size Separation Spatial\n");
    printf("      4:TOUCH:Spatial Hierarchical Hash\n");
    printf("      5:Partition Based Spatial-Merge Join\n");
    printf("      6:cTOUCH:Spatial Hierarchical Has\n");
    printf("      7:dTOUCH:Spatial Hierarchical Has\n");
    printf("\n");
    printf("   -J               Algorithm for joining the buckets\n");
    printf("   -p               # of partitions\n");
    printf("   -b               the number of cells to be merged in the hierarchy\n");
    printf("   -e               Epsilon of the similarity join\n");
    printf("   -r               # of runs\n");
    printf("   -i <path> <path>  Dataset A followed by B\n");
    printf("   -n #A #B  number of element to be read\n");
    printf("   -v verbose\n");
    printf("\n-a 1 -i 'c:\\data\\A.dat' 'c:\\data\\B.dat'\n");

}

//Parsing Arguments
void parse_args(int argc, const char* argv[]) {

    int x;
    if(argc<2)
    {
    	usage(argv[0]);
        exit(1);
    }
    for ( x= 1; x < argc; ++x)
    {
        switch (argv[x][1])
        {
        case 'h':
            usage(argv[0]);
            exit(1);
            break;
		case 'i':
            if (++x < argc)
            {
                input_dsA=argv[x];
                input_dsB=argv[++x];
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument, %s", argv[x-1]);
                usage(argv[0]);
            }
            break;
		case 'a':
			sscanf(argv[++x], "%u", &algorithm);
            break;
		case 'n':
			sscanf(argv[++x], "%u", &numA);
			sscanf(argv[++x], "%u", &numB);
            break;
		case 'J':
			sscanf(argv[++x], "%u", &localJoin);
			break;
		case 'r':       /* # of runs */
			sscanf(argv[++x], "%u", &runs);
            break;
		case 't':       /* Testing */
			sscanf(argv[++x], "%u", &PartitioningTypeMain);
            break;
		case 'e':       /* epsilon */
			sscanf(argv[++x], "%lf", &epsilon);
            break;
		case 'p':       /* # of partitions */
			sscanf(argv[++x], "%u", &partitions);
            break;
		case 'b':       /* base for the number of components to merge in every level of the hierarchy */
			sscanf(argv[++x], "%u", &base);
            break;
		case 'v':       /* verbose */
			verbose = true;
            break;
        default:
            fprintf(stderr, "Error: Invalid command line parameter, %c\n", argv[x][1]);
            usage(argv[0]);
        }
    }
}

//
//
//// The following functions are for creating the corresponding object of their specified join method
//
////Spatial Grid Hash Join algorithm
//void SGrid()
//{
//	Box universe;
//	//Box::combine(inputA->universe,inputB->universe,universe);
//	if(numA > 0 && numB > 0)
//	{
//		universe = universeA;
//	}
//	else
//	{
//		universe = inputA->universe;
//	}
//
//	Box::expand(universe,epsilon);
//	cout<< "Universe: " << universe.low << " " << universe.high <<endl;
//	SpatialGridHash* spatialGridHash = new SpatialGridHash(universe,partitions);
//
//	cout << "Buidling ";
//	spatialGridHash->build(dsA);
//	cout << "Done\nAnalysis " ;
//	spatialGridHash->analyze(dsA,dsB);
//	cout << "Done\nProbing ";
//	spatialGridHash->probe(dsB);
//	cout << "\nDone." << endl;
//	delete spatialGridHash;
//}
//
////Size Separation Spatial join algorithm
//void S3(ResultPairs& S3Results)
//{
//	Box universe;
//	if(numA > 0 && numB > 0)
//	{
//		Box::combine(universeA,universeB,universe);
//	}
//	else
//	{
//		Box::combine(inputA->universe,inputB->universe,universe);
//	}
//
//	Box::expand(universe,epsilon);
//	cout<< "Universe: " << universe.low << " " << universe.high <<endl;
//	Box::combine(inputA->universe,inputB->universe,universe);
//	S3Hash* s3Hash = new S3Hash(universe,partitions);
//
//	cout << "Building Started" << endl;
//	s3Hash->build(dsA, dsB);
//	cout << "Building Done" << endl;
//	s3Hash->analyze(dsA,dsB);
//	cout << "Analysis Done" << endl;
//	s3Hash->probe(S3Results);
//
//	cout << "Probing Done" << endl;
//	delete s3Hash;
//}
//
////Partition Based Spatial-Merge join algorithm
//void PBSM()
//{
//	ResultList PBSMResults;
//	Box universe;
//	if(numA > 0 && numB > 0)
//	{
//		universe = universeA;
//	}
//	else
//	{
//		universe = inputA->universe;
//	}
//
//	Box::expand(universe,epsilon);
//	cout<< "Universe: " << universe.low << " " << universe.high <<endl;
//	PBSMHash* pbsmHash = new PBSMHash(universe,partitions);
//
//	cout << "Building Started" << endl;
//	pbsmHash->build(dsA, dsB);
//	cout << "Building Done" << endl;
//	pbsmHash->analyze(dsA,dsB);
//	cout << "Analysis Done" << endl;
//	pbsmHash->probe(PBSMResults);
//	cout << "Probing Done" << endl;
//	delete pbsmHash;
//}
//
//void dTOUCH()
//{
//
//	TOUCH* touch = new cTOUCH(partitions);
//        cout << "Reading files" << endl;
//        
//	cout << "Forming the partitions" << endl;
//	touch->createPartitions();
//	cout << "Assigning the objects of B" << endl;
//	touch->assignment();
//	cout << "Assigning Done." << endl;
//	touch->analyze(dsA,dsB);
//	cout << "Analysis Done" << endl;
//	cout << "Probing, doing the join" << endl;
//	touch->probe();
//	cout << "Done." << endl;
//}
//
//void cTOUCH()
//{
//
//	TOUCH* touch = new cTOUCH(partitions);
//        cout << "Reading files" << endl;
//        
//	cout << "Forming the partitions" << endl;
//	touch->createPartitions();
//	cout << "Assigning the objects of B" << endl;
//	touch->assignment();
//	cout << "Assigning Done." << endl;
//	touch->analyze(dsA,dsB);
//	cout << "Analysis Done" << endl;
//	cout << "Probing, doing the join" << endl;
//	touch->probe();
//	cout << "Done." << endl;
//}
//
void doTOUCH()
{
	TOUCH* touch = new TOUCH(partitions);
        
        touch->PartitioningType = PartitioningTypeMain;
        touch->base = base;
        touch->localPartitions = localPartitions;	
        touch->verbose  =  verbose;		
        touch->algorithm    =  algorithm;	
        touch->localJoin    =  localJoin;	
        touch->epsilon	=  epsilon;	
        touch->partitions   = partitions;
        
        
        touch->readBinaryInput(input_dsA, input_dsB);
	cout << "Forming the partitions" << endl;
	touch->createPartitions();
	cout << "Assigning the objects of B" << endl;
	touch->assignment();
	cout << "Assigning Done." << endl;
	touch->analyze();
	cout << "Analysis Done" << endl;
	cout << "Probing, doing the join" << endl;
	touch->probe();
	cout << "Done." << endl;
}

void NLalgo()
{
    cout << "New NL join algorithm created" << endl;
    JoinAlgorithm* nl = new JoinAlgorithm();
    cout << "Reading data" << endl;
    nl->readBinaryInput(input_dsA, input_dsB);
    cout << "Nested loop join" << endl;
    nl->NL(nl->dsA, nl->dsB);
    
}

int main(int argc, const char* argv[])
{
	//Parsing the arguments
	parse_args(argc, argv);
	
	switch(algorithm)
	{
		case algo_NL:
                    NLalgo();
		break;
		case algo_PS:
			//@todo
		break;
		case algo_TOUCH:
			doTOUCH();
		break;
//		case algo_cTOUCH:
//			cTOUCH();
//		break;
//		case algo_dTOUCH:
//			dTOUCH();
//		break;
//		case algo_SGrid:
//			SGrid();
//		break;
//		case algo_S3:
//			S3();
//		break;
//		case algo_PBSM:
//			PBSM();
//		break;
		default:
			cout<<"No such an algorithm!"<<endl;
			exit(0);
		break;
	}

	cout<<"Terminated."<<endl;
	return 0;
}
