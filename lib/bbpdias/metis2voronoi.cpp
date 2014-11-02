#include <cassert>
#include <fstream>
#include <string>
#include <iostream>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/unordered_map.hpp>

#include <metis.h>

#include <SpatialIndex.h>

#include "libbbpdias/tools.hpp"
#include "libbbpdias/cgal_tools.hpp"
#include "libbtree/btree.hpp"
#include "Rtree.cpp"

#include "delaunay_connectivity.hpp"
#include "ExtSort.cpp"

extern "C" {
	void METIS_PartGraphRecursive(int *nvtxs, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
                              idxtype *adjwgt, int *wgtflag, int *numflag, int *nparts,
                              int *options, int *edgecut, idxtype *part);
	idxtype *idxmalloc(int n, char *msg);
	idxtype *idxsmalloc(int n, idxtype ival, char *msg);
}


typedef uint32_t tetra_id;

typedef uint32_t size_type;

typedef std::list<vertex_id> vertex_list;
typedef std::list<dias::vect> point_list;

size_type page_size = 4096;

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
                if (std::find (v_ptr->second.neighbours.begin (),
                               v_ptr->second.neighbours.end (),
                               t.vertices[j])
                    == v_ptr->second.neighbours.end ())
                    v_ptr->second.neighbours.push_back (t.vertices[j]);
            }

        v_ptr->second.include_in_voronoi_mbr (t.centroid);
    }
}

long bytes = 0;

static SpatialIndex::id_type
serialize_vertices (const dias::vertex_db::const_iterator & v_start,
                    const dias::vertex_db::const_iterator & v_end,
                    SpatialIndex::IStorageManager & storage)
{
    SpatialIndex::id_type result_page = SpatialIndex::StorageManager::NewPage;
    std::vector<byte> serialized;
    serialized.reserve (page_size);
    dias::vertex_db::const_iterator v_ptr = v_start;
    uint32_t vertices_in_page = std::distance (v_start, v_end);
    serialized.resize (sizeof (vertices_in_page));
    byte * s_ptr = &serialized[0];
    dias::serialize (vertices_in_page, &s_ptr);
    assert (s_ptr = &serialized.back ());
    while (v_ptr != v_end)
    {
        v_ptr->second.serialize (v_ptr->first, serialized);
        v_ptr++;
    }

    bytes += serialized.size ();

    assert (serialized.size () <= page_size);

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
        v_id_to_page.insert (v_ptr->first, p);
        v_ptr++;
    }
}

static std::set<SpatialIndex::id_type> crawl_page_ids;

