#include <cassert>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <SpatialIndex.h>

#include "Query.cpp"
#include "QueryGenerator.cpp"
#include "cpu_timer.cpp"
#include "Rtree.cpp"
#include "delaunay_connectivity.hpp"
//#define VERTICES_IN_PAGE 200 // TODO enum or something


#ifdef PROFILING
tpie::cpu_timer loadingc,a1,a2,totalc,seedingc,getvertexc,visittimec,candidatec,readverticespagec,getvertexpagec,getfromcachec,c1,c2,c3,c4,c5,p1,p2,p3,p4,d1,d2,d3;
#endif



#include "delaunay_connectivity.hpp"
const SpatialIndex::id_type header_page_id = 0;

typedef boost::unordered_set<SpatialIndex::id_type> visitedSet;
typedef std::vector<dias::Query> query_vect;
typedef std::set<SpatialIndex::id_type> page_id_set;
typedef std::map<vertex_id, SpatialIndex::id_type> vertex_to_page_id_map_cache;

class CrawlStatistics
{
public:
	int SeedIOs;
	int CrawlIOs;
	int ConsideredPoints; // Used in BFS maybe Result or maybe Rejected
	int RejectedPoints;   // Used in BFS but not inside query
	int ResultPoints;     // Used in BFS and inside Query
	int UseLessPoints;    // Never used in BFS but were in page
	int UselessPage;
	int ResultPage;
	int MixedPage;

	CrawlStatistics()
	{
		UselessPage=0;
		ResultPage=0;
		MixedPage=0;
		SeedIOs=0;
		CrawlIOs=0;
		ConsideredPoints=0;
		UseLessPoints=0;
		ResultPoints=0;
		RejectedPoints=0;
	}

	void print()
	{
            std::cout  << SeedIOs << "," << CrawlIOs << "," << UselessPage << "," << ResultPage << "," << MixedPage << ","
            << ResultPoints << ","  << RejectedPoints << "," << UseLessPoints << std::endl;
	}

};

static const std::string process_attach = "-p";

