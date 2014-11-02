#ifndef MESH_STREAM_HPP_
#define MESH_STREAM_HPP_

#include <BBP/Model/Microcircuit/Containers/Neurons.h>

#include <spatialindex/SpatialIndex.h>

#include "data_stream.hpp"

class mesh_stream : public data_stream
{
private:
    bbp::Array<bbp::Vertex_Index>::const_iterator t;
    bbp::Array<bbp::Vertex_Index>::const_iterator e;

    dias::vertex_array vertices;

    virtual void start_neuron()
	{
	    data_stream::start_neuron();
	    const bbp::Mesh & mesh = n_i->morphology().mesh();
	    t = mesh.triangles().begin();
	    e = mesh.triangles().end();
	    vertices = mesh.vertices();
	}

public:
    mesh_stream (const bbp::Neurons * n, unsigned _limit) :
	data_stream(n, _limit)
	{
	    start_neuron();
	}

    virtual SpatialIndex::IData* getNext()
        {
            assert (t != e);
            assert (have_next);

            dias::triangle_ids tids = { {*t++, *t++, *t++} };
            dias::vect r = dias::get_triangle_center (tids, vertices, *gtrafo);

            world.expand (r);

            SpatialIndex::Region rgn = dias::mbr (r, r).as_region ();
            std::string data = "";

            SpatialIndex::IData * result = new data_segment(next_id, rgn,
                                                            data.length(),
                                                            data.c_str());
            next_id++;

            i++;
            if (i == limit)
                have_next = false;

            if (t == e)
                advance_neuron();

            return result;
        }

};

#endif
