#ifndef DATA_STREAM_HPP_
#define DATA_STREAM_HPP_

#include <iostream>
#include <memory>

#include <BBP/Model/Microcircuit/Containers/Neurons.h>

#include <SpatialIndex.h>

#include <boost/utility.hpp>

#include "tools.hpp"
#include "bbp_tools.hpp"
#include "mbr.hpp"

class data_stream : public SpatialIndex::IDataStream, boost::noncopyable
{
protected:
    unsigned limit;
    unsigned i;

    SpatialIndex::id_type next_id;

    bool have_next;

    dias::mbr world;
    dias::mbr neuron_center;

    const dias::global_transformer * gtrafo;

    bbp::Neurons::const_iterator n_i;
    bbp::Neurons::const_iterator n_e;

    data_stream (const bbp::Neurons * n, unsigned _limit) :
	limit(_limit),
	i(0),
        next_id(1),
	have_next(true),
	gtrafo(NULL),
	n_i(n->begin()),
	n_e(n->end())
	{
	}

    virtual void start_neuron ()
	{
	    std::cout << "Neuron a" << n_i->gid() << " with morphology "
		      << n_i->morphology().label() << std::endl;
	    gtrafo = &n_i->global_transform();
            const bbp::Vector_3D<bbp::Micron> & neuron_center_bbp
                = n_i->position();
            const dias::vect n_c = { {neuron_center_bbp.x(),
                                      neuron_center_bbp.y(),
                                      neuron_center_bbp.z()} };
            neuron_center.expand (n_c);
	}

    void advance_neuron ()
	{
	    n_i++;
	    if (n_i == n_e)
	    {
		have_next = false;
		return;
	    }
	    start_neuron();
	}

public:
    unsigned objs_loaded() const
	{
	    return i;
	}

    virtual bool hasNext()
	{
	    return have_next;
	}

    virtual size_t size()
	{
	    abort(); // FIXME
	}

    virtual void rewind()
	{
	    abort(); // FIXME
	}

    SpatialIndex::Region getNeuronCenterWorldExtent () const
        {
            return neuron_center.as_region ();
        }

    SpatialIndex::Region getWorldExtent () const
        {
            return world.as_region ();
        }
};

#endif
