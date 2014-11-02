#include <cassert>
#include <inttypes.h>
#include <fstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/unordered_map.hpp>

#include <SpatialIndex.h>

#include "libbbpdias/tools.hpp"
#include "libbbpdias/cgal_tools.hpp"
#include "libbbpdias/spatialindex_tools.hpp"
#include "libbtree/btree.hpp"
#include "Rtree.cpp"

#include "delaunay_connectivity.hpp"
#include "ExtSort.cpp"

typedef uint32_t tetra_id;

typedef uint32_t size_type;

typedef std::list<vertex_id> vertex_list;
typedef std::list<dias::vect> point_list;

struct tetrahedra
{
    vertex_id vertices[4];
    dias::vect centroid;

    tetrahedra (vertex_id v[], const dias::vertex_db & vertex_store)
        {
            vertices[0] = v[0];
            vertices[1] = v[1];
            vertices[2] = v[2];
            vertices[3] = v[3];

            dias::vect a = (vertex_store.find (v[0]))->second.coords;
            dias::vect b = (vertex_store.find (v[1]))->second.coords;
            dias::vect c = (vertex_store.find (v[2]))->second.coords;
            dias::vect d = (vertex_store.find (v[3]))->second.coords;

            dias::tetcircumcenter (a, b, c, d, centroid);
        }
};

typedef boost::unordered_map<tetra_id, tetrahedra> tetrahedra_db;
typedef std::pair<tetra_id, tetrahedra> tetrahedra_db_value;

static void read_vertex (boost::tokenizer<>::iterator & p,
                         const boost::tokenizer<>::iterator & end,
                         dias::vertex_db & vertices,
                         vertex_id & v_id)
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

    vertices.insert (std::make_pair (v_id++, dias::vertex_info (vertex)));
}

static tetrahedra read_tetrahedra (boost::tokenizer<>::iterator & p,
                                   const boost::tokenizer<>::iterator & end,
                                   std::list<vertex_id> & finalized_vertices,
                                   const dias::vertex_db & vertices,
                                   int v_count)
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
            v_ids[j] = v_count + v;
            finalized_vertices.push_back (v_ids[j]);
        }
        j++;
    }
    assert (j == 4);

    return tetrahedra (v_ids, vertices);
}

static void add_neighbours (const tetrahedra & t, dias::vertex_db & v)
{
    for (int i = 0; i < 4; i++)
    {
        dias::vertex_db::iterator v_ptr = v.find (t.vertices[i]);
        assert (v_ptr != v.end ());

        for (int j = 0; j < 4; j++)
            if (i != j)
            {
// Inefficient with vector
                if (std::find (v_ptr->second.neighbours.begin (),
                               v_ptr->second.neighbours.end (),
                               t.vertices[j])
                    == v_ptr->second.neighbours.end ())
                    v_ptr->second.neighbours.push_back (t.vertices[j]);
            }

        v_ptr->second.include_in_voronoi_mbr (t.centroid);
    }
}

