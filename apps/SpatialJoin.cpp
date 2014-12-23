/*
 *  File: SpatialJoin.cpp
 *  Authors: Sadegh Nobari, Alvis Logins
 * 
 *  Implementation of Spatial join algorithms
 * 
 *  Structure:
 *  - SpatialJoin main file parses input arguments
 *  - Every spatial join algorithm corresponds to one class in scr/include dir
 *  - Class hierarchy is introduced:
 *      <Join algorithm>.h 
 *              -> CommonTOUCH.h (if derivative from TOUCH) 
 *                      -> JoinAlgorithm.h
 * 
 */

#include "algoPS.h"
#include "algoNL.h"
#include "S3Hash.h"
#include "TOUCH.h"
#include "dTOUCH.h"
#include "cTOUCH.h"
#include "reTOUCH.h"
#include "rereTOUCH.h"

//#include "test.h" //test CUDA

/*
 * Input parameters
 */
int PartitioningTypeMain                = Hilbert_Sort;     // Sorting algorithm
int localPartitions                     = 100;              //The local join resolution
bool verbose				=  false;           // Output everything or not?
int algorithm				=  algo_NL;         // Choose the algorithm
int localJoin				=  algo_NL;         // Choose the algorithm for joining the buckets, The local join
int runs				=  1;               // # of runs //@todo unsupported
double epsilon				=  0.5;             // the epsilon of the similarity join
int leafsize				=  100;             // # of partitions: in S3 is # of levels; in SGrid is resolution. Leafnode size.
unsigned int numA = 0 ,numB = 0;                            //number of elements to be read from datasets
int nodesize                            = 2;                // number of children per node if not leaf
int maxLevelCoef                        = 1;                // coefficient in probability to assign object to first tree in dTOUCH
int traversalType                       = join_UD;          // join traversal of a tree

std::string input_dsA = "../data/RandomData-100K.bin";
std::string input_dsB = "../data/RandomData-1600K.bin";

void usage(const char *program_name) {

    printf("   Usage: %s\n", program_name);
    printf("   -h               Print this help menu.\n");
    printf("   -a               Algorithms\n");
    printf("      0:Nested Loop\n");
    printf("      1:Plane-Sweeping\n");
    printf("      2:Spatial Grid Hash\n");
    printf("      3:Size Separation Spatial\n");
    printf("      4:TOUCH:Spatial Hierarchical Hash\n");
    printf("      5:Partition Based Spatial-Merge Join\n");
    printf("      6:cTOUCH:Spatial Hierarchical Hash\n");
    printf("      7:dTOUCH:Spatial Hierarchical Hash\n");
    printf("      8:reTOUCH:Spatial Hierarchical Hash\n");
    printf("      9:rereTOUCH:Spatial Hierarchical Hash\n");
    printf("\n");
    printf("   -J               Algorithm for joining the buckets\n");
    printf("   -l               leaf size\n");
    printf("   -b               fanout\n");
    printf("   -g               number of SGH cells per dimension\n");
    printf("   -m               maximum level parameter for dTOUCH\n");
    printf("   -e               Epsilon of the similarity join\n");
    printf("   -i               <path> <path>  Dataset A followed by B\n");
    printf("   -n               #A #B  number of element to be read\n");
    printf("   -t               type of tree traversal (UD 1, BU 0)\n");
    printf("   -v               verbose\n");

}

//Parsing Arguments
void parse_args(int argc, const char* argv[]) {

    int x;
    int t;
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
                                /* Local join algorithm */
			sscanf(argv[++x], "%u", &localJoin);
			break;
		case 'r':       /* # of runs */
			sscanf(argv[++x], "%u", &runs);
            break;
		case 't':       /* Partition type */
			sscanf(argv[++x], "%u", &PartitioningTypeMain);
            break;
		case 'e':       /* epsilon */
			sscanf(argv[++x], "%lf", &epsilon);
            break;
		case 'l':       /* number of objects in a leaf */
			sscanf(argv[++x], "%u", &leafsize);
            break;
		case 'b':       /* number of children for a node */
			sscanf(argv[++x], "%u", &nodesize);
            break;
		case 'm':       /* maximum level in dTOUCH parameter */
			sscanf(argv[++x], "%u", &maxLevelCoef);
            break;
		case 't':       /* maximum level in dTOUCH parameter */
			sscanf(argv[++x], "%u", &traversalType);
            break;
		case 'g':       /* base for the number of components to merge in every level of the hierarchy */
			sscanf(argv[++x], "%u", &localPartitions);
            break;
		case 'v':       /* verbose */
                        t = 1;
			sscanf(argv[++x], "%u", &t);
                        verbose = (t == 1) ? true : false;
            break;
        default:
            fprintf(stderr, "Error: Invalid command line parameter, %c\n", argv[x][1]);
            usage(argv[0]);
        }
    }
}

