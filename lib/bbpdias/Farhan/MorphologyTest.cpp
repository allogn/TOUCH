#include <iostream>
#include <fstream>
#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Neuron.h>
#include <BBP/Model/Microcircuit/Containers/Morphologies.h>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include "range_query_visitor.hpp"
#include "tools.hpp"
#include "AABBCylinder.h"
#include <SpatialIndex.h>

using namespace bbp;
using namespace std;
using namespace SpatialIndex;

#define NEURONS_COUNT 1692
#define MORPHOLOGIES_COUNT 48
#define QUERIES_FOR_ANALYSIS 1000

int main(int argc, const char* argv[])
{
	/*
	 *  Build 48 Index with Links
	 */


	// Load Circuit
    Experiment experiment;
    experiment.open(blue_config_filename);
    Microcircuit & microcircuit = experiment.microcircuit();
    const Targets & targets = experiment.targets();
    const Cell_Target target = targets.cell_target("Column");
    microcircuit.load(target, NEURONS | MORPHOLOGIES);

    //Make Neuron Rtrees
    ISpatialIndex *neuronTrees[MORPHOLOGIES_COUNT];
    string *morphologyLabels[MORPHOLOGIES_COUNT];

    int cm=0;
    Morphologies & myMorphologies = microcircuit.morphologies();
    Morphologies::iterator myMorphologiesEnd = myMorphologies.end();
      for (Morphologies::iterator i = myMorphologies.begin(); i != myMorphologiesEnd; ++i)
      {
    	  morphologyLabels[cm] = i->label();
    	  neuronTrees[cm] = RTree::createNewRTree (createNewMemoryStorageManager(), 0.7, 127, 127, 3,RTree::RV_RSTAR,indexIdentifier);
    	  cm++;
      }

    Neurons & myNeurons = microcircuit.neurons();
    Neurons::iterator myNeuronsEnd = myNeurons.end();
    for (Neurons::iterator i = myNeurons.begin(); i != myNeuronsEnd; ++i)
    {
    	cm=0;
    	for (cm=0;cm<MORPHOLOGIES_COUNT;cm++)
    		if (strcmp(i->morphology().label(),morphologyLabels[cm])==0) break;

    	Transform_3D<Micron> trafo = i->global_transform();
    	Sections mySections = i->morphology().all_sections();
    	Sections::iterator mySectionsEnd = mySections.end();
    	for (Sections::iterator s = mySections.begin(); s != mySectionsEnd; ++s)
    	    {
    		 Segments segments = s->segments();
    		 Segments::const_iterator segments_end = segments.end();
    		 for (Segments::const_iterator j = segments.begin(); j != segments_end ; ++j)
    		     {
     			 vect plow, phigh;
     			 get_segment_mbr (*j, trafo, &plow, &phigh);
     			 SpatialIndex::Region mbr = SpatialIndex::Region(plow.data(),phigh.data(),3);

     			 std::stringstream strStream;
     			 strStream << i->gid() <<"-"<< s->id()<< "-" << j->id();
     			 neuronTrees[cm]->insertData (strStream.str().length(), (byte*)(strStream.str().c_str()), mbr, segmentid);
    		     }
    	    }
    }

    // Make Morphology Rtrees
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


    // PRELOAD the Trees amd Neuron Morphology Mapping


    ISpatialIndex *neurons[NEURONS_COUNT];
    global_transformer *transforms[NEURONS_COUNT];

    int cm=0;
    int cn=0;
    string baseName = "";
    Morphologies & myMorphologies = microcircuit.morphologies();
    Neurons & myNeurons = microcircuit.neurons();

    cout << "PreLoading Mappings \n";
    Morphologies::iterator myMorphologiesEnd = myMorphologies.end();
      for (Morphologies::iterator m = myMorphologies.begin(); m != myMorphologiesEnd; ++m)
      {
    	  baseName = m->label();
    	  m->
          IStorageManager* diskfile = StorageManager::loadDiskStorageManager(baseName);
          trees[cm] = RTree::loadRTree(*diskfile, 1);

          std::cout << "Checking R-tree structure... ";
          if (!trees[cm]->isIndexValid()) std::cerr << "R-tree internal checks failed!\n"; else std::cout << "OK\n";
          IStatistics * tree_stats;
          trees[cm]->getStatistics (&tree_stats);
          cout << *tree_stats;

          Neurons::iterator myNeuronsEnd = myNeurons.end();
          for (Neurons::iterator n = myNeurons.begin(); n != myNeuronsEnd; ++n)
              {
        	  if (strcmp(n->morphology().label().c_str(),m->label().c_str())==0)
        		  {
        		  transforms[cn] = n->global_transform().inverse();
        		  neurons[cn] = trees[cm];
        		  }
        	  cn++;
        	  if (cn>=NEURONS_COUNT) break;
              }
          cn=0;cm++;
      }




	/*
	 *  Query the Index
	 */








}
