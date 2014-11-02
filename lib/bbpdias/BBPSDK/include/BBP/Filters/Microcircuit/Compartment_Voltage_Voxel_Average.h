/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Responsible authors:	Thomas Traenkler
								Nikolai Chapochnikov

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_VOLTAGE_VOXEL_AVERAGE_H
#define BBP_COMPARTMENT_VOLTAGE_VOXEL_AVERAGE_H

#include <memory>
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/System/File/File.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/System/Time/Sleep.h"
#include "BBP/Common/Dataset/Containers/Array.h"
#include "BBP/Common/System/Time/Timer.h"

#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
#include "BBP/Common/System/parallel.h"
#ifdef BBP_SUPPORT_TBB
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/spin_mutex.h"
#include "tbb/blocked_range.h"
#endif // tbb
//#endif // threads
#endif // experimental

#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"
#include "BBP/Filters/Microcircuit/Segment_Voxel_Mapper.h"
#include "BBP/Model/Microcircuit/Neuron.h"
#include "BBP/Model/Microcircuit/Segment.h"
#include "BBP/Common/Exception/Exception.h"


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#undef min

namespace bbp {

// ----------------------------------------------------------------------------

struct Voxel_Occupancy
{
    //! Linear voxel identifier.
    Index index;
    //! The percentage of the voxel that is occupied by the compartment.
    Percentage percentage;

    Voxel_Occupancy()
    {}
    Voxel_Occupancy(Index index, Percentage percentage)
	: index(index), percentage(percentage) {}

	template <class Archive>
	void serialize (Archive & ar, const unsigned int version)
    {
        ar & index;
        ar & percentage;
    }
};

//---------------------------------------------------------------------------

class Voxel_Average;

//---------------------------------------------------------------------------

/*!
    \ingroup Filter
*/
class Compartment_Voltage_Voxel_Average
    : public Filter <Microcircuit, Volume <Millivolt, Micron, Millisecond> >
{
    friend class Voxel_Average;
public:
	Cell_Target		cell_target;
	//! set empty space voltage
	inline Compartment_Voltage_Voxel_Average(
        Millivolt empty_space_voltage = - 100.0f);
	//! set volume dimensions and set empty space voltage
	inline Compartment_Voltage_Voxel_Average(
        const Box <Micron> &  bounding_box,
        Micron          pixel_size,
		Millivolt       empty_space_voltage = - 100.0f);
	//! set volume dimensions, target and set empty space voltage
	inline Compartment_Voltage_Voxel_Average(
        const Box <Micron> &      bounding_box,
		Micron              pixel_size,
        const Cell_Target & cell_target,
        Millivolt           empty_space_voltage = - 100.0f);

    //! load mapping from file and set empty space voltage
    /*!
        \bug Serialization bug on Windows with boost 1.35 & 1.36 for native
        binary files (TT)
    */
	inline Compartment_Voltage_Voxel_Average(
        Filename    mapping_filename,
        bool        portable_format = false,
		Millivolt   empty_space_voltage = - 100.0f);

    ~Compartment_Voltage_Voxel_Average(void);

	//! initializes and starts the filter
	void start();
	//! compute the volume voltage average and write it to output
	inline void process();

	//! load the volume compartment mapping from file
	void open(Filename mapping_filename, bool portable_format = false);

    //! name of the filter
	Label name() const;
	//! check filter configuration
	bool is_configured() const;

#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
protected:
    std::vector<size_t> _frame_indices;
    Array<Millivolt>  _millivolt_array;
    //std::vector<Voxel_Average> _voxel_thread_functions;
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SUPPORT_TBB
    std::auto_ptr<Parallel_For_Each<std::vector<size_t>,
        Voxel_Average> > parallel_for_each;
#else
    tbb::task_scheduler_init init;
#endif
//#endif // threads
#endif // experimental

private:
	friend class boost::serialization::access;

    Box <Micron>                       bounding_box;
    Micron                             pixel_size;
    Volume <Millivolt, Micron, Millisecond>         initial_voltages;
    Count                              resolution_x,
                                       resolution_y,
                                       resolution_z;
    bool                               mapping_loaded;
    std::vector< std::vector < Voxel_Occupancy > > index_to_voxel_mapping;
    Millivolt                          empty_space_voltage;

        template <class Archive>
        void serialize (Archive & ar, const unsigned int version);
};


// ----------------------------------------------------------------------------

template <class Archive>
void Compartment_Voltage_Voxel_Average::serialize(
    Archive & ar, const unsigned int version)
{
    ar  & bounding_box
		& pixel_size;
	ar	& resolution_x
		& resolution_y
		& resolution_z;
	ar  & index_to_voxel_mapping;
}


// ----------------------------------------------------------------------------

#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED

//! \todo This should not be public. (TT)
class Voxel_Average
{
public:
    Compartment_Voltage_Voxel_Average & _master;
    Voxel_Average(Compartment_Voltage_Voxel_Average & master)
        : _master(master)
    {
    }

#ifdef BBP_SUPPORT_TBB
    void operator()(const tbb::blocked_range<int> & range) const
    {
        Volume<Millivolt, Micron, Millisecond> & volume      = _master.output();
        Array<Millivolt>          & millivolts  = _master._millivolt_array;
        std::vector< std::vector < Voxel_Occupancy > > & mapping 
            = _master.index_to_voxel_mapping;

        for (int i = range.begin(); i != range.end(); ++i)
        {
            size_t number_of_segments = mapping[i].size();
            // for all segments in this voxel
            for (size_t j = 0 ; j < number_of_segments; ++j)
            {
                volume(mapping[i][j].index) += 
                    (Millivolt) mapping[i][j].percentage * millivolts[i];
            }
        }
    }
#endif

