#include <cassert>
#include <fstream>
#include <string>

#include "libbbpdias/tools.hpp"
#include "libbbpdias/cgal_tools.hpp"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>

#include <SpatialIndex.h>

#include "libbtree/btree.hpp"
#include "Rtree.cpp"

#include "delaunay_connectivity.hpp"
#include "ExtSort.cpp"

typedef unsigned tetra_id;

typedef std::list<vertex_id> vertex_list;
typedef std::list<dias::vect> point_list;

struct tetrahedra
{
    vertex_id vertices[4];
    dias::vect centroid;

    tetrahedra (vertex_id v[], const vertex_db & vertex_store)
        {
            vertices[0] = v[0];
            vertices[1] = v[1];
            vertices[2] = v[2];
            vertices[3] = v[3];

            dias::vect a = (vertex_store.find (v[0]))->second.coords;
            dias::vect b = (vertex_store.find (v[1]))->second.coords;
            dias::vect c = (vertex_store.find (v[2]))->second.coords;
            dias::vect d = (vertex_store.find (v[3]))->second.coords;

            dias::tetcircumcenter(a, b, c, d, centroid);
#ifndef NDEBUG
            std::cerr << "centroid = " << centroid << '\n';
            std::cerr << "for: " << v[0] << ' ' << v[1] << ' ' << v[2] << ' '
                      << v[3] << '\n';
#endif
        }
};

typedef std::map<tetra_id, tetrahedra> tetrahedra_db;
typedef std::pair<tetra_id, tetrahedra> tetrahedra_db_value;

static void read_vertex (boost::tokenizer<>::iterator & p,
                         const boost::tokenizer<>::iterator & end,
                         vertex_db & vertices,
                         vertex_id & v_id, int & v_count, dias::Rtree* sTree)
{
    dias::vect vertex;
    int j = 0;
    while (p != end)
    {
        std::string val_str = *p++;
        float val = boost::lexical_cast<float> (val_str);
        vertex[j++] = val;
    }
    assert (j == 3);

    dias::Data temp;
    temp.id = v_id;
    temp.midPoint[0] = vertex[0];
    temp.midPoint[1] = vertex[1];
    temp.midPoint[2] = vertex[2];

    sTree->Insert(temp);

    vertices.insert (std::make_pair (v_id++, vertex_info (vertex)));
    v_count++;
}

static tetrahedra read_tetrahedra (boost::tokenizer<>::iterator & p,
                                   const boost::tokenizer<>::iterator & end,
                                   std::list<vertex_id> & finalized_vertices,
                                   const vertex_db & vertices, int v_count)
{
    vertex_id v_ids[4];
    int j = 0;
    while (p != end)
    {
        std::string v_str = *p++;
        int v = boost::lexical_cast<int> (v_str);
        if (v >= 0)
            v_ids[j] = v;
        else
        {
            v_ids[j] = v_count + v + 1;
            finalized_vertices.push_back (v_ids[j]);
        }
        j++;
    }
    assert (j == 4);

    return tetrahedra (v_ids, vertices);
}

static void add_neighbours (const tetrahedra & t, vertex_db & v)
{
    for (int i = 0; i < 4; i++)
    {
        vertex_db::iterator v_ptr = v.find (t.vertices[i]);
        assert (v_ptr != v.end ());

        for (int j = 0; j < 4; j++)
            if (i != j)
                v_ptr->second.neighbours.push_back (t.vertices[j]);

        v_ptr->second.include_in_voronoi_mbr (t.centroid);
    }
}

#if 0
static SpatialIndex::id_type
serialize_vertices (const vertex_db::const_iterator & v_start,
                    const vertex_db::const_iterator & v_end,
                    SpatialIndex::IStorageManager & storage)
{
    SpatialIndex::id_type result_page = SpatialIndex::StorageManager::NewPage;
    std::vector<byte> serialized;
    serialized.reserve (page_size);
    vertex_db::const_iterator v_ptr = v_start;
    unsigned vertices_in_page = std::distance (v_start, v_end);
    serialized.resize (sizeof (vertices_in_page));
    byte * s_ptr = &serialized[0];
    serialize (vertices_in_page, &s_ptr);
    assert (s_ptr = &serialized.back ());
    while (v_ptr != v_end)
    {
#ifndef NDEBUG
        std::cerr << "Vertex id = " << v_ptr->first << ' ';
        std::cerr << v_ptr->second << '\n';
#endif
        v_ptr->second.serialize (v_ptr->first, serialized);
        v_ptr++;
    }
    assert (serialized.size () <= page_size);
    storage.storeByteArray (result_page, serialized.size (), &serialized[0]);
    return result_page;
}