static SpatialIndex::id_type
serialize_vertices (const dias::vertex_db::const_iterator & v_start,
                    const dias::vertex_db::const_iterator & v_end,
                    SpatialIndex::IStorageManager & storage)
{
    SpatialIndex::id_type result_page = SpatialIndex::StorageManager::NewPage;
    std::vector<byte> serialized;
    serialized.reserve (dias::default_page_size);
    dias::vertex_db::const_iterator v_ptr = v_start;
    uint32_t vertices_in_page = std::distance (v_start, v_end);
    serialized.resize (sizeof (vertices_in_page));
    byte * s_ptr = &serialized[0];
    dias::serialize (vertices_in_page, &s_ptr);
    assert (s_ptr == &serialized.back () + 1);
#ifndef NDEBUG
    std::cerr << "serializing vertices = " << vertices_in_page << '\n';
    std::cerr << "serialize_vertices, serialized.size () = "
              << serialized.size () << '\n';
    size_t serialized_bytes = sizeof (vertices_in_page);
#endif
    assert (serialized_bytes == serialized.size ());
    while (v_ptr != v_end)
    {
#ifdef VERBOSE
        std::cerr << "Vertex id = " << v_ptr->first << ' ';
        std::cerr << v_ptr->second << '\n';
#endif
        v_ptr->second.serialize (v_ptr->first, serialized);
#ifndef NDEBUG
        serialized_bytes += v_ptr->second.serialized_size ();
        if (serialized_bytes != serialized.size ())
        {
            std::cerr << "serialized_bytes = " << serialized_bytes << '\n';
            std::cerr << "serialized.size () = " << serialized.size () << '\n';
        }
        assert (serialized_bytes == serialized.size ());
        std::cerr << "vtx size = " << v_ptr->second.serialized_size () << '\n';
        std::cerr << "serialized.size () = " << serialized.size () << '\n';
#endif
        v_ptr++;
    }

    if (serialized.size () > dias::default_page_size)
    {
        std::cerr << "serialized.size () = " << serialized.size () << '\n';
        std::cerr << "page_size = " << dias::default_page_size << '\n';
        assert (serialized.size () <= dias::default_page_size);
    }
    storage.storeByteArray (result_page, serialized.size (), &serialized[0]);
    return result_page;
}

static void
add_id_to_page_mappings (dias::vertex_to_page_id_map & v_id_to_page,
                         const dias::vertex_db::const_iterator & v_start,
                         const dias::vertex_db::const_iterator & v_end,
                         SpatialIndex::id_type p)
{
    dias::vertex_db::const_iterator v_ptr = v_start;
    while (v_ptr != v_end)
    {
#ifdef VERBOSE
        std::cerr << "Adding a mapping from vertex id = " << v_ptr->first
                  << " to page id = " << p << '\n';
#endif
        v_id_to_page.insert (v_ptr->first, p);
        v_ptr++;
    }
}

void shrink_page(dias::vertex_db::iterator begin, dias::vertex_db::iterator end, dias::vertex_db & shrunkpage)
{
    std::set<vertex_id> dests;

    for(dias::vertex_db::iterator itv = begin; itv != end; ++itv) {
        dests.insert(itv->first);
    }

    dias::vertex_db::iterator v_start1 = begin;
    dias::vertex_db::iterator v_ptr1 = v_start1;

    while(v_ptr1 != end){
        dias::vertex_info vi = ((dias::vertex_info)(v_ptr1->second));
        std::vector<vertex_id> *ns = &vi.neighbours;
        std::vector<vertex_id>::iterator vit = ns->begin();
        while(vit != ns->end()){
            std::set<vertex_id>::iterator it = dests.find(*vit);
            if(it != dests.end()){
                vit = ns->erase(vit);
            }else{
                ++vit;
            }
        }

        shrunkpage.insert(std::make_pair(v_ptr1->first, vi));
        v_ptr1++;
    }
}

static std::set<SpatialIndex::id_type> crawl_page_ids;

static void
output_page_of_vertices (const dias::vertex_db::iterator & v_start,
                         const dias::vertex_db::iterator & v_end,
                         dias::vertex_db & vertices,
                         SpatialIndex::IStorageManager & storage,
                         dias::vertex_to_page_id_map & v_id_to_page)
{
	dias::vertex_db shrunkpage;

	shrink_page(v_start, v_end, shrunkpage);

    SpatialIndex::id_type new_page = serialize_vertices (shrunkpage.begin(), shrunkpage.end(), storage);
    add_id_to_page_mappings (v_id_to_page, v_start, v_end, new_page);
    vertices.erase (v_start, v_end);
    crawl_page_ids.insert (new_page);
}

static void output_finalized_vertices_for_sorting (
    vertex_list & finalized_vertices,
    dias::vertex_db & vertices,
    dias::ExtSort* es)
{
    vertex_list::iterator v_ptr = finalized_vertices.begin ();
    while (v_ptr != finalized_vertices.end ())
    {
        dias::vertex_db::iterator v = vertices.find (*v_ptr);
        assert (v != vertices.end ());

        dias::Vertex_info* temp = new dias::Vertex_info();
        temp->vid = v->first;
        temp->vi  = v->second;
#ifdef VERBOSE
        std::cout << "\ninserting: "<< temp->vid << " :" << temp->vi.coords[0]
                  << "," << temp->vi.coords[1] << temp->vi.coords[2];
#endif

        es->insert(temp);

        vertices.erase (v);
        v_ptr++;
    }
    finalized_vertices.clear ();
}

