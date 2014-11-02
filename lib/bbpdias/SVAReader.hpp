#ifndef SVAREADER_HPP_
#define SVAREADER_HPP_

#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "delaunay_connectivity.hpp"
#include "Vertex_info.hpp"

namespace dias {

 class SVAReader
 {
 private:
     std::ifstream f;
     dias::vertex_db vertices;
     std::queue<dias::vertex_id> finalized_vertices;
     dias::vertex_id v_id;

     void read_vertex (boost::tokenizer<>::iterator & p,
                       const boost::tokenizer<>::iterator & end)
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

             vertices.insert (std::make_pair (v_id++,
                                              dias::vertex_info (vertex)));
         }

     dias::tetrahedra
     read_tetrahedra (boost::tokenizer<>::iterator & p,
                      const boost::tokenizer<>::iterator & end)
         {
             dias::vertex_id v_ids[4];
             int j = 0;
             while (p != end)
             {
                 std::string v_str = *p++;
                 int v = boost::lexical_cast<int> (v_str);
                 if (v >= 0)
                     v_ids[j] = v;
                 else
                 {
                     v_ids[j] = v_id + v;
                     finalized_vertices.push (v_ids[j]);
                 }
                 j++;
             }
             assert (j == 4);

             return dias::tetrahedra (v_ids, vertices);
         }

     void add_neighbours (const dias::tetrahedra & t)
         {
             for (int i = 0; i < 4; i++)
             {
                 dias::vertex_db::iterator v_ptr
                     = vertices.find (t.vertices[i]);
                 assert (v_ptr != vertices.end ());

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

 public:
     SVAReader(const std::string & fn) :
         f (fn.c_str ()),
         v_id (1)
         {
         }

    ~SVAReader()
        {
            f.close ();
        }

    // FIXME: will fail if last line on file is empty
    bool hasNext()
        {
            return finalized_vertices.size () > 0 || !f.eof ()
                || vertices.size () > 0;
        }

    Vertex_info * getNext()
        {
            if (finalized_vertices.size () > 0)
            {
                dias::vertex_id v_id = finalized_vertices.front ();
                finalized_vertices.pop ();
                dias::vertex_db::iterator v_ptr = vertices.find (v_id);
                assert (v_ptr != vertices.end ());
                Vertex_info * result = new Vertex_info;
                result->vid = v_id;
                result->vi = v_ptr->second;
                vertices.erase (v_ptr);
                return result;
            }

            if (!f.eof ())
            {
                while (finalized_vertices.size () == 0 && !f.eof ())
                {
                    std::string line = "";

                    while (line == "")
                    {
                        if (f.eof ())
                            break;
                        std::getline (f, line);
                    }
                    if (f.eof ())
                        break;

                    boost::char_delimiters_separator<char> sep (false, "", 0);
                    boost::tokenizer<> tokens (line, sep);
                    boost::tokenizer<>::iterator p = tokens.begin ();

                    std::string type = *p++;

                    if (type == "#")
                        continue;

                    if (type == "v")
                        read_vertex (p, tokens.end ());
                    else if (type == "c")
                    {
                        dias::tetrahedra t
                            = read_tetrahedra (p, tokens.end ());
                        add_neighbours (t);
                    }
                    else
                        assert (false);
                }

                if (finalized_vertices.size () > 0)
                    return getNext ();
            }

            assert (vertices.size () > 0);
            assert (finalized_vertices.size () == 0);
            dias::vertex_db::const_iterator p = vertices.begin ();
            finalized_vertices.push (p->first);
            return getNext ();
        }

     unsigned get_read_vertices (void) const
         {
             return v_id - 1;
         }
};

}

#endif