void dTOUCHrun()
{
    dTOUCH* touch = new dTOUCH();

    touch->PartitioningType = PartitioningTypeMain;
    touch->nodesize         = nodesize;
    touch->leafsize         = leafsize;
    touch->localPartitions  = localPartitions;	
    touch->verbose          = verbose;		
    touch->localJoin        = localJoin;	
    touch->epsilon          = epsilon;	
    touch->numA             = numA;
    touch->numB             = numB;
    touch->treeTraversal    = traversalType;
    touch->maxLevelCoef     = maxLevelCoef;
    touch->file_dsA         = input_dsA;
    touch->file_dsB         = input_dsB;
    
    touch->run();
    touch->saveLog();
    touch->print();
}

void cTOUCHrun()
{
    cTOUCH* touch = new cTOUCH();

    touch->PartitioningType = PartitioningTypeMain;
    touch->nodesize         = nodesize;
    touch->leafsize         = leafsize;
    touch->localPartitions  = localPartitions;	
    touch->verbose          = verbose;		
    touch->localJoin        = localJoin;	
    touch->epsilon          = epsilon;	
    touch->numA             = numA;
    touch->numB             = numB;
    touch->treeTraversal    = traversalType;
    touch->maxLevelCoef     = maxLevelCoef;
    touch->file_dsA         = input_dsA;
    touch->file_dsB         = input_dsB;

    touch->run();
    touch->saveLog();
    touch->print();
}

void TOUCHrun()
{
    TOUCH* touch = new TOUCH();

    touch->PartitioningType = PartitioningTypeMain;
    touch->nodesize         = nodesize;
    touch->leafsize         = leafsize;
    touch->localPartitions  = localPartitions;	
    touch->verbose          = verbose;		
    touch->localJoin        = localJoin;	
    touch->epsilon          = epsilon;	
    touch->numA             = numA;
    touch->numB             = numB;
    touch->treeTraversal    = traversalType;
    touch->maxLevelCoef     = maxLevelCoef;
    touch->file_dsA         = input_dsA;
    touch->file_dsB         = input_dsB;

    touch->run();
    touch->saveLog();
    touch->print();
}

void reTOUCHrun()
{
    reTOUCH* touch = new reTOUCH();

    touch->PartitioningType = PartitioningTypeMain;
    touch->nodesize         = nodesize;
    touch->leafsize         = leafsize;
    touch->localPartitions  = localPartitions;	
    touch->verbose          = verbose;		
    touch->localJoin        = localJoin;	
    touch->epsilon          = epsilon;	
    touch->numA             = numA;
    touch->numB             = numB;
    touch->treeTraversal    = traversalType;
    touch->maxLevelCoef     = maxLevelCoef;
    touch->file_dsA         = input_dsA;
    touch->file_dsB         = input_dsB;

    touch->run();
    touch->saveLog();
    touch->print();
}

void rereTOUCHrun()
{
    rereTOUCH* touch = new rereTOUCH();

    touch->PartitioningType = PartitioningTypeMain;
    touch->nodesize         = nodesize;
    touch->leafsize         = leafsize;
    touch->localPartitions  = localPartitions;	
    touch->verbose          = verbose;		
    touch->localJoin        = localJoin;	
    touch->epsilon          = epsilon;	
    touch->numA             = numA;
    touch->numB             = numB;
    touch->treeTraversal    = traversalType;
    touch->maxLevelCoef     = maxLevelCoef;
    touch->file_dsA         = input_dsA;
    touch->file_dsB         = input_dsB;

    touch->run();
    touch->saveLog();
    touch->print();
}

void algoNLrun()
{
    algoNL* nl = new algoNL();
            
    nl->verbose             = verbose;
    nl->epsilon             = epsilon;
    nl->numA                = numA;
    nl->numB                = numB;
    nl->file_dsA            = input_dsA;
    nl->file_dsB            = input_dsB;
    
    nl->run();
    nl->print();
}

void algoPSrun()
{
    algoPS* ps = new algoPS();
            
    ps->verbose             = verbose;
    ps->epsilon             = epsilon;
    ps->numA                = numA;
    ps->numB                = numB;
    ps->file_dsA            = input_dsA;
    ps->file_dsB            = input_dsB;
    
    ps->run();
    ps->print();
}

void S3run()
{
    S3Hash* ps = new S3Hash();
            
    ps->verbose             = verbose;
    ps->epsilon             = epsilon;
    ps->numA                = numA;
    ps->numB                = numB;
    ps->file_dsA            = input_dsA;
    ps->file_dsB            = input_dsB;
    
    ps->run();
    ps->print();
}

int main(int argc, const char* argv[])
{
    //Parsing the arguments
    parse_args(argc, argv);

    switch(algorithm)
    {
        case algo_NL:
            algoNLrun();
        break;
        case algo_PS:
            algoPSrun();
        break;
        case algo_TOUCH:
            TOUCHrun();
        break;
        case algo_cTOUCH:
            cTOUCHrun();
        break;
        case algo_dTOUCH:
            dTOUCHrun();
        break;
        case algo_reTOUCH:
            reTOUCHrun();
        break;
        case algo_S3:
            S3run();
        break;
        case algo_rereTOUCH:
            rereTOUCHrun();
        break;
        default:
            std::cout << "No such an algorithm!" << std::endl;
            exit(0);
        break;
    }
    
    std::cout << "Terminated." << std::endl;
    return 0;
}
