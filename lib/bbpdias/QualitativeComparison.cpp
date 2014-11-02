#include <vector>
#include <iomanip>
#include "Rtree.cpp"
#include "QueryGenerator.cpp"
#include "cpu_timer.cpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <string>

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <SpatialIndex.h>

#include "Query.cpp"
#include "Data.cpp"
#include "Rtree.cpp"
#include "QueryGenerator.cpp"

#include "delaunay_connectivity.hpp"
const SpatialIndex::id_type header_page_id = 0;

typedef boost::unordered_set<SpatialIndex::id_type> visitedSet;
typedef std::vector<dias::Query> query_vect;
typedef std::set<SpatialIndex::id_type> page_id_set;
typedef std::map<vertex_id, SpatialIndex::id_type> vertex_to_page_id_map_cache;

using namespace std;

int main(int argc, const char* argv[]) {
	dias::Rtree* myTree = new dias::Rtree(argv[1], false);
	dias::QueryGenerator qg;
	qg.Load(argv[3]);

	std::string input_base_fn (argv[2]);
	std::string seed_fn = input_base_fn + "_seed";
	std::string crawl_fn = input_base_fn + "_graph";

	boost::shared_ptr<dias::Rtree> seed_index (new dias::Rtree (seed_fn, false));
	boost::shared_ptr<SpatialIndex::IStorageManager> crawl_file(SpatialIndex::StorageManager::loadDiskStorageManager (crawl_fn));

	for (vector<dias::Query>::iterator query = qg.queries.begin(); query != qg.queries.end(); ++query) {

		myTree->RangeSearch(*query);
		std::vector<dias::Data> rtreedataresult = query->getQueryResult();

		set<std::string> rtreeresult;

		for(unsigned int i=0; i<rtreedataresult.size(); i++) {
			stringstream oss;
			oss << rtreedataresult.at(i).midPoint;
			rtreeresult.insert(oss.str());
		}

		vector<dias::Vertex> oaresult;

        std::queue<dias::vertex_info> que;
        visitedSet visited;

        seed_index->Seed (*query);
        if (query->points > 0)
        {
            SpatialIndex::id_type page_id = query->seed.id;

            std::vector<dias::vertex_info> allVertices;
            dias::read_all_vertices_from_page (*crawl_file, page_id,
                                               allVertices);
            visited.insert(page_id);

            for (std::vector<dias::vertex_info>::const_iterator it
                     = allVertices.begin ();
                 it != allVertices.end ();
                 it++)
            {
                dias::Box vtx_voronoi_mbr = it->voronoi_mbr.as_box ();
                if (dias::Box::overlap(vtx_voronoi_mbr, query->box))
                {
                    que.push (*it);
                }
            }
        }

        while (true)
        {
            if (que.empty ()==true) break;

            const struct dias::vertex_info & vtx = que.front ();

            dias::Vertex vert(vtx.coords[0], vtx.coords[1], vtx.coords[2]);

            if (dias::Box::enclose(query->box, vert)) {
            	oaresult.push_back(vert);
			}

            BOOST_FOREACH (SpatialIndex::id_type n, vtx.neighbours)
            {
                if (visited.find (n) != visited.end ())
                    continue;

                std::vector<dias::vertex_info> allVertices;
                dias::read_all_vertices_from_page (*crawl_file, n, allVertices);
                visited.insert (n);

                for (std::vector<dias::vertex_info>::const_iterator it
                         = allVertices.begin (); it != allVertices.end ();  it++)
                {
                    if (dias::Box::overlap(it->voronoi_mbr.as_box (), query->box))
                    {
                        que.push (*it);
                    }
				}
            }
            que.pop ();
        }

        set<std::string> oasresult;

		for(vector<dias::Vertex>::iterator it=oaresult.begin(); it != oaresult.end (); it++) {
			stringstream oss;
			oss << *it;
			oasresult.insert(oss.str());
		}

        std::cout << "RTree Result Size: " << rtreeresult.size() << " HALT Result Size: " << oasresult.size() << " for " << *query << std::endl;

        set<std::string>::iterator its=oasresult.begin();

        while(its != oasresult.end ()) {
        	set<std::string>::iterator e = rtreeresult.find(*its);

        	if(e != rtreeresult.end()) {
				rtreeresult.erase(*e);
				oasresult.erase(its++);
        	} else {
        		++its;
        	}
        }

        if(rtreeresult.size() > 0) {
			std::cout << "RTree Result Difference (" << rtreeresult.size() << ")" << std::endl;
			for(set<std::string>::iterator it=rtreeresult.begin(); it != rtreeresult.end (); it++) {
				std::cout << "> " << *it << std::endl;
			}
        }

        if(oasresult.size() > 0) {
			std::cout << "HALT Result Difference (" << oasresult.size() << ")" << std::endl;
			for(set<std::string>::iterator it=oasresult.begin(); it != oasresult.end (); it++) {
				std::cout << "< " << *it << std::endl;
			}
        }
	}

	return 0;
}