    inline void operator()(size_t i);
};

//#endif // threads
#endif // experimental

// ----------------------------------------------------------------------------

Compartment_Voltage_Voxel_Average::
Compartment_Voltage_Voxel_Average(Millivolt empty_space_voltage)
 : 
   pixel_size(1.0f),
   mapping_loaded(false),
   empty_space_voltage(empty_space_voltage)
#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
#ifdef BBP_SUPPORT_TBB
   ,init(tbb::task_scheduler_init::automatic)
#endif
{
//      parallel_for_each.reset(new Parallel_For_Each<std::vector<size_t>,
//        Voxel_Average>(Voxel_Average(*this), hardware_concurrency()));
//#endif // threads
#else
{
#endif
}

// ----------------------------------------------------------------------------

Compartment_Voltage_Voxel_Average::
Compartment_Voltage_Voxel_Average(Filename mapping_filename,
                                  bool portable_format,
                                  Millivolt empty_space_voltage)
    : 
      mapping_loaded(true),
	  empty_space_voltage(empty_space_voltage)
#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
#ifdef BBP_SUPPORT_TBB
   ,init(tbb::task_scheduler_init::automatic)
#endif
{
//      parallel_for_each.reset(new Parallel_For_Each<std::vector<size_t>,
//        Voxel_Average>(Voxel_Average(*this), hardware_concurrency()));
//#endif // threads
#else
{
#endif
	open(mapping_filename, portable_format);
}


// ----------------------------------------------------------------------------

Compartment_Voltage_Voxel_Average::
Compartment_Voltage_Voxel_Average(const Box<Micron> & bounding_box, 
											   Micron pixel_size, 
											   Millivolt empty_space_voltage)
	: 
      bounding_box(bounding_box), 
	  pixel_size(pixel_size),
	  mapping_loaded(false),
	  empty_space_voltage(empty_space_voltage)
#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
#ifdef BBP_SUPPORT_TBB
   ,init(tbb::task_scheduler_init::automatic)
#endif
{
//      parallel_for_each.reset(new Parallel_For_Each<std::vector<size_t>,
//        Voxel_Average>(Voxel_Average(*this), hardware_concurrency()));
//#endif
#else
{
#endif
}

// ----------------------------------------------------------------------------

Compartment_Voltage_Voxel_Average::
Compartment_Voltage_Voxel_Average(const Box<Micron> &         bounding_box, 
								  Micron                pixel_size,
								  const Cell_Target &   cell_target,
								  Millivolt             empty_space_voltage)
	: 
      cell_target(cell_target),
      bounding_box(bounding_box),
      pixel_size(pixel_size), 
	  mapping_loaded(false),
	  empty_space_voltage(empty_space_voltage)
#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
#ifdef BBP_SUPPORT_TBB
   ,init(tbb::task_scheduler_init::automatic)
#endif
{
//      parallel_for_each.reset(new Parallel_For_Each<std::vector<size_t>,
//        Voxel_Average>(Voxel_Average(*this), hardware_concurrency()));
//#endif
#else
{
#endif

}

// ----------------------------------------------------------------------------

#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
inline void Voxel_Average::operator()(size_t i)
{
    Volume<Millivolt, Micron, Millisecond> & volume      = _master.output();
    Array<Millivolt>          & millivolts  = _master._millivolt_array;
    std::vector< std::vector < Voxel_Occupancy > > & mapping 
        = _master.index_to_voxel_mapping;

    size_t number_of_segments = mapping[i].size();
    // for all segments in this voxel
    for (size_t j = 0 ; j < number_of_segments; ++j)
    {
        volume(mapping[i][j].index) += 
            (Millivolt) mapping[i][j].percentage * millivolts[i];
    }
}
//#endif
#endif

// ----------------------------------------------------------------------------

inline void Compartment_Voltage_Voxel_Average::process()
{
    Timer timer;


    if (initialized == false)
    {
        timer.start("Initializing Compartment_Voltage_Voxel_Average");
#ifdef BBP_EXPERIMENTAL
//#ifdef BBP_THREADS_SUPPORTED
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_FOUND_TBB
        //! \todo This is a hack to get going with parallel processing. (TT)
        _frame_indices.resize(input().dataset().dynamics->
            compartment_voltages.size());
        for (size_t i = 0; i < _frame_indices.size(); ++i)
        {
            _frame_indices[i] = i;
        }
#endif
//#endif // threads
#endif // experimental
        /*!
            \todo check if all the necessary data is loaded
            (mvd, morphologies, ...)
        */

         // construct mapping if not already loaded from file
        if (mapping_loaded == false)
        {
            // compute segment mapping from scratch
            Segment_Voxel_Mapper	mapper(pixel_size, bounding_box);
		    mapper.input_ptr(input_ptr());
            Segment_Voxel_Mapping & segment_mapping = mapper.output();
		    mapper.process();

		    // constructing the compartment index to voxel mapping
		    resolution_x = segment_mapping.resolution().x();
		    resolution_y = segment_mapping.resolution().y();
		    resolution_z = segment_mapping.resolution().z();

		    std::clog << "Volume Voltage Average: Sizing volumes..."
                << std::endl;
		    // initialize voltage output volume
		    output().resize(resolution_x, resolution_y, resolution_z,
			    std::numeric_limits<Millivolt>::min() );

		    // initialize empty baseline voltage output volume
		    initial_voltages.resize(resolution_x, resolution_y, resolution_z,
			    std::numeric_limits<Millivolt>::min() );
		    std::clog << "Volume Voltage Average: Volumes sized."
                << std::endl;

		    std::cout << "Compartment Volume Mapping: Constructing..."
                << std::endl;

            size_t _report_index_size =
                input().dataset().dynamics->compartment_voltages.size();
		    index_to_voxel_mapping.resize(_report_index_size);

		    // iterate over all voxels
		    //! \todo could remove the processed voxels already to save memory!
		    for(Count x = 0; x < resolution_x; x++)
		    for(Count y = 0; y < resolution_y; y++)
		    for(Count z = 0; z < resolution_z; z++)
		    {
			    // retrieve segments in that voxel
			    Index position_index = segment_mapping.linear_index(x,y,z);
			    Segments_in_Voxel voxel_segments =
				    segment_mapping.segments_in_voxel(x,y,z);
			    Micron3 volume_in_voxel = 0;
			    size_t number_of_segments_in_voxel = voxel_segments.size();

			    // initialize occupied voxels
			    if (number_of_segments_in_voxel  != 0)
				    initial_voltages(x,y,z) = 0;

			    // add up total tissue volume inside that voxel
			    for (size_t i = 0; i < number_of_segments_in_voxel; ++i)
			    {
				    volume_in_voxel += voxel_segments[i].volume;
			    }

                Microcircuit & in = input();
			    // map voltage frame index to voxel location
			    for (size_t i = 0; i < number_of_segments_in_voxel; ++i)
                {
				    Neuron  & neuron =
                        in.neuron(voxel_segments[i].segment.cell_gid);
				    Section_Normalized_Distance distance_in_section  =
					    neuron.morphology().section_distance(
						    voxel_segments[i].segment.section_id,
						    voxel_segments[i].segment.segment_id);
				    Compartment_ID compartment_id = neuron.compartment(
					    voxel_segments[i].segment.section_id,
                        distance_in_section);
				    Report_Frame_Index voltage_frame_index =
					    neuron.compartment_frame_index(
						    voxel_segments[i].segment.section_id,
						    compartment_id);

				    // normalize by volume in voxel and write to volume
                    // compartment map
				    index_to_voxel_mapping[(size_t) voltage_frame_index].
					    push_back( Voxel_Occupancy(
							    position_index,
							    voxel_segments[i].volume / volume_in_voxel));
                }
		    }
		    std::cout << "Compartment Volume Mapping: Constructed."
                << std::endl;
        }
        // case if mapping already loaded from file
        else
        {
            size_t mapping_size = index_to_voxel_mapping.size();

	        // resize output averaged voltage volume
	        output().resize(resolution_x,
			        resolution_y,
			        resolution_z,
			        empty_space_voltage);

	        // resized the initial voltages volume before averaging
	        // which deals with the empty space case
	        initial_voltages.resize(
				        resolution_x,
				        resolution_y,
				        resolution_z,
				        empty_space_voltage);

            // initialize initial voltages volume
            for (size_t i = 0 ; i < mapping_size ; ++i)
            {
                size_t vector_size = index_to_voxel_mapping[i].size();
		        for (size_t j = 0 ; j < vector_size ; ++j)
                {
		            initial_voltages(index_to_voxel_mapping[i][j].index) = 0;
                }
            }
        }
        initialized = true;
        timer.print();
    }

    timer.start("Compartment_Voltage_Voxel_Average frame processing");
    output() = initial_voltages;

#ifdef BBP_EXPERIMENTAL
#ifdef BBP_SUPPORT_TBB
    _millivolt_array = Array<Millivolt>(
        input().dataset().dynamics->compartment_voltages.frame(),
        input().dataset().dynamics->compartment_voltages.size());
    //parallel_for_each->operator()(_frame_indices);
    int _report_index_size = (int) (
        input().dataset().dynamics->compartment_voltages.size());
    
    tbb::parallel_for(
        tbb::blocked_range<int>(0, _report_index_size), 
        Voxel_Average(* this));
    
    timer.print();
#endif
#else
    Microcircuit & in = input();
    Millivolt * _millivolt_array =
        in.dataset().dynamics->compartment_voltages.frame();
    //! \bug Potential bug, but OpenMP needs int. (TT)
    size_t _report_index_size = (
        in.dataset().dynamics->compartment_voltages.size());

	Volume <Millivolt, Micron, Millisecond> & volume = output();

    //! \todo parallel_for with threading will speed this up. (TT)
    //#pragma omp parallel for
    for(size_t  i = 0 ; i < _report_index_size; ++i)
    {
		size_t number_of_segments = index_to_voxel_mapping[i].size();
		for (size_t j = 0 ; j < number_of_segments; ++j)
		{
			volume(index_to_voxel_mapping[i][j].index) +=
                (Millivolt) index_to_voxel_mapping[i][j].percentage
                * _millivolt_array[i];
		}
    }
    volume.time(in.dataset().dynamics->compartment_voltages.time());
    timer.print();
#endif
}

// ----------------------------------------------------------------------------

inline bool Compartment_Voltage_Voxel_Average::is_configured() const
{
	// check if input contains voltages
    if (input().dataset().dynamics->compartment_voltages.frame() == 0)
    {
		return false;
    }

    if (Filter<Microcircuit, 
        Volume <Millivolt, Micron, Millisecond> >::is_configured()
        == false)
    {
        return false;
    }

	// if all checks passed successfully, green signal to start
    return true;
}

//---------------------------------------------------------------------------

// this code is currently not used, but kept for reference
//class Compartment_Voltage_Voxel_Average;
//BOOST_CLASS_TRACKING(Compartment_Voltage_Voxel_Average, boost::serialization::track_never)
//BOOST_CLASS_TRACKING(Voxel_Occupancy, boost::serialization::track_never)

//void SerializeClass(Filename filename);
//inline void Compartment_Voltage_Voxel_Average::SerializeClass(Filename filename)
//{
//    //std::ofstream serialize_ofs(filename.c_str());
//    //std::cout << "Serializing the Class" << std::endl;
//    //boost::archive::binary_oarchive oa(serialize_ofs);
//    //std::cout << "done, now writing the file" << std::endl;
//    //oa << *this ;
//    //std::cout << "done, file written" << std::endl;
//}

//---------------------------------------------------------------------------

}
#endif