int main (int argc, char* argv[])
{

	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><>< ARGUMENTS ><><><><><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>


        if (argc != 3 && argc != 4)
        {
            std::cerr << "Wrong number of parameters!\n";
            return 1;
        }
        std::string input_base_fn (argv[1]);
        std::string seed_fn = input_base_fn + "_seed";
        std::string crawl_fn = input_base_fn + "_graph";
        std::string query_fn (argv[2]);

        const bool pause_for_attach
            = (argc == 4 && process_attach == argv[3]);
        if (pause_for_attach)
        {
            std::cerr << "PID = " << getpid () << '\n';
            std::cerr << "Press Enter to continue\n";
            std::string dummy;
            std::getline (std::cin, dummy);
        }

        unsigned grand_total_ios = 0;
        unsigned grand_total_objs = 0;

        try {
#ifdef DETAILED_STATISTICS
	std::ofstream writeFile;
	writeFile.open ("statistics.csv",std::ios::out |std::ios::trunc);
#endif
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><<>< LOADING Files ><<><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef PROFILING
loadingc.start();
#endif
	dias::QueryGenerator qg;
	qg.Load (query_fn);

	boost::shared_ptr<dias::Rtree> seed_index (new dias::Rtree (seed_fn, false));
	boost::shared_ptr<SpatialIndex::IStorageManager> crawl_file(SpatialIndex::StorageManager::loadDiskStorageManager (crawl_fn));

#ifdef PROFILING
loadingc.stop();
totalc.start();
#endif
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><<>< For Each Query ><><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
        std::cout << "Seed I/Os,Crawl I/Os,Completely Useless Pages,Result Pages,Mixed Pages,Result Points,Rejected Points,Useless Points" <<std::endl;

	BOOST_FOREACH (dias::Query q, qg.queries)
	{
            std::queue<dias::vertex_info> que;
            visitedSet visited;
            CrawlStatistics cs;

// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// <><><><><><><><><><><><><<<><>>< SEEDING ><<<>><><><><><><><><><><><><><
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef PROFILING
            seedingc.start();
#endif

            seed_index->Seed (q);
            if (q.points>0)
            {
                SpatialIndex::id_type page_id = q.seed.id;

                std::vector<dias::vertex_info> allVertices;
                dias::read_all_vertices_from_page (*crawl_file, page_id,
                                                   allVertices);
                visited.insert(page_id);
                cs.SeedIOs += q.nodeAccesses;
                cs.CrawlIOs = 1; // This read belongs to crawl I/Os
                int uselessness=0,usefullness=0;
                for (std::vector<dias::vertex_info>::const_iterator it
                         = allVertices.begin ();
                     it != allVertices.end ();
                     it++)
                {
                    dias::Box vtx_voronoi_mbr = it->voronoi_mbr.as_box ();
                    if (dias::Box::overlap(vtx_voronoi_mbr, q.box))
                    {
                        cs.ConsideredPoints++;
                        usefullness++;
                        if (dias::Box::enclose(q.box,dias::Vertex(it->coords[0],it->coords[1],it->coords[2])))
                            cs.ResultPoints++;
                        else
                            cs.RejectedPoints++;
                        que.push (*it);
                    }
                    else
                    {
                        cs.UseLessPoints++;
                        uselessness++;
//					delete *it; TODO what to do with useless vertices? // do nothings its just for measuring the performance
                    }
                }
                if (uselessness==allVertices.size()) cs.UselessPage++;
                else if (usefullness==allVertices.size()) cs.ResultPage++;
                else cs.MixedPage++;
            }
#ifdef PROFILING
            seedingc.stop();
#endif
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// <><><><><><><><><><><><><<<><>><>< BFS ><<><<>><><><><><><><><><><><><><
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

            while (true)
            {

// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// <><><><><><><><><><><><><<<><>><>< VISIT <><<>><><><><><><><><><><><><><
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef PROFILING
                visittimec.start();
#endif

#ifdef PROFILING
                a1.start();
#endif
                if (que.empty ()==true) break;
                const struct dias::vertex_info & vtx = que.front ();

#ifdef PROFILING
                a1.stop();
#endif
                if (dias::Box::enclose(q.box,dias::Vertex(vtx.coords[0],vtx.coords[1],vtx.coords[2])))
                    cs.ResultPoints++;
                else
                    cs.RejectedPoints++;

#ifdef PROFILING
                visittimec.stop();
                candidatec.start();
#endif
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// <><><><><><><><><><><><><< CANDIDATE FOR BFS <><><><><><><><><><><><><><
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
                BOOST_FOREACH (SpatialIndex::id_type n, vtx.neighbours)
                {
#ifdef PROFILING
                    p3.start();
#endif
                    if (visited.find (n) != visited.end ())
                        continue;

                    std::vector<dias::vertex_info> allVertices;
                    dias::read_all_vertices_from_page (*crawl_file, n,
                                                       allVertices);
                    int uselessness=0,usefullness=0;

#ifdef DETAILED_STATISTICS
                    // Statistics
                     dias::Box pointBB;
                     dias::Box boundingBoxBB;
                     std::vector<dias::Vertex> vertices;
                     int countResults=0;
                     int countRejected=0;
                     int countUseless=0;
                     int totalLinks=0;
                     int insideLinks=0;
                     int outsideLinks=0;
                     std::set<vertex_id> uniquePageLinks;
                     for (std::vector<dias::vertex_info>::const_iterator it = allVertices.begin ();it != allVertices.end (); it++)
                     {
                     	dias::Vertex vertex(it->coords[0],it->coords[1],it->coords[2]);
                     	if (dias::Box::overlap(it->voronoi_mbr.as_box (), q.box))
                     	{
                     		if (dias::Box::enclose(q.box,vertex))
                     		{
 								countResults++;
                     		}
                     		else
                     		{
                     			countRejected++;
                     		}
                     	}
                     	else
                     	{
 							countUseless++;
                     	}
                     	if (vertices.size()==0)
                     		boundingBoxBB = it->voronoi_mbr.as_box();
                     	else
                     		dias::Box::combine(boundingBoxBB,it->voronoi_mbr.as_box(),boundingBoxBB);

                     	totalLinks += it->neighbours.size();
                     	for (std::vector<vertex_id>::const_iterator nt = it->neighbours.begin();nt!=it->neighbours.end();nt++)
                     	{
                     		if (*nt==n) insideLinks++;
                     		else outsideLinks++;
                     		uniquePageLinks.insert(*nt);
                     	}

 						vertices.push_back(vertex);
                     }
                     dias::Box::boundingBox(pointBB,vertices);
                     writeFile << vertices.size() << "," << countResults << "," << countRejected << "," << countUseless << ","
 							  << totalLinks << "," << insideLinks << "," << outsideLinks <<"," << uniquePageLinks.size() << ","
 							<< dias::Box::volume(pointBB) << "," << dias::Box::volume(boundingBoxBB) << std::endl;

#endif

                    visited.insert (n);
#ifdef PROFILING
                    p3.stop();
#endif

                    for (std::vector<dias::vertex_info>::const_iterator it
                             = allVertices.begin ();
                         it != allVertices.end ();
                         it++)
                    {
                        if (dias::Box::overlap(it->voronoi_mbr.as_box (), q.box))
                        {
                            que.push (*it);
                            cs.ConsideredPoints++;
                            usefullness++;
                        }
                        else
                        {
                            cs.UseLessPoints++;
                            uselessness++;
//							delete *it;
                        }
					}
                    cs.CrawlIOs++;
                    if (uselessness==allVertices.size()) cs.UselessPage++;
                    else if (usefullness==allVertices.size()) cs.ResultPage++;
                    else cs.MixedPage++;
                }
                que.pop ();
//		delete vtx;
#ifdef PROFILING
                candidatec.stop();
#endif
            }
            cs.print();
            grand_total_ios += cs.SeedIOs + cs.CrawlIOs;
            grand_total_objs += cs.ResultPoints;
        }


#ifdef PROFILING
        totalc.stop();
        std::cout << "\nPROFILING:"
                  << "\n\tLoading:" << loadingc
                  << "\n\tTOTAL time:" << totalc
                  << "\n\tSeeding time:" << seedingc
                  << "\n\tVisiting Top of Queue:" << visittimec
                  << "\n\t\t a1:" << a1
                  << "\n\tChecking Candidates:" << candidatec
                  << "\n\t\t p1:" << p1
                  << "\n\t\t p2:" << p2
                  << "\n\t\t p3:" << p3
                  << "\n"
                  << "\n\t\t\t c1:" << c1
                  << "\n\t\t\t c2:" << c2
                  << "\n\t\t\t c3:" << c3
                  << "\n\t\t\t c4:" << c4
                  << "\n\t\t\t c5:" << c5
                  << "\n\t\t\t d1:" << d1
                  << "\n\t\t\t d2:" << d2
                  << "\n\t\t\t d3:" << d3
                  << "\n";
#endif
        }
        catch (std::exception & e) {
            std::cerr << "std::exception!\n";
            std::cerr << e.what ();
            return 99;
        }
        catch (Tools::Exception & e) {
            std::cerr << "SpatialIndex Exception!\n";
            std::cerr << e.what ();
            return 100;
        }
#ifdef DETAILED_STATISTICS
        writeFile.close();
#endif
        if (pause_for_attach)
        {
            std::cerr << "Grand total I/Os = " << grand_total_ios << '\n';
            std::cerr << "Grand total objects = " << grand_total_objs << '\n';
            std::cerr << "I/O per object = "
                      << (float)grand_total_ios / grand_total_objs << '\n';
            std::cerr << "Press Enter to continue\n";
            std::string dummy;
            std::getline (std::cin, dummy);
        }

return 0;
}
