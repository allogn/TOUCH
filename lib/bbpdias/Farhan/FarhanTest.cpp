#include <iostream>
#include <fstream>
#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Containers/Morphologies.h>
#include <SpatialIndex.h>
#include "AABBCylinder.h"
#include "tools.hpp"


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

    // Building Morphology Index

    id_type indexIdentifier=0,segmentid=0;
    Morphologies & myMorphologies = microcircuit.morphologies();
    Morphologies::iterator myMorphologiesEnd = myMorphologies.end();
      for (Morphologies::iterator i = myMorphologies.begin(); i != myMorphologiesEnd; ++i)
      {
      	cout << "Indexing Morphology: " << i->label();
      	string baseName = i->label();
        IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(baseName, 4096);
        ISpatialIndex *tree = RTree::createNewRTree (*diskfile, 0.7, 127, 127, 3,RTree::RV_RSTAR,indexIdentifier);
        indexIdentifier++; segmentid=0;

      	Sections mySections = i->all_sections();
      	Sections::iterator mySectionsEnd = mySections.end();
      	for (Sections::iterator s = mySections.begin(); s != mySectionsEnd; ++s)
      	{
            Segments segments = s->segments();
      		Segments::const_iterator segments_end = segments.end();
      		for (Segments::const_iterator j = segments.begin(); j != segments_end ; ++j)
      		{

      			Box<bbp::Micron> Mbr = AABBCylinder::calculateAABBForCylinder(j->begin().center(),
      								  j->begin().radius(),j->end().center(),j->begin().radius());

      			vect plow, phigh;

      			plow[0] = Mbr.center().x() - Mbr.dimensions().x() / 2;
      			phigh[0] = Mbr.center().x() + Mbr.dimensions().x() / 2;
      			plow[1] = Mbr.center().y() - Mbr.dimensions().y() / 2;
      			phigh[1] = Mbr.center().y() + Mbr.dimensions().y() / 2;
      			plow[2] = Mbr.center().z() - Mbr.dimensions().z() / 2;
      			phigh[2] = Mbr.center().z() + Mbr.dimensions().z() / 2;

				SpatialIndex::Region mbr = SpatialIndex::Region(plow.data(),phigh.data(),3);

      			std::stringstream strStream;
      			strStream << s->id()<< "-" << j->id();

      			tree->insertData (strStream.str().length(), (byte*)(strStream.str().c_str()), mbr, segmentid);
      			segmentid++;
      		}
      	}
      	cout << ".. Total Segments: " << segmentid << "\n";
        tree->~ISpatialIndex();
        diskfile->~IStorageManager();
      }

    } catch (Tools::Exception& e) {
      cout << e.what() << endl;
      exit(0);
    }
   // delete tree_stats;
    return 0;
}