static void output_page_of_vertices(const dias::vertex_db::iterator & v_start,
                                     const dias::vertex_db::iterator & v_end,
                                     SpatialIndex::IStorageManager & storage,
                                     dias::vertex_to_page_id_map & v_id_to_page)
{
    SpatialIndex::id_type new_page = serialize_vertices (v_start, v_end, storage);
    add_id_to_page_mappings (v_id_to_page, v_start, v_end, new_page);
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
        assert (v != vertices.end ()); // Do not comment out this line

        dias::Vertex_info* temp = new dias::Vertex_info();
        temp->vid = v->first;
        temp->vi  = v->second;

        es->insert(temp);

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

static void calculate_shrunk_page_size(std::vector<std::pair<vertex_id,
		dias::vertex_info> >::iterator vbegin, std::vector<std::pair<vertex_id,
		dias::vertex_info> >::iterator vend, int & shrunksize)
{
    std::set<vertex_id> dests;
    std::vector<std::pair<vertex_id,dias::vertex_info> >::iterator itv = vbegin;

    while(itv != vend) {
    	dests.insert((vertex_id)(itv->first));
    	itv++;
    }

    itv = vbegin;
    int pagesize = sizeof(size_type);

    while(itv != vend) {

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

    shrunksize += pagesize;
}

static void calculate_avg_shrunk_page_size (std::map<int,std::vector<std::pair<vertex_id, dias::vertex_info> > > * pmap, float * avg_ps) {

	std::map<int,std::vector<std::pair<vertex_id, dias::vertex_info> > >::iterator it;

	int shrunksize = 0;

	//per partition calculation of sizes
    for( it = pmap->begin(); it != pmap->end(); ++it ) {
    	std::vector< std::pair<vertex_id, dias::vertex_info> > v = (std::vector< std::pair<vertex_id, dias::vertex_info> >)(it->second);
    	calculate_shrunk_page_size(v.begin(), v.end(), shrunksize);
    }

    *avg_ps = shrunksize/pmap->size();
}

void shrink_page(std::vector<std::pair<vertex_id, dias::vertex_info> > & v,
		dias::vertex_db & shrunkpage,
		dias::vertex_db & overflow)
{
	dias::vertex_db page;
    std::set<vertex_id> dests;

    int shrunksize = 0;

    std::vector<std::pair<vertex_id,dias::vertex_info> >::iterator vbegin = v.begin();
    std::vector<std::pair<vertex_id,dias::vertex_info> >::iterator vend = v.end();

    calculate_shrunk_page_size(vbegin, vend, shrunksize);

    while((shrunksize > page_size) && (v.begin() != vend)) {
    	vend--;
    	calculate_shrunk_page_size(v.begin(), vend, shrunksize);
    }

    for(std::vector<std::pair<vertex_id,dias::vertex_info> >::iterator itv = v.begin(); itv != vend; ++itv){
        dests.insert(itv->first);
        page.insert(std::make_pair(itv->first, itv->second));
    }

    //handle the overflow
    while(vend != v.end()) {
    	overflow.insert(std::make_pair(vend->first, vend->second));
    	vend++;
    }

    dias::vertex_db::iterator v_start1 = page.begin();
    dias::vertex_db::iterator v_ptr1 = v_start1;

    while(v_ptr1 != page.end()){
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

static void output_vertices (dias::vertex_db & vertices,
		SpatialIndex::IStorageManager & storage,
		dias::vertex_to_page_id_map & v_id_to_page)
{
	dias::vertex_db::iterator v_ptr = vertices.begin ();

	//do the metis magic
	//translate ids to Fortran-style, first vertex is 1
    std::string id_tid_file_name = "tmpidtidmap";
    boost::shared_ptr<SpatialIndex::IStorageManager> idtidtmp_file (SpatialIndex::StorageManager::createNewDiskStorageManager (id_tid_file_name,page_size));
    idmap_t id_tid_map (idtidtmp_file, page_size);

    long v_id = 0;
    long edges = 0;
    int verts = 0;
    std::set<vertex_id> vs;

    while (v_ptr != vertices.end ())
    {
    	vs.insert(v_ptr->first);
        verts++;

        id_tid_map.insert (v_ptr->first, v_id);
        v_id++;
        v_ptr++;
    }

    v_ptr = vertices.begin ();

    long tedges = 0;

    while (v_ptr != vertices.end ())
    {
    	std::vector<vertex_id> n = ((dias::vertex_info)v_ptr->second).neighbours;

    	BOOST_FOREACH (vertex_id n_id, n)
		{
    		tedges++;
    		std::set<vertex_id>::iterator it = vs.find(n_id);
			if(it != vs.end()) {
				edges++;
			}
		}

        v_ptr++;
    }

    std::cout << "\nTotal edges " << tedges << " inside edges " << edges << std::endl;

    v_ptr = vertices.begin ();

    idxtype * xadj = NULL;
    xadj = idxsmalloc(verts+1, 0, "ReadGraph: xadj");

    idxtype * adjncy = NULL;
    adjncy = idxmalloc(edges, "ReadGraph: adjncy");

    idxtype * vwgt = NULL;
    vwgt = idxmalloc(verts, "ReadGraph: vwgt");

    xadj[0] = 0;

    long cvert = 0;
    long cedg = 0;
    long tweight = 0;

    //transform to CSR format
    while (v_ptr != vertices.end ())
    {
    	dias::vertex_info vi = ((dias::vertex_info)v_ptr->second);
    	std::vector<vertex_id> n = vi.neighbours;

        vwgt[cvert] = vi.serialized_size();
        tweight += vi.serialized_size();

        BOOST_FOREACH (vertex_id n_id, n)
        {
        	std::set<vertex_id>::iterator it = vs.find(n_id);
			if(it != vs.end()) {
				adjncy[cedg] = get_remapped_vertex (id_tid_map, n_id);
        		cedg++;
        	}
        }

        xadj[cvert+1] = cedg;

        cvert++;
        v_ptr++;
    }

    vs.clear();

	//call metis
    int wgtflag = 2;
    int numflag = 0;
    int options[5] = {0};
    int edgecut;
    float compression = 0.6;

    options[0] = 0;
    float avg_ps;
    idxtype * part;

    int nparts = 0;

    std::map<int,std::vector<std::pair<vertex_id, dias::vertex_info> > > pmap;
    std::map<int,std::vector<std::pair<vertex_id, dias::vertex_info> > >::iterator it;

    do {
    	nparts = ceil((tweight/page_size)*compression);

		part = idxmalloc(verts, "main: part");

		METIS_PartGraphRecursive(&verts, xadj, adjncy, vwgt, NULL, &wgtflag, &numflag, &nparts, options, &edgecut, part);

		std::cout << "edge cut " << edgecut << std::endl;

		//partition the vertex infos
		v_ptr = vertices.begin ();

		cvert = 0;
		pmap.clear();

		//group vertices into partitions
		while (v_ptr != vertices.end ())
		{
			int id = part[cvert];

			it = pmap.find(id);

			if(it != pmap.end()) {
				std::vector<std::pair<vertex_id, dias::vertex_info> > v = (std::vector<std::pair<vertex_id, dias::vertex_info> >) it->second;
				v.insert(v.begin(), std::pair<vertex_id, dias::vertex_info>((vertex_id)v_ptr->first, (dias::vertex_info)v_ptr->second));
				pmap.erase(it);
				pmap.insert(std::pair<int,std::vector<std::pair<vertex_id, dias::vertex_info> > >(id, v));
			} else {
				std::vector<std::pair<vertex_id, dias::vertex_info> > v;
				v.insert(v.begin(), std::pair<vertex_id, dias::vertex_info>((vertex_id)v_ptr->first, (dias::vertex_info)v_ptr->second));
				pmap.insert(std::pair<int,std::vector<std::pair<vertex_id, dias::vertex_info> > >(id, v));
			}

			cvert++;
			v_ptr++;
		}

		delete [] part;

		calculate_avg_shrunk_page_size(&pmap, &avg_ps);
		compression += 0.05;

		std::cout << "Average page size (shrunk) " << avg_ps << std::endl;

    } while (avg_ps > page_size);

    //delete only after we are certain we are not going to use metis again for this chunk
    delete [] xadj;
    delete [] adjncy;
    delete [] vwgt;

    long pagecount = 0;

    dias::vertex_db overflow;

    //shrink vertex info page by page
    for( it = pmap.begin(); it != pmap.end(); ++it ) {
    	std::vector< std::pair<vertex_id, dias::vertex_info> > v = (std::vector< std::pair<vertex_id, dias::vertex_info> >)(it->second);

    	dias::vertex_db page;
    	dias::vertex_db shrunkpage;

    	shrink_page(v, shrunkpage, overflow);
    	output_page_of_vertices(shrunkpage.begin(), shrunkpage.end(), storage, v_id_to_page);
    	pagecount++;
    }

    long bsize = sizeof (uint32_t);
    dias::vertex_db page;

    for(dias::vertex_db::iterator vdi = overflow.begin(); vdi != overflow.end(); ++vdi) {
    	dias::vertex_info vi = (dias::vertex_info)vdi->second;
    	vertex_id vid = (vertex_id)vdi->first;

		bsize += vi.serialized_size();

		if(bsize > page_size) {
			pagecount++;
			output_page_of_vertices(page.begin(), page.end(), storage, v_id_to_page);
			page.clear();
			bsize = vi.serialized_size() + sizeof (uint32_t);
			page.insert(std::make_pair (vid, vi));
		} else {
			page.insert(std::make_pair (vid, vi));
		}
    }

    //pagecount++;
    //output_page_of_vertices(page.begin(), page.end(), storage, v_id_to_page);

    std::cout << "bytes per page " << (bytes/pagecount) << std::endl;
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
    vertices.insert (std::make_pair (v_id, v));
    vertex_num++;
    if (vertex_num % vertices_in_chunk == 0) {
        output_vertices (vertices, *disk_file, *v_id_to_page);
    }
}

int main (int argc, char *argv[])
{
    std::string input_fn (argv[1]);
    std::string output_base_fn (argv[2]);
    std::string seed_fn = output_base_fn + "_seed";
    std::string crawl_fn = output_base_fn + "_graph";

    std::string tmp_page_to_id_map_name = output_base_fn + "tmpbtree";

    std::ifstream inp (input_fn.c_str ());

     disk_file.reset (SpatialIndex::StorageManager::createNewDiskStorageManager
                      (crawl_fn, page_size));

     boost::shared_ptr<SpatialIndex::IStorageManager>
         tmp_btree_file (
             SpatialIndex::StorageManager::createNewDiskStorageManager (
                 tmp_page_to_id_map_name, page_size));

     vertex_id v_id = 1;

     v_id_to_page.reset (new dias::vertex_to_page_id_map (tmp_btree_file,
                                                          page_size));

     vertex_list finalized_vertices;


     dias::ExtSort* es = new dias::ExtSort(8*4000000); // for 400MB buckets

     while (!inp.eof ())
     {
         std::string line;
         std::getline (inp, line);

         if (line == "")
         {
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

     std::cout << "Starting Sort" << std::endl;
     es->sort (paginate_sorted_vertices);
     output_vertices (vertices, *disk_file, *v_id_to_page);

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
         assert (length <= page_size);
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
                 dias::map_query_result q_r
                     = v_id_to_page->range_query (std::make_pair (neighbour,
                                                                  neighbour));
 #ifndef NDEBUG
                 if (q_r.size () != 1)
                 {
                     std::cerr << "neighbour = " << neighbour << '\n';
                     std::cerr << "q_r.size () = " << q_r.size () << '\n';
                 }
                 assert (q_r.size () == 1);
                 assert (q_r[0].first == neighbour);
 #endif
                 neighbour = static_cast<vertex_id> (q_r[0].second);
                 dias::serialize (neighbour, &id_ptr);
                 assert (id_ptr == ptr);
             }
             j++;
         }
         disk_file->storeByteArray (i, length, &bytes[0]);
     }

}