static void
add_id_to_page_mappings (vertex_to_page_id_map & v_id_to_page,
                         const vertex_db::const_iterator & v_start,
                         const vertex_db::const_iterator & v_end,
                         SpatialIndex::id_type p)
{
    vertex_db::const_iterator v_ptr = v_start;
    while (v_ptr != v_end)
    {
#ifndef NDEBUG
        std::cerr << "Adding a mapping from vertex id = " << v_ptr->first
                  << " to page id = " << p << '\n';
#endif
        v_id_to_page.insert (v_ptr->first, p);
        v_ptr++;
    }
}

static void output_page_of_vertices (const vertex_db::iterator & v_start,
                                     const vertex_db::iterator & v_end,
                                     vertex_db & vertices,
                                     SpatialIndex::IStorageManager & storage,
                                     vertex_to_page_id_map & v_id_to_page)
{
    SpatialIndex::id_type new_page = serialize_vertices (v_start, v_end,
                                                         storage);
    add_id_to_page_mappings (v_id_to_page, v_start, v_end, new_page);
    vertices.erase (v_start, v_end);
}
#endif

static void output_finalized_vertices_for_sorting (
    vertex_list & finalized_vertices,
    vertex_db & vertices,ExtSort* es)
{
    vertex_list::iterator v_ptr = finalized_vertices.begin ();
    while (v_ptr != finalized_vertices.end ())
    {
        vertex_db::iterator v = vertices.find (*v_ptr);
        assert (v != vertices.end ()); // Do not comment out this line

        ///

        Vertex_info* temp = new Vertex_info();
        temp->vid = v->first;
        temp->vi  = v->second;
#ifndef NDEBUG
        std::cout << "\ninserting: "<< temp->vid << " :" << temp->vi.coords[0] << "," << temp->vi.coords[1] << temp->vi.coords[2];
#endif

        es->insert(temp);

//        v->second.output_in_binary (*v_ptr, YOUR_STREAM_HERE);
        vertices.erase (v);
        v_ptr++;
    }
    finalized_vertices.clear ();
}

typedef dias::btree<int> idmap_t;

int get_remapped_vertex (const idmap_t & m, vertex_id id)
{
    idmap_t::query_answer r = m.range_query (std::make_pair (id, id));
    assert (r.size () == 1);
    assert (r[0].first == id);
    return r[0].second;
}

static void output_vertices (vertex_db & vertices)
{
    std::string tmp_file_name = "tmpidmap";
    boost::shared_ptr<SpatialIndex::IStorageManager>
        tmp_file (SpatialIndex::StorageManager::createNewDiskStorageManager (
                      tmp_file_name,page_size));
    idmap_t idmap (tmp_file, page_size);

    vertex_db::iterator v_start = vertices.begin ();
    vertex_db::iterator v_ptr = v_start;

    ofstream myfile;
    myfile.open ("example.txt");

    long v_id = 1;
    long edges = 0;
//    map<vertex_id,long> idmap;

    while (v_ptr != vertices.end ())
    {
        edges += ((vertex_info)v_ptr->second).neighbours.size ();

//	idmap.insert(pair<vertex_id,long>((vertex_id)v_ptr->first,v_id));
        idmap.insert (v_ptr->first, v_id);
	v_id++;
        v_ptr++;
    }
   
    v_id--; 
    ldiv_t divresult = ldiv(edges,2);
    myfile << v_id << " " << divresult.quot << " 0\n";

    v_start = vertices.begin ();
    v_ptr = v_start;
    v_id = 1;
  
    while (v_ptr != vertices.end ())
    {
        std::vector<vertex_id> n = ((vertex_info)v_ptr->second).neighbours;
        vertex_id id = (vertex_id)v_ptr->first;

//                map<vertex_id,long>::iterator it;

//        it = idmap.find(id);
        std::cout << get_remapped_vertex (idmap, id) << " : ";
        //std::cout << it->second << " : ";

        BOOST_FOREACH (vertex_id n_id, n)
        {
//                it = idmap.find(n_id);
                myfile << get_remapped_vertex (idmap, n_id) << " ";
        }

        myfile << endl;

        v_id++;
        v_ptr++;
    }

    myfile.close();
}

