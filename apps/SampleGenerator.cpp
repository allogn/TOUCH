/*
 *  File: SampleGenerator.cpp
 * 
 *  Generation of random samples and logging statistics about their size
 * 
 */

#include <string>
#include <sstream>
#include <cstdlib>
#include <limits>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/uniform.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random.hpp>

#include "DataFileReader.hpp"
#include "BufferedFile.hpp"

FLAT::uint64 sampleSize      = 1000; // size of one sample

std::string inputFile   = "";
std::string outputPath  = "./samples/";
std::string logName     = "stats.csv";
int num                 = 1;
int distribution        = 0;
bool verbose            = false;
bool expand             = false;
typedef boost::mt19937 ENG;
typedef boost::normal_distribution<double> GDIST;
typedef boost::uniform_real<double> UDIST;
typedef boost::variate_generator<ENG,GDIST> GEN;
typedef boost::variate_generator<ENG,UDIST> UGEN;

void usage(const char *program_name) {

    printf("   Usage: %s\n", program_name);
    printf("   -h               Print this help menu\n");
    printf("   -i               Input filename\n");
    printf("   -p               Path to samples\n");
    printf("   -s               Sample size\n");
    printf("   -v               Verbose\n");
    printf("   -n               Number of files\n");
    printf("   -d               Type of random distribution (0 - Random, 1 - Gauss, 2 - Clustered)\n");
    printf("   -e               (boolean) Expand\n");

}

//Parsing Arguments
void parse_args(int argc, const char* argv[]) {

    int x,t;
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
                inputFile=argv[++x];
                break;
            case 'p':
                outputPath=argv[++x];
                break;
            case 's':
                sscanf(argv[++x], "%u", &sampleSize);
                break;
            case 'n':
                sscanf(argv[++x], "%u", &num);
                break;
            case 'd':
                sscanf(argv[++x], "%u", &distribution);
                break;
            case 'v':  
                t = 1;
                sscanf(argv[++x], "%u", &t);
                verbose = (t == 1) ? true : false;
                break;
            case 'e':  
                t = 1;
                sscanf(argv[++x], "%u", &t);
                expand = (t == 1) ? true : false;
                break;
            default:
                fprintf(stderr, "Error: Invalid command line parameter, %c\n", argv[x][1]);
                usage(argv[0]);
        }
    }
}

