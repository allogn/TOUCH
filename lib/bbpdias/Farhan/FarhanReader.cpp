#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include "range_query_visitor.hpp"
#include "tools.hpp"
#include <SpatialIndex.h>

using namespace bbp;
using namespace std;
using namespace SpatialIndex;

#define QUERIES_FOR_ANALYSIS 1000

int main(int argc, const char* argv[])
{
	std::cout << "Reading Index File ";
    // Reading RTree Sequential Load
    try {
    std::string baseName = "RTreeSerial";

    IStorageManager* diskfile = StorageManager::loadDiskStorageManager(baseName);
    //StorageManager::createNewRandomEvictionsBuffer(*diskfile, 10, false);
    ISpatialIndex *tree = RTree::loadRTree(*diskfile, 1);

   //  RTree Statistics

    std::cout << "Checking R-tree structure... ";
    if (!tree->isIndexValid()) std::cerr << "R-tree internal checks failed!\n"; else std::cout << "OK\n";
    IStatistics * tree_stats;
    tree->getStatistics (&tree_stats);
    cout << *tree_stats;
    //tree->

    // PERFORMANCE EVALUATION RANDOM RANGE QUERIES
    //cout << "Start Range Query Analysis \n";
    range_query_visitor visitor;
 	double plow[3],phigh[3];
 	plow[0]  = 98.2538; plow[1]  = 1005.14; plow[2]  = 92.9046;
 	phigh[0] = 452.301; phigh[1] = 1385.56; phigh[2] = 456.743;
 	//plow[0]  = -1698.38; plow[1] = -1065.03; plow[2] = -1724.75;
 	//phigh[0] = 2248.56;  phigh[1]= 1894.28;  phigh[2]= 2276.71;

    SpatialIndex::Region query_region = SpatialIndex::Region(plow,phigh,3);
 	boost::mt11213b generator (42u);
	const double x[3] = {0.0, 0.0, 0.0};
	SpatialIndex::Point rnd_point1 (x, query_region.m_dimension);
	SpatialIndex::Point rnd_point2 (x, query_region.m_dimension);


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

 		//cout << "Point1: (" <<  rnd_point1.m_pCoords[0] << "," <<  rnd_point1.m_pCoords[1] << "," << rnd_point1.m_pCoords[2] << ")   ";
 		//cout << "Point2: (" <<  rnd_point2.m_pCoords[0] << "," <<  rnd_point2.m_pCoords[1] << "," << rnd_point2.m_pCoords[2] << ")\n";

	    SpatialIndex::Region query = SpatialIndex::Region (rnd_point1,rnd_point2);
	    visitor.new_query();
 		tree->intersectsWithQuery(query,visitor);
 		if (j%100==0) cout << "Queries Done: " << j << "\n";
 	}
 	visitor.print_stats();

/*

    // PERFORMANCE EVALUATION RANDOM POINT QUERIES
    cout << "Start Point Query Analysis \n";
    range_query_visitor visitor;
	  double plow[3],phigh[3];
	  //plow[0]  = 98.2538; plow[1]  = 1005.14; plow[2]  = 92.9046;
	  //phigh[0] = 452.301; phigh[1] = 1385.56; phigh[2] = 456.743;
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
		  tree->pointLocationQuery(rnd_point,visitor);

	  }
	  visitor.print_stats();
*/

    // RangeQuery Result
/*
    vect qlow,qhigh;
    qlow[0]  = 0;
    qhigh[0] = 90;
    qlow[1]  = 0;
    qhigh[1] = 90;
    qlow[2]  = 0;
    qhigh[2] = 90;
    range_query_visitor visitor;
    SpatialIndex::Region mbr = SpatialIndex::Region(qlow.data(),qhigh.data(), 3);
    IShape * query = new SpatialIndex::Region (mbr);

    cout << " Intersect Query\n";
    tree->intersectsWithQuery(*query,visitor);
    visitor.print_stats();

    visitor.reset_stats();
    cout << " Contains Query\n";
    tree->containsWhatQuery(*query,visitor);
    visitor.print_stats();

*/
    // Evaluating RTree Performance
    /*
    vect plow, phigh;
    plow[0] = -1698.44;
    phigh[0] = 2248.47;
    plow[1] = -1064.97;
    phigh[1] = 1894.41;
    plow[2] = -1724.49;
    phigh[2] = 2276.45;
    SpatialIndex::Region world_region = SpatialIndex::Region(plow.data(),phigh.data(), 3);
    cout << "World extent according to the stream: " << world_region << '\n';

    boost::mt11213b generator (42u);
    double x[3] = {0.0, 0.0, 0.0};

    access_count_visitor v;
    for (int j = 0; j < 10000; j++)
		{
    	Point rnd_point (x, world_region.m_dimension);
    	for (size_t i = 0; i < world_region.m_dimension; i++)
			{
    		boost::uniform_real<> uni_dist (world_region.m_pLow[i],world_region.m_pHigh[i]);
    		boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
    		rnd_point.m_pCoords[i] = uni();
			}
    	assert (world_region.containsPoint (rnd_point));
    	// cout << "Random point: " << rnd_point << '\n';
    	v.new_query();
    	tree->pointLocationQuery (rnd_point, v);
        }
    v.print_stats();
*/

	// Removing and Ending

    delete tree;
    delete diskfile;
    } catch (Tools::Exception& e) {
      cout << e.what() << endl;
      exit(0);
    }
   // delete tree_stats;
    return 0;
}