static void finalize_all_vertices (vertex_list & finalized_vertices,
                                   vertex_db & vertices)
{
    BOOST_FOREACH (vertex_id v, finalized_vertices)
    {
        vertices.erase (v);
    }
    finalized_vertices.clear ();
    BOOST_FOREACH (vertex_db_pair v_pair, vertices)
    {
        finalized_vertices.push_back (v_pair.first);
    }
}

enum { vertices_in_chunk = 10000000 };

static int vertex_num = 0;

static vertex_db vertices;

boost::shared_ptr<vertex_to_page_id_map> v_id_to_page;

boost::shared_ptr<SpatialIndex::IStorageManager> disk_file;

std::ofstream outp;

bool do_outp;

void paginate_sorted_vertices (vertex_id v_id, const struct vertex_info & v)
{
#ifndef NDEBUG
    std::cerr << "Hello v_id = " << v_id << " vertex_info = " << v << '\n';
#endif
    vertices.insert (std::make_pair (v_id, v));
    vertex_num++;
    if (vertex_num % vertices_in_chunk == 0)
        output_vertices (vertices);
}

int main (int argc, char *argv[])
{
    assert (argc == 3 || argc == 4);

    std::string input_fn (argv[1]);
    std::string output_base_fn (argv[2]);
    std::string seed_fn = output_base_fn + "_seed";
    std::string crawl_fn = output_base_fn + "_graph";

    if (argc == 4)
    {
        std::string output_fn (argv[3]);
        outp.open (output_fn.c_str ());
        do_outp = true;
    }
    else
        do_outp = false;

    std::ifstream inp (input_fn.c_str ());
    dias::Rtree* seedTree = new dias::Rtree(seed_fn,true);

    disk_file.reset (SpatialIndex::StorageManager::createNewDiskStorageManager
                     (crawl_fn, page_size));

    vertex_id v_id = 1;

    // TODO: almost the same as vertex_id.  Comes from the svreader_sva.cpp
    int v_count = 0;

    SpatialIndex::id_type header_page = SpatialIndex::StorageManager::NewPage;
    std::vector<byte> serialized_header (page_size, 0);
    disk_file->storeByteArray (header_page, serialized_header.size (),
                              &serialized_header[0]);

    v_id_to_page.reset (new vertex_to_page_id_map (disk_file, page_size));

    vertex_list finalized_vertices;


    ExtSort* es = new ExtSort(400000);

    while (!inp.eof ())
    {
        std::string line;
        std::getline (inp, line);

        if (line == "")
        {
#ifndef NDEBUG
            std::cerr << "Empty line encountered!\n";
#endif
            continue;
        }

        assert (line != "");

        boost::char_delimiters_separator<char> sep (false, "", 0);
        boost::tokenizer<> tokens (line, sep);
        boost::tokenizer<>::iterator p = tokens.begin();
        std::string type = *p++;

        if (type == "#")
            ;
        else if (type == "v")
        {
            read_vertex (p, tokens.end(), vertices, v_id, v_count,seedTree);
        }
        else if (type == "c")
        {
            tetrahedra t = read_tetrahedra (p, tokens.end(),
                                            finalized_vertices, vertices,
                                            v_count);
            add_neighbours (t, vertices);
//            output_finalized_vertices (finalized_vertices, vertices,
//                                       *disk_file, v_id_to_page, outp,
//                                       do_outp, false);
            output_finalized_vertices_for_sorting (finalized_vertices,
                                                   vertices, es);

        }
        else
            assert (false);
    }
    std::cerr << "Total vertices read = " << v_id - 1 << '\n';

    finalize_all_vertices (finalized_vertices, vertices);
    output_finalized_vertices_for_sorting (finalized_vertices, vertices, es);

    std::cout << "Starting Sort\n";
    es->sort (paginate_sorted_vertices);
    std::cout << "Finished sorting\n";
    output_vertices (vertices);

    delete es;

    std::cerr << "Total vertices written = " << vertex_num << '\n';

    v_id_to_page->write_header (header_page);

    if (argc == 3)
        outp.close ();

    assert (header_page == 0);
    std::cerr << "Header page id = " << header_page << '\n';
    delete seedTree;
}
