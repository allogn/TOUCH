#ifndef SEGMENT_STREAM_HPP_
#define SEGMENT_STREAM_HPP_

#include <memory>

#include <BBP/Model/Microcircuit/Containers/Neurons.h>
#include <BBP/Model/Microcircuit/Containers/Sections.h>
#include <BBP/Model/Microcircuit/Containers/Segments.h>

#include <SpatialIndex.h>

#include "data_stream.hpp"

class segment_stream : public data_stream
{
private:
    std::auto_ptr<bbp::Sections> sections;
    bbp::Sections::const_iterator s_i;
    bbp::Sections::const_iterator s_e;
    std::auto_ptr<bbp::Segments> segments;
    bbp::Segments::const_iterator g_i;
    bbp::Segments::const_iterator g_e;

    void init_segments ()
        {
            segments.reset(new bbp::Segments(s_i->segments()));
            g_i = segments->begin();
            g_e = segments->end();
            if (g_i == g_e)
                advance_section();
        }

    virtual void start_neuron ()
        {
            data_stream::start_neuron();
            sections.reset(new
                           bbp::Sections(n_i->morphology().all_sections()));
            s_i = sections->begin();
            s_e = sections->end();
            init_segments();
        }

    void advance_section ()
	{
	    s_i++;
	    if (s_i == s_e)
	    {
		advance_neuron();
		return;
	    }
	    init_segments();
	}

public:
    segment_stream (const bbp::Neurons * n, unsigned _limit) :
	data_stream(n, _limit)
	{
	    start_neuron();
	}

    virtual SpatialIndex::IData* getNext()
        {
            assert (g_i != g_e);
            assert (have_next);

            dias::vect r = dias::get_segment_center (*g_i, *gtrafo);
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

            g_i++;
            if (g_i == g_e)
                advance_section();

            return result;
        }
};

#endif
