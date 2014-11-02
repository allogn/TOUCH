#include <cassert>
#include <cstring>
#include <fstream>
#include <string>

#define PROFILING
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <SpatialIndex.h>

#include "Query.cpp"
#include "Rtree.cpp"
#include "QueryGenerator.cpp"
#include "cpu_timer.cpp"

#ifdef PROFILING
tpie::cpu_timer loadingc,a1,a2,totalc,getvertexc,visittimec,candidatec,readverticespagec,getvertexpagec,getfromcachec,c1,c2,p1,p2,p3,p4;
#endif

#include "delaunay_connectivity.hpp"



static const SpatialIndex::id_type header_page_id = 0;

typedef std::vector<dias::Query> query_vect;
typedef boost::unordered_set<SpatialIndex::id_type> page_id_set;

typedef boost::unordered_map<vertex_id, SpatialIndex::id_type>
vertex_to_page_id_map_cache;




static SpatialIndex::id_type
get_vertex_page (vertex_id id, const vertex_to_page_id_map & v_id_to_page,
                 vertex_to_page_id_map_cache & v_id_to_page_cache)
{
    vertex_to_page_id_map_cache::const_iterator in_cache
        = v_id_to_page_cache.find (id);
    if (in_cache != v_id_to_page_cache.end ())
        return in_cache->second;

    map_query_result q_r = v_id_to_page.range_query (std::make_pair (id, id));
    assert (q_r.size () == 1);
    assert (q_r[0].first == id);

    v_id_to_page_cache.insert (q_r[0]);
    return q_r[0].second;
}

static const struct vertex_info &
get_vertex (vertex_id id, vertex_to_page_id_map & v_id_to_page,
            SpatialIndex::IStorageManager & disk_file, vertex_db & vertices,
            vertex_to_page_id_map_cache & v_id_to_page_cache,
            unsigned & crawl_ios)
{
#ifdef PROFILING
	getvertexc.start();
	getfromcachec.start();
#endif
#ifndef NDEBUG
    std::cerr << "Getting vertex id = " << id;
#endif

    vertex_db::const_iterator existing_vertex = vertices.find (id);

    if (existing_vertex != vertices.end ())
    {
#ifdef PROFILING
	getfromcachec.stop();
	getvertexc.stop();
#endif
#ifndef NDEBUG
        std::cerr << " (already read)\n";
#endif
        return existing_vertex->second;
    }
#ifdef PROFILING
	getfromcachec.stop();
	getvertexpagec.start();
#endif
    SpatialIndex::id_type page_id = get_vertex_page (id, v_id_to_page,
                                                     v_id_to_page_cache);

#ifndef NDEBUG
    std::cerr << " from page id = " << page_id << '\n';
#endif

    crawl_ios++;
#ifdef PROFILING
    getvertexpagec.stop();
	readverticespagec.start();
#endif
    read_all_vertices_from_page (disk_file, page_id, vertices);
#ifdef PROFILING
    readverticespagec.stop();
#endif
    existing_vertex = vertices.find (id);
    assert (existing_vertex != vertices.end ());
#ifdef PROFILING
    getvertexc.stop();
#endif
    return existing_vertex->second;
}

static vertex_to_page_id_map
load_vertex_to_page_id_map (boost::shared_ptr<SpatialIndex::IStorageManager>
                            storage)
{
    size_t length;
    boost::shared_array<byte> bytes
        = dias::read_byte_array (*storage, header_page_id, length);
    byte * c_bytes = bytes.get();

    SpatialIndex::id_type btree_root_id;
    dias::unserialize (btree_root_id, &c_bytes);

    return vertex_to_page_id_map (header_page_id, storage);
}