static int calculate_shrunk_page_size(dias::vertex_db::iterator begin, dias::vertex_db::iterator end)
{
    std::set<vertex_id> dests;

    for(dias::vertex_db::iterator itv = begin; itv != end; ++itv) {
        dests.insert(itv->first);
    }

    dias::vertex_db::iterator itv = begin;

    int pagesize = sizeof(size_type);

    while(itv != end) {

    	dias::vertex_info vi = (dias::vertex_info)(itv->second);
        pagesize += vi.serialized_header_size();

        std::vector<vertex_id> *ns = &vi.neighbours;
        std::vector<vertex_id>::iterator vit = ns->begin();

        while(vit != ns->end()){
            std::set<vertex_id>::iterator it = dests.find(*vit);
            if(it == dests.end()){
                pagesize += sizeof (vertex_id);
            }
            ++vit;
        }

        itv++;
    }

    return pagesize;
}

static void
output_vertices (dias::vertex_db & vertices,
                 SpatialIndex::IStorageManager & storage,
                 dias::vertex_to_page_id_map & v_id_to_page,
                 std::ostream & outp, bool do_outp,
                 bool force_remainder)
{
    dias::vertex_db::iterator v_start = vertices.begin ();
    dias::vertex_db::iterator v_ptr = v_start;
    dias::vertex_db::iterator tmp = v_start;

    while (v_ptr != vertices.end ())
    {
        if (calculate_shrunk_page_size(v_start, v_ptr) > dias::default_page_size)
        {
            output_page_of_vertices (v_start, tmp, vertices, storage, v_id_to_page);
            v_start = tmp;
        }

        tmp = v_ptr;

        v_ptr++;
    }

    if (force_remainder)
    {
        output_page_of_vertices (v_start, v_ptr,  vertices, storage, v_id_to_page);
    }
}

static void finalize_all_vertices (vertex_list & finalized_vertices,
                                   dias::vertex_db & vertices)
{
    BOOST_FOREACH (vertex_id v, finalized_vertices)
    {
        vertices.erase (v);
    }
    finalized_vertices.clear ();
    BOOST_FOREACH (dias::vertex_db_pair v_pair, vertices)
    {
        finalized_vertices.push_back (v_pair.first);
    }
}

enum { vertices_in_chunk = 500000 };

static vertex_id vertex_num = 0;

static dias::vertex_db vertices;

boost::shared_ptr<dias::vertex_to_page_id_map> v_id_to_page;

boost::shared_ptr<SpatialIndex::IStorageManager> disk_file;

std::ofstream outp;

bool do_outp;

void paginate_sorted_vertices (vertex_id v_id,
                               const struct dias::vertex_info & v)
{
#ifdef VERBOSE
    std::cerr << "Hello v_id = " << v_id << " vertex_info = " << v << '\n';
#endif
    vertices.insert (std::make_pair (v_id, v));
    vertex_num++;
    if (vertex_num % vertices_in_chunk == 0)
    {
        output_vertices (vertices, *disk_file, *v_id_to_page, outp, do_outp,
                         false);
    }
}

std::map<vertex_id, vertex_id> vertex_to_page;

vertex_id get_page_id (vertex_id neighbour)
{
	vertex_id r;

	std::map<vertex_id, vertex_id>::iterator it = vertex_to_page.find(neighbour);

	if(it == vertex_to_page.end()) {
		dias::map_query_result q_r = v_id_to_page->range_query (std::make_pair (neighbour, neighbour));

		r = static_cast<vertex_id> (q_r[0].second);

		if(vertex_to_page.size() > 1000) {
			vertex_to_page.clear();
		}

		vertex_to_page.insert(std::make_pair(neighbour, r));
	} else {
		r = it->second;
	}

    return r;
}

