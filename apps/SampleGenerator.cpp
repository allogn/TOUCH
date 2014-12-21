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

#include "DataFileReader.hpp"
#include "BufferedFile.hpp"

FLAT::uint64 sampleSize      = 1000; // size of one sample

std::string inputFile   = "../data/RandomData-100K.bin";
std::string outputPath  = "./samples/";
std::string logName     = "stats.csv";
bool verbose            = false;

void usage(const char *program_name) {

    printf("   Usage: %s\n", program_name);
    printf("   -h               Print this help menu\n");
    printf("   -i               Input filename\n");
    printf("   -p               Path to samples\n");
    printf("   -s               Sample size\n");
    printf("   -v               Verbose\n");

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
            case 'v':  
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
    while (dsA.size() >= sampleSize)
    {
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

int main(int argc, const char* argv[])
{
    //Parsing the arguments
    parse_args(argc, argv);
    generateSamples(inputFile, outputPath);

    return 0;
}