void generateSamples(std::string file_in, std::string path_out) 
{
    FLAT::BufferedFile* outputFile;
    
    //create log file
    std::string logfile = "";
    logfile.append(path_out);
    logfile.append(logName);
    ofstream log(logfile.c_str(),ios_base::app);
    log << "Filename, Average, Standard deviation, Minimum, Maximum\n";
    
    std::vector<FLAT::SpatialObject*> dsA;
    double max, min, avg, std; //statistics variables
    
    std::string prefix = "sample"; //prefix to names of result files
    std::ostringstream filename;
    int fileNum = 1; //index of a file
    int mixInd; //variable for randomization
    FLAT::SpatialObject* temp;
    double vol;
    
    FLAT::Box universe;
    
    if (verbose) std::cout << "Start reading the datasets" << std::endl;
    FLAT::DataFileReader *inputA = new FLAT::DataFileReader(file_in);

    if (sampleSize > inputA->objectCount)
        std::cout << "Number of objects in dataset must be greater or equal to dataset size\n";
    
    if (verbose)
        std::cout << "Size of dataset: every " << sampleSize << " out of " << inputA->objectCount << std::endl;
    
    
    FLAT::Box mbr;
    for (int i=0;i<DIMENSION;i++)
    {
            universe.low.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::max();
            universe.high.Vector[i]  = std::numeric_limits<FLAT::spaceUnit>::min();
    }
    FLAT::SpatialObject* sobj;

    /*
     * Read all file
     */
    dsA.reserve(inputA->objectCount);
    while(inputA->hasNext())
    {
        dsA.push_back(inputA->getNext());
        for (int i=0;i<DIMENSION;i++)
        {
            universe.low.Vector[i] = std::min(universe.low.Vector[i],mbr.low.Vector[i]);
            universe.high.Vector[i] = std::max(universe.high.Vector[i],mbr.high.Vector[i]);
        }
    }

    if (verbose) std::cout << "Reading Completed." << std::endl;
    
    /*
     * Mix first <sampleSize> elements with the rest
     * Write them to a file
     * Delete from the array
     * Until the array becomes empty
     */
    int donenum = 0;
    while (dsA.size() >= sampleSize && donenum < num)
    {
        donenum++;
        max = std::numeric_limits<double>::min();
        min = std::numeric_limits<double>::max();
        avg = 0;
        std = 0;
        
        filename.str("");
        filename << outputPath << prefix << fileNum << ".bin";
        
        if (verbose) 
        {
            std::cout << "File: " << filename.str() << std::endl;
            std::cout << "\tWriting headers..." << std::endl;
        }
        
        //Writing one file with random <sampleSize> objects
        outputFile = new FLAT::BufferedFile();
        outputFile->create(filename.str());
        if (outputFile->eof == true)
        {
            std::cout << "Output error. Check if output path exists. " << std::endl;
            exit(1);
        }
        
        //Mix objects
        if (verbose) std::cout << "\tMixing objects..." << std::endl;
        for (int i = 0; i < sampleSize; i++)
        {
            mixInd = (rand() % (sampleSize - i)) + i;
            temp = dsA.at(i);
            dsA.at(i) = dsA.at(mixInd);
            dsA.at(mixInd) = temp;
        }
        
        //Write objects and add each object to statistics
        if (verbose) std::cout << "\tWriting objects..." << std::endl;
        std::vector<FLAT::SpatialObject*>::iterator it = dsA.begin();
        for (int i = 0; i < sampleSize; i++)
        {
            vol = FLAT::Box::volume((*it)->getMBR());
            
            if (expand)
                (*it)->randomExpand(20.);
            
            if (vol > max) max = vol;
            if (vol < min) min = vol;
            avg += vol;
            std += vol*vol;
            outputFile->write((*it));
            it++;
        }
        avg /= sampleSize;
        std = sqrt(std - avg*avg);
        
        dsA.erase(dsA.begin(),--it);
        
        //Write header
        outputFile->writeUInt32(inputA->objectType);
        outputFile->writeUInt64(sampleSize);         // Number of Objects in Dataset
        outputFile->writeUInt32(inputA->objectByteSize);      // Size in Bytes of each Object
        outputFile->write(&(inputA->universe));
        
        outputFile->close();
        
        //write log
        log << filename.str() << "," << avg << "," << std << "," << min << "," << max << "\n";
        
        fileNum++;
    }
    
    if (verbose) std::cout << "Done." << std::endl;

}