int main (int argc, char* argv[])
{
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><>< ARGUMENTS ><><><><><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    if (argc != 3)
    {
        std::cerr << "Wrong number of arguments!\n";
        return 1;
    }
    std::string input_base_fn (argv[1]);
    std::string seed_fn = input_base_fn + "_seed";
    std::string crawl_fn = input_base_fn + "_graph";
    std::string query_fn (argv[2]);

	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><<>< LOADING Files ><<><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef PROFILING
loadingc.start();
#endif
    std::cerr << "Opening seed index file " << seed_fn << " ... ";
    boost::shared_ptr<dias::Rtree> seed_index (new dias::Rtree (seed_fn, false));
    std::cerr << "OK\n";
    std::cerr << "Opening crawl file " << crawl_fn << " ... ";
    boost::shared_ptr<SpatialIndex::IStorageManager> crawl_file
        (SpatialIndex::StorageManager::loadDiskStorageManager (crawl_fn));
    std::cerr << "OK\n";
    dias::QueryGenerator qg;
    qg.Load (query_fn);
    vertex_to_page_id_map v_id_to_page = load_vertex_to_page_id_map (crawl_file);    //TODO Should find some other solution other than Btree
#ifdef PROFILING
loadingc.stop();
totalc.start();
#endif
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><<>< For Each Query ><><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    BOOST_FOREACH (dias::Query q, qg.queries)
    {
    	// Variables for Performance Statistics
        int PointsConsidered = 0;
        int QueryResult = 0;
        unsigned crawl_ios = 0;
        unsigned seed_ios =0;
        tpie::cpu_timer seedtimer,crawltimer;

        // Containers
        vertex_to_page_id_map_cache v_id_to_page_cache;  // TODO I think its unavoidable at the moment but we shud think about it
        vertex_db vertices;             				 // TODO avoid we are already storing info in Page cache
        std::set<vertex_id> visited;							 // TODO Should be as much optimized as possible..
        std::queue<vertex_id> BFSque;

        // <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
        // <><><><><><><><><><><><><<<><>>< SEEDING ><<<>><><><><><><><><><><><
        // <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
        std::cerr << "Query: " << q << '\n';
        std::cerr << "Querying seed index... ";

        seedtimer.start();
        seed_index->Seed (q);
        seedtimer.stop();
        seed_ios =q.nodeAccesses;

        if (q.points == 0)
            std::cerr << "empty query result\n";
        else
        {
            BFSque.push (q.seed.id);
            visited.insert (q.seed.id);
        }

		// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
		// <><><><><><><><><><><><><<<><>><>< BFS ><<><<>><><><><><><><><><><><><><
		// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
        crawltimer.start();
        while (!BFSque.empty ())
        {

			// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
			// <><><><><><><><><><><><><<<><>><>< VISIT <><<>><><><><><><><><><><><><><
			// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef PROFILING
visittimec.start();
#endif
            vertex_id v = BFSque.front ();
            BFSque.pop ();
            PointsConsidered++;
#ifdef PROFILING
a1.start();
#endif
            struct vertex_info vtx = get_vertex (v, v_id_to_page, *crawl_file,
                                                 vertices,v_id_to_page_cache,
                                                 crawl_ios);
#ifdef PROFILING
a1.stop();
#endif
            dias::Vertex vtx_coords (vtx.coords[0], vtx.coords[1], vtx.coords[2]);
            if (dias::Box::enclose(q.box, vtx_coords))
            {
                QueryResult++;
            }

#ifdef PROFILING
visittimec.stop();
#endif

			// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
			// <><><><><><><><><><><><><< CANDIDATE FOR BFS <><><><><><><><><><><><><><
			// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
            BOOST_FOREACH (vertex_id n, vtx.neighbours)
            {
#ifdef PROFILING
candidatec.start();
p1.start();
#endif
			bool notfound = (visited.find(n)==visited.end());
#ifdef PROFILING
p1.stop();
#endif
                if (notfound)
                {
#ifdef PROFILING
p2.start();
#endif
                    struct vertex_info n_vtx = get_vertex (n, v_id_to_page,
                                                           *crawl_file,
                                                           vertices,v_id_to_page_cache,
                                                           crawl_ios);
#ifdef PROFILING
p2.stop();
p3.start();
#endif

                    visited.insert (n);

                    if (dias::Box::overlap(n_vtx.voronoi_mbr.as_box (), q.box))
                        BFSque.push (n);

#ifdef PROFILING
p3.stop();
#endif
                }

#ifdef PROFILING
candidatec.stop();
#endif
            }
        }
        crawltimer.stop();
        std::cout << QueryResult << "," << (PointsConsidered-QueryResult) << "," << (vertices.size()-PointsConsidered) << "," << seed_ios << "," << seedtimer << "," << crawl_ios << "," << crawltimer << std::endl;
    }

#ifdef PROFILING
totalc.stop();
#endif

#ifdef PROFILING
std::cout << "\nPROFILING:"
	      << "\n\tLoading:" << loadingc
	      << "\n\tTOTAL time:" << totalc
          << "\n\tVisiting Top of Queue:" << visittimec
          << "\n\t\t a1:" << a1
          << "\n\tChecking Candidates:" << candidatec
          << "\n\t\t p1:" << p1
          << "\n\t\t p2:" << p2
          << "\n\t\t p3:" << p3
          << "\n"
          << "\n\tGet Vertex Function:"<< getvertexc
          << "\n\t\t Get from cache:" << getfromcachec
          << "\n\t\t Get PageId:" << getvertexpagec
          << "\n\t\t Read Page:" << readverticespagec
          << "\n\t\t\t c1:" << c1
          << "\n\t\t\t c2:" << c2
          << "\n";
#endif

    return 0;
}