int main (int argc, char *argv[])
{
    assert (argc == 3 || argc == 4);

    std::string input_fn (argv[1]);
    std::string output_base_fn (argv[2]);
    std::string seed_fn = output_base_fn + "_seed";
    std::string crawl_fn = output_base_fn + "_graph";

    std::string tmp_page_to_id_map_name = output_base_fn + "tmpbtree";

    if (argc == 4)
    {
        std::string output_fn (argv[3]);
        outp.open (output_fn.c_str ());
        do_outp = true;
    }
    else
        do_outp = false;

    std::ifstream inp (input_fn.c_str ());
    disk_file.reset (SpatialIndex::StorageManager::createNewDiskStorageManager
                     (crawl_fn, dias::default_page_size));

    boost::shared_ptr<SpatialIndex::IStorageManager>
        tmp_btree_file (
            SpatialIndex::StorageManager::createNewDiskStorageManager (
                tmp_page_to_id_map_name, dias::default_page_size));

    vertex_id v_id = 1;

    v_id_to_page.reset (new dias::vertex_to_page_id_map (
                            tmp_btree_file,
                            dias::default_page_size));

    vertex_list finalized_vertices;

    dias::ExtSort* es = new dias::ExtSort(4000000); // for 400MB buckets

    while (!inp.eof ())
    {
        std::string line;
        std::getline (inp, line);

        if (line == "")
        {
#ifdef VERBOSE
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
            read_vertex (p, tokens.end(), vertices, v_id);
        }
        else if (type == "c")
        {
            tetrahedra t = read_tetrahedra (p, tokens.end(),
                                            finalized_vertices, vertices,
                                            v_id);
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

    std::cout << "Starting Sort";
    es->sort (paginate_sorted_vertices);

    output_vertices (vertices, *disk_file, *v_id_to_page, outp, do_outp, true);

    delete es;

    std::cerr << "Total disk pages written = " << crawl_page_ids.size ()
              << '\n';
    std::cerr << "Total vertices written = " << vertex_num << '\n';

    if (argc == 3)
        outp.close ();

    boost::scoped_ptr<dias::Rtree> seed_tree (new dias::Rtree(seed_fn, true));

    std::cerr << "2nd pass, replacing vertex ids with "
        "page ids and building seed index...\n";

    BOOST_FOREACH (SpatialIndex::id_type i, crawl_page_ids)
    {
        size_t length;
        boost::shared_array<byte> bytes
            = dias::read_byte_array (*disk_file, i, length);
        assert (length <= dias::default_page_size);
#ifdef VERBOSE
        std::cerr << "Page id = " << i << ", length = " << length << '\n';
#endif
        byte * ptr = bytes.get ();
        uint32_t vertices_in_page;
        dias::unserialize (vertices_in_page, &ptr);
        SpatialIndex::id_type j = 0;
        while (j < vertices_in_page)
        {
            byte * start = ptr;
            dias::vertex_info::size_type current_size;
            dias::unserialize (current_size, &ptr);
#ifdef VERBOSE
            std::cerr << "Vertex size = " << current_size << '\n';
#endif
            ptr += sizeof (vertex_id);
            dias::vect pos = dias::unserialize_vect_as_float (&ptr);
            ptr += dias::mbr::serialized_size_in_float ();

            dias::Data temp;
            temp.id = i;
            temp.midPoint[0] = pos[0];
            temp.midPoint[1] = pos[1];
            temp.midPoint[2] = pos[2];

            seed_tree->Insert (temp);

            while (ptr - start < (ptrdiff_t)current_size)
            {
                byte * id_ptr = ptr;
                vertex_id neighbour;
                dias::unserialize (neighbour, &ptr);

                neighbour = get_page_id(neighbour);

                dias::serialize (neighbour, &id_ptr);
                assert (id_ptr == ptr);
            }
            j++;
        }
        disk_file->storeByteArray (i, length, &bytes[0]);
    }
}