void generateNewSamples(std::string path_out) 
{
    if (verbose) cout << "New sample generation started" << endl;
    FLAT::BufferedFile* outputFile;
    ofstream placeLog("./places.log", ios_base::out);
    
    //create log file
    std::string logfile = "";
    logfile.append(path_out);
    logfile.append(logName);
    ofstream log(logfile.c_str(),ios_base::app);
    log << "Filename, Average, Standard deviation, Minimum, Maximum\n";
    
    std::vector<FLAT::SpatialObject*> dsA;
    double max, min, avg, std; //statistics variables
    
    std::string prefix = "sample"; //prefix to names of result files
    std::ostringstream filename;
    int fileNum = 1; //index of a file
    int mixInd; //variable for randomization
    FLAT::SpatialObject* temp;
    double vol;
    
    FLAT::Box universe;
    
    FLAT::Box mbr;
    for (int i=0;i<DIMENSION;i++)
    {
            universe.low.Vector[i] = std::numeric_limits<FLAT::spaceUnit>::max();
            universe.high.Vector[i]  = std::numeric_limits<FLAT::spaceUnit>::min();
    }
    FLAT::SpatialObject* sobj;
    if (verbose) cout << "Universe created" << endl;
    
    dsA.reserve(sampleSize*num);
    if (verbose) cout << "Place reserved" << endl;
    
    FLAT::Box* b;
    ENG eng;
    
    double baseSize = 20;
    double maxExpand = 10.;
    int numOfClusters = 5;
    //srand (time(NULL));
    std::vector<FLAT::Vertex> clusterCenter(numOfClusters);
    if (distribution == 2)
    {
        for (int i = 0; i < numOfClusters; i++)
        {
            clusterCenter[i] = FLAT::Vertex(
                    (double)rand()/(double)RAND_MAX*2000-1000,
                    (double)rand()/(double)RAND_MAX*2000-1000,
                    (double)rand()/(double)RAND_MAX*2000-1000
                    );
        }
    }
    int chooseCluster;
    
    UDIST dist1(-1000,1000);
    UGEN gen1(eng,dist1);
    GDIST dist2(0,1000);
    GEN gen2(eng,dist2);
    GDIST dist3(0,200);
    GEN gen3(eng,dist3);
    if (verbose) cout << "Generators and clusters created" << endl;
    for (int i = 0; i < sampleSize*num; i++)
    {
        b = new FLAT::Box();
        for (int dim = 0; dim < DIMENSION; dim++)
        {
            switch (distribution)
            {
                case 0:
                    b->high[dim] = gen1();
                    break;
                case 1:
                    b->high[dim] = gen2();
                    break;
                case 2:
                    b->high[dim] = gen3();
                    break;
                    
            }
            b->low[dim] = b->high[dim] - baseSize;
        }
        if (distribution == 2)
        {
            chooseCluster = (int) ((double)rand()/((double)RAND_MAX+1) * numOfClusters); 
            b->high = b->high + clusterCenter[chooseCluster];
            b->low = b->low + clusterCenter[chooseCluster];
        }
        dsA.push_back(b);
        for (int i=0;i<DIMENSION;i++)
        {
            universe.low.Vector[i] = std::min(universe.low.Vector[i],mbr.low.Vector[i]);
            universe.high.Vector[i] = std::max(universe.high.Vector[i],mbr.high.Vector[i]);
        }
    }

    if (verbose) std::cout << "Objects created." << std::endl;
    
    /*
     * Mix first <sampleSize> elements with the rest
     * Write them to a file
     * Delete from the array
     * Until the array becomes empty
     */
    int donenum = 0;
    while (dsA.size() >= sampleSize && donenum < num)
    {
        donenum++;
        max = std::numeric_limits<double>::min();
        min = std::numeric_limits<double>::max();
        avg = 0;
        std = 0;
        
        filename.str("");
        filename << outputPath << prefix << fileNum << ".bin";
        
        if (verbose) 
        {
            std::cout << "File: " << filename.str() << std::endl;
            std::cout << "\tWriting headers..." << std::endl;
        }
        
        //Writing one file with random <sampleSize> objects
        outputFile = new FLAT::BufferedFile();
        outputFile->create(filename.str());
        if (outputFile->eof == true)
        {
            std::cout << "Output error. Check if output path exists. " << std::endl;
            exit(1);
        }
        
        //Mix objects
        if (verbose) std::cout << "\tMixing objects..." << std::endl;
        for (int i = 0; i < sampleSize; i++)
        {
            mixInd = (rand() % (sampleSize - i)) + i;
            temp = dsA.at(i);
            dsA.at(i) = dsA.at(mixInd);
            dsA.at(mixInd) = temp;
        }
        
        //Write objects and add each object to statistics
        if (verbose) std::cout << "\tWriting objects..." << std::endl;
        std::vector<FLAT::SpatialObject*>::iterator it = dsA.begin();
        for (int i = 0; i < sampleSize; i++)
        {
            
            if (expand)
            {
                (*it)->randomExpand(maxExpand);
            }
            vol = FLAT::Box::volume((*it)->getMBR());
            if (vol > max) max = vol;
            if (vol < min) min = vol;
            avg += vol;
            std += vol*vol;
            outputFile->write((*it));
            placeLog << (*it)->getCenter() << "\n";
            it++;
        }
        avg /= sampleSize;
        std = sqrt(std - avg*avg);
        
        dsA.erase(dsA.begin(),--it);
        
        //Write header
        outputFile->writeUInt32(FLAT::BOX);
        outputFile->writeUInt64(sampleSize);         // Number of Objects in Dataset
        outputFile->writeUInt32(DIMENSION*2*sizeof(FLAT::BOX));      // Size in Bytes of each Object
        outputFile->write(&(universe));
        
        outputFile->close();
        
        //write log
        log << filename.str() << "," << avg << "," << std << "," << min << "," << max << "\n";
        
        fileNum++;
    }
    
    if (verbose) std::cout << "Done." << std::endl;

}

int main(int argc, const char* argv[])
{
    //Parsing the arguments
    parse_args(argc, argv);
    if (inputFile == "")
    {
        generateNewSamples(outputPath);
    }
    else
    {
        generateSamples(inputFile, outputPath);
    }

    return 0;
}
