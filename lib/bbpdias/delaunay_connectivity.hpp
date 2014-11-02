#ifndef DELAUNAY_CONNECTIVITY_HPP_
#define DELAUNAY_CONNECTIVITY_HPP_

#include <cstring>
#include <stdint.h>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "libbtree/btree.hpp"
#include "libbbpdias/tools.hpp"
#include "libbbpdias/mbr.hpp"
#include "libbbpdias/spatialindex_tools.hpp"
#include "libbbpdias/cgal_tools.hpp"

namespace dias {

    typedef uint32_t vertex_id;

    typedef dias::btree<SpatialIndex::id_type, vertex_id> vertex_to_page_id_map;
    typedef vertex_to_page_id_map::query_answer map_query_result;

    typedef boost::unordered_set<vertex_id> vertex_set;

struct vertex_info
{
    dias::vect coords;
    std::vector<vertex_id> neighbours;
    dias::mbr voronoi_mbr;

    typedef uint32_t size_type;

    friend inline std::ostream & operator << (std::ostream & out,
                                              const vertex_info & v)
        {
            out << v.coords << ' ';
            v.voronoi_mbr.output_without_endl (out);
            BOOST_FOREACH (vertex_id n_id, v.neighbours)
            {
                out << " " << n_id;
            }
            out << '\n';
            return out;
        }

    vertex_info ()
    {
        coords[0] = coords[1] = coords[2] = 0.0;
    }

    vertex_info (dias::vect _coords) :
        coords (_coords)
        {
        }

    vertex_info (dias::vect _coords, size_t expected_neighbours) :
        coords (_coords),
        neighbours (expected_neighbours, 0)
        {
        }

    size_t serialized_header_size (void) const
            {
                return sizeof (size_type) + sizeof(vertex_id)
                    + dias::vect_size_float
                    + dias::mbr::serialized_size_in_float ();
            }

    size_t serialized_size (void) const
        {
            return serialized_header_size() + sizeof (vertex_id) * neighbours.size ();
        }

    void serialize (vertex_id v_id, std::vector<byte> & serialized) const
        {
            size_type s = serialized_size ();
#ifdef VERBOSE
            std::cerr << "Serializing bytes = " << s << '\n';
            std::cerr << "<< sizeof (length) = " << sizeof (s)
                      << " sizeof(vid) = " << sizeof (v_id) << '\n';
#endif
            serialized.reserve (serialized.size () + s);
            serialized.resize (serialized.size () + s);
            byte *end_of_serialized = &serialized.back () - s + 1;
            dias::serialize (s, &end_of_serialized);
            dias::serialize (v_id, &end_of_serialized);
            dias::serialize_vect_as_float (coords, &end_of_serialized);
            voronoi_mbr.serialize_as_float (&end_of_serialized);
            BOOST_FOREACH (vertex_id v, neighbours)
            {
                dias::serialize (v, &end_of_serialized);
            }
#ifndef NDEBUG
            if (end_of_serialized != &serialized.back () + 1)
            {
                std::cerr << "end_of_serialized = "
                          << (void *)end_of_serialized << '\n';
                std::cerr << "&serialized.back () + 1 = "
                          << (void *)(&serialized.back () + 1) << '\n';
            }
#endif
            assert (end_of_serialized == &serialized.back () + 1);
        }

    void include_in_voronoi_mbr (const dias::vect & point_to_include)
        {
            voronoi_mbr.expand (point_to_include);
        }

    std::ostream & output_in_binary (vertex_id v_id, std::ostream & out)
        {
            std::vector<byte> buffer;
            serialize (v_id, buffer);
            out.write ((char *)(&buffer[0]), buffer.size ());
            return out;
        }

    static struct vertex_info make_from_buffer (byte ** ptr, ptrdiff_t size)
        {
            byte * start = *ptr;

            dias::vect coords = dias::unserialize_vect_as_float (ptr);
            struct vertex_info
                result(coords,
                       (size - (*ptr - start
                                + dias::mbr::serialized_size_in_float ()))
                       / sizeof (vertex_id));
            result.voronoi_mbr = dias::mbr::unserialize_as_float (ptr);

            std::vector<vertex_id>::iterator n_ptr
                = result.neighbours.begin ();
            while (*ptr - start < (ptrdiff_t)size)
            {
                dias::unserialize (*n_ptr, ptr);
                n_ptr++;
            }
            assert (n_ptr == result.neighbours.end ());
            assert (start + size == *ptr);

            return result;
        }


    static struct vertex_info read_in_binary (std::istream & in,
                                              vertex_id & v_id)
        {
            size_type length;
            in.read ((char *)&length, sizeof (length));
            in.read ((char *)&v_id, sizeof (v_id));

            length -= (sizeof(length) + sizeof (v_id));
            std::vector<byte> buffer (length);
            in.read ((char *)(&buffer[0]), length);
            byte * ptr = &buffer[0];

            return vertex_info::make_from_buffer (&ptr, (ptrdiff_t)length);
        }
};

typedef boost::unordered_map<vertex_id, vertex_info> vertex_db;
typedef std::pair<vertex_id, vertex_info> vertex_db_pair;

inline void
read_all_vertices_from_page (SpatialIndex::IStorageManager & disk_file,
                             SpatialIndex::id_type page_id,
                             std::vector<vertex_info> & vertices)
{
#ifdef PROFILING
    c1.start();
#endif

    size_t length;
    boost::shared_array<byte> bytes
        = dias::read_byte_array (disk_file, page_id, length);
    byte * c_bytes = bytes.get ();

#ifndef NDEBUG
    byte * start = c_bytes;
#endif
    vertex_id current_id = 0xFFFFFFFF;
    uint32_t vertices_in_page;
    dias::unserialize (vertices_in_page, &c_bytes);
    vertices.reserve (vertices.size () + vertices_in_page);

#ifdef VERBOSE
    std::cerr << "Loading all vertices in page id = "
              << page_id <<  " containing " <<  vertices_in_page
              << " vertices\n";
#endif
    unsigned i = 0;
    dias::vertex_info::size_type current_size = 0xFFFFFFF;
    byte * current_start = NULL;
#ifdef PROFILING
    c1.stop();
    c2.start();
#endif
    while (i < vertices_in_page)
    {
        assert (c_bytes - start < (ptrdiff_t)length);
        current_start = c_bytes;
        dias::unserialize (current_size, &c_bytes);
        dias::unserialize (current_id, &c_bytes);
#ifdef VERBOSE
        std::cerr << "Reading id = " << current_id << " size = "
                  << current_size << '\n';
#endif
        vertices.push_back (
            vertex_info::make_from_buffer (&c_bytes,
                                           current_size
                                           - sizeof (current_size)
                                           - sizeof (current_id)));
        i++;
    }
#ifdef PROFILING
    c2.stop();
#endif
    assert (c_bytes - start == (ptrdiff_t)length);
}

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




}

#endif
