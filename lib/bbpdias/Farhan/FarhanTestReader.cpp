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
    const URI blue_config_filename(argv[1]);
    try {
    	// LOADING CURCUIT
        Experiment experiment;
        experiment.open(blue_config_filename);
        Microcircuit & microcircuit = experiment.microcircuit();
        const Targets & targets = experiment.targets();
        const Cell_Target target = targets.cell_target("Column");
        microcircuit.load(target, NEURONS | MORPHOLOGIES);

        // PRELOAD the Trees amd Neuron Morphology Mapping
        ISpatialIndex *trees[MORPHOLOGIES_COUNT];
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
          // SINGLE QUERY
          cout << "Start Querying \n";
          range_query_visitor visitor;
		  SpatialIndex::Region query;

		  micron_vector plow,phigh;
		  plow[0] = 0; plow[1] = 0; plow[2] = 0;
		  phigh[0]=90; phigh[1]=90; phigh[2]=90;

          for (int i=0;i<NEURONS_COUNT;i++)
          {
    		  get_transformed_cube_mbr(plow,phigh,*transforms[i],&query);
    		  neurons[i]->intersectsWithQuery(query,visitor);
    		  visitor.inc_neuron();
          }
          visitor.print_stats();
*/

          // PERFORMANCE EVALUATION RANDOM RANGE QUERIES
          {
          cout << "Start Range Query Analysis \n";
          range_query_visitor visitor;
  		  double plow[3],phigh[3];
  		  plow[0]  = 98.2538; plow[1]  = 1005.14; plow[2]  = 92.9046;
  		  phigh[0] = 452.301; phigh[1] = 1385.56; phigh[2] = 456.743;
  	 	  //plow[0]  = -1698.38; plow[1] = -1065.03; plow[2] = -1724.75;
  	 	 // phigh[0] = 2248.56;  phigh[1]= 1894.28;  phigh[2]= 2276.71;
          SpatialIndex::Region query_region = SpatialIndex::Region(plow,phigh,3);
  		  boost::mt11213b generator (42u);
  		  const double x[3] = {0.0, 0.0, 0.0};
  		  SpatialIndex::Point rnd_point1 (x, query_region.m_dimension);
  		  SpatialIndex::Point rnd_point2 (x, query_region.m_dimension);
  		  SpatialIndex::Region query;

  		  for (int j=0;j<QUERIES_FOR_ANALYSIS;j++)
  		  {
  		     for (size_t i = 0; i < query_region.m_dimension; i++)
  	 	     {
  	 	          boost::uniform_real<> uni_dist (query_region.m_pLow[i],query_region.m_pHigh[i]);
  	 	          boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
  	 	          rnd_point1.m_pCoords[i] = uni();

  	 	          boost::uniform_real<> uni_dist1 (rnd_point1.m_pCoords[i],query_region.m_pHigh[i]);
  	 	          boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni1(generator, uni_dist1);
  	 	          rnd_point2.m_pCoords[i] = uni1();
  	 	     }
  			  visitor.new_query();
  			  visitor.reset_neuron();
  			  for (int i=0;i<NEURONS_COUNT;i++)
  			  {
  				  Vector_3D<Micron> low  = Vector_3D<Micron>((Micron)rnd_point1.m_pCoords[0],(Micron)rnd_point1.m_pCoords[1],(Micron)rnd_point1.m_pCoords[2]);
  				  Vector_3D<Micron> high = Vector_3D<Micron>((Micron)rnd_point2.m_pCoords[0],(Micron)rnd_point2.m_pCoords[1],(Micron)rnd_point2.m_pCoords[2]);
  				  get_transformed_cube_mbr(low,high,*transforms[i],&query);
  				  neurons[i]->intersectsWithQuery(query,visitor);
  			  }
  			  if (j%100==0) cout << "Queries Done: " << j << "\n";
  		  }
  		  visitor.print_stats();
          }
/*
          {
          // PERFORMANCE EVALUATION RANDOM POINT QUERIES
          cout << "Start Point Query Analysis \n";
          range_query_visitor visitor;
		  double plow[3],phigh[3];
		  //plow[0]  = 98.2538; plow[1]  = 1005.14; plow[2]  = 92.9046;
		 // phigh[0] = 452.301; phigh[1] = 1385.56; phigh[2] = 456.743;
  	 	  plow[0]  = -1698.38; plow[1] = -1065.03; plow[2] = -1724.75;
  	 	  phigh[0] = 2248.56;  phigh[1]= 1894.28;  phigh[2]= 2276.71;
          SpatialIndex::Region query_region = SpatialIndex::Region(plow,phigh,3);
		  boost::mt11213b generator (42u);

		  for (int j=0;j<QUERIES_FOR_ANALYSIS;j++)
		  {
			  const double x[3] = {0.0, 0.0, 0.0};
			  SpatialIndex::Point rnd_point (x, query_region.m_dimension);
			  SpatialIndex::Point tmp_point (x, query_region.m_dimension);

			  for (size_t i = 0; i < query_region.m_dimension; i++)
		      {
		          boost::uniform_real<> uni_dist (query_region.m_pLow[i],query_region.m_pHigh[i]);
		          boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
		          rnd_point.m_pCoords[i] = uni();
		      }
			  visitor.new_query();
			  visitor.reset_neuron();
			  //visitor.reset_stats();
			  for (int i=0;i<NEURONS_COUNT;i++)
			  {
				 // cout << "Point Query: (" <<  rnd_point.m_pCoords[0] << "," <<  rnd_point.m_pCoords[1] << "," << rnd_point.m_pCoords[2] << ") Transformed to: (";
				  Vector_3D<Micron> v = Vector_3D<Micron>((Micron)rnd_point.m_pCoords[0],(Micron)rnd_point.m_pCoords[1],(Micron)rnd_point.m_pCoords[2]);
				  v = *transforms[i]*v;
				  tmp_point.m_pCoords[0] = (double)v.x();
				  tmp_point.m_pCoords[1] = (double)v.y();
				  tmp_point.m_pCoords[2] = (double)v.z();
				 // cout <<  tmp_point.m_pCoords[0] << "," <<  tmp_point.m_pCoords[1] << "," << tmp_point.m_pCoords[2] << ")\n";
				  neurons[i]->pointLocationQuery(tmp_point,visitor);
			  }

		  }
		  visitor.print_stats();
          }
*/
          // PERFORMANCE EVALUATION RANDOM RANGE QUERIES





    }
    catch (Tools::Exception& e)
    {
      cout << e.what() << endl;
      exit(0);
    }
   // delete tree_stats;
    return 0;
}
