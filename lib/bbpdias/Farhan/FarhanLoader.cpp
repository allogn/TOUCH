#include <iostream>
#include <fstream>
#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <SpatialIndex.h>

#include "tools.hpp"
#include "segment_stream.hpp"

using namespace bbp;
using namespace std;
using namespace SpatialIndex;

int main(int argc, const char* argv[])
{
    try {
    const URI blue_config_filename(argv[1]);

    // Loading NeuroCurcuit

    Experiment experiment;
    experiment.open(blue_config_filename);
    Microcircuit & microcircuit = experiment.microcircuit();
    const Targets & targets = experiment.targets();
    const Cell_Target target = targets.cell_target("Column");
    microcircuit.load(target, NEURONS | MORPHOLOGIES);

    // LOADING RTree Bulk Load

    Neurons & myNeurons = microcircuit.neurons();
    std::cout << "BULK LOADING R-Tree ";
    std::string baseName = "100KRtreeBulk";

    IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(baseName, 4096);
    id_type indexIdentifier=0;
    data_stream* ds = new segment_stream(&myNeurons, 5000000);
    ISpatialIndex *tree = RTree::createAndBulkLoadNewRTree (RTree::BLM_STR,*ds,*diskfile, 0.5,127, 127, 3,RTree::RV_RSTAR,indexIdentifier);

/*
    // VALIDATING QUERY
    vect plow,phigh;
    plow[0] = 0; plow[1] = 0; plow[2] =0;
    phigh[0]=90; phigh[1]=90; phigh[2]=90;
    SpatialIndex::Region query  = SpatialIndex::Region(plow.data(),phigh.data(),3);

    // LOADING RTree Sequential Load

    std::cout << "SEQUENTIAL LOADING R-Tree \n";
    std::string baseName = "RTreeSerial1200";
    IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(baseName, 4096);
    id_type indexIdentifier=0,segmentid=0;
    ISpatialIndex *tree = RTree::createNewRTree (*diskfile, 0.7, 127, 127, 3,RTree::RV_RSTAR,indexIdentifier);
    int nc=0;
    Neurons & myNeurons = microcircuit.neurons();
    Neurons::iterator myNeuronsEnd = myNeurons.end();
    for (Neurons::iterator i = myNeurons.begin(); i != myNeuronsEnd; ++i)
    {
    	if (nc>=1200) break; else nc++;
    	Transform_3D<Micron> trafo = i->global_transform();
    	if (nc%50==0) cout << "Neurons Done: " << nc << "\n";
    	Sections mySections = i->morphology().all_sections();
    	Sections::iterator mySectionsEnd = mySections.end();
    	for (Sections::iterator s = mySections.begin(); s != mySectionsEnd; ++s)
    	{
            Segments segments = s->segments();
    		Segments::const_iterator segments_end = segments.end();
    		for (Segments::const_iterator j = segments.begin(); j != segments_end ; ++j)
    		{
    		    //SpatialIndex::Region mbr;
    		    //get_segment_mbr(*j, trafo, NULL);

    			vect plow, phigh;
    			get_segment_mbr (*j, trafo, &plow, &phigh);
    			SpatialIndex::Region mbr = SpatialIndex::Region(plow.data(),phigh.data(),3);

    			std::stringstream strStream;
    			strStream << i->gid() <<"-"<< s->id()<< "-" << j->id();

				//if (query.containsRegion(mbr)==true)   cout << "Validating Containment Data: " << segmentid <<"\n";
				//if (query.intersectsRegion(mbr)==true) cout << "Validating Intersect Data: " << segmentid <<"\n";

    			tree->insertData (strStream.str().length(), (byte*)(strStream.str().c_str()), mbr, segmentid);
    			segmentid++;
    		}
    	}
    }
*/
    tree->~ISpatialIndex();
    diskfile->~IStorageManager();

    // RTree Statistics

    /*std::cout << "Checking R-tree structure... ";
    if (!tree->isIndexValid()) std::cerr << "R-tree internal checks failed!\n"; else std::cout << "OK\n";
    IStatistics * tree_stats;
    tree->getStatistics (&tree_stats);
    cout << *tree_stats;
    delete tree_stats;
*/


    // Removing and Ending
    delete tree;
    delete diskfile;


    } catch (Tools::Exception& e) {
      cout << e.what() << endl;
      exit(0);
    }
    return 0;
}
