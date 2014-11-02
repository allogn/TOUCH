/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007. All rights reserved.

        Responsible authors:    Nikolai Chapochnikov
                                Thomas Traenkler

*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SEGMENT_VOXEL_MAPPER_H
#define BBP_SEGMENT_VOXEL_MAPPER_H

#include <vector>
#include <map>
#include <utility>
#include <limits>
#include <cmath>

#include "BBP/Common/Filter/Filter.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Experiment/Experiment.h"
#include "BBP/Model/Microcircuit/Neuron.h"
#include "BBP/Model/Microcircuit/Soma.h"
#include "BBP/Model/Microcircuit/Containers/Sections.h"

#ifdef BBP_USE_BOOST_SERIALIZATION
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning ( push )
#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4512 )
#endif
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning ( pop )
#endif
#endif


namespace bbp {

typedef size_t Segment_Index;

//! A pixel coordinate like in 2D could be x=50,y=64.
/*!
    \todo The type this is based on might change with resolution and
    whether coordinates can become negative. Checks and conversion 
    on this type need to be reviewed.
*/
typedef int Pixel_Coordinate;
const int UNDEFINED_PIXEL_COORDINATE = std::numeric_limits<int>::max();

// ----------------------------------------------------------------------------

/*!
    \brief Computes a mapping between segments and voxels.
    \ingroup Filter
*/
class Segment_Voxel_Mapper
    : public Filter <Microcircuit, Segment_Voxel_Mapping>
{
#ifdef BBP_USE_BOOST_SERIALIZATION
    friend class boost::serialization::access;
#endif

public :
    //! the cells that should be included in the density calculation
    Cell_Target cell_target;

    //! Constructs and intializes the segment to voxel mapper filter.
    /*!
        \param voxel_size Size of the volume elements specified by the side 
        length of the voxel cube in micron.
        \param bounding_box Specifies the height, width, depth and position of
        the mapped area. 
        \param clean_cut specifies whether segments are cut at the borders
        of the volume or if the whole segment is removed instead.
    */
    Segment_Voxel_Mapper(Micron voxel_size, 
                         Box <Micron> & bounding_box, 
                         bool clean_cut = true);
    
    ~Segment_Voxel_Mapper()
    {
#ifdef BBP_DEBUG
        //! \todo remove this debug output.
        std::cout << "Segment_Voxel_Mapper::~Segment_Voxel_Mapper()" << std::endl;
#endif
    }

    void initialize(Experiment&);
    //! Calculates the segment volume density for all the voxels of the volume.
    inline void process();

private:
    Segment_Index       current_global_segment_index;

    Neuron            * current_neuron;
    const bool          clean_cut;
    Vector_3D <Micron>  minimum,
                        maximum;
    Micron              voxel_size, voxel_size_reciprocal;
    Micron              half_pixel;
    Micron3             voxel_volume;
    Micron3             soma_volume;
    Micron3             neurites_volume;
    const bool          safe_mode;
    
private:
    //! Calculate the volume contained in a cone geometry.
    inline Micron3 cone_volume(Micron r1, Micron r2, Micron h);
    //! Insert a neuron into the mapping volume.
    void insert_neuron(const Neuron & neuron);
    //! Insert a neuron cell body into the mapping volume.
    void insert_soma(const Soma & soma);
    //! Insert a segment into the mapping volume
    void insert_segment(const Segment & segment);
    //! Insert a segment smaller than a voxel into the volume.
    void insert_small_segment(const Segment & segment);
    //! Insert a segment larger than a voxel into the volume.
    void insert_large_segment(const Segment & segment);
    
#ifdef BBP_USE_BOOST_SERIALIZATION
    /*!
        \todo Serialize functions should move to << operator. Review
        implications for SWIG wrapping. (TT)
    */
    template <class Archive>
    void serialize (Archive & ar, const unsigned int version)
    {
        //! \todo fix endianness during loading or writing
        ar & boost::serialization::base_object<Segment_Voxel_Mapper> (*this);
        ar & output().segment_index_gid_lookup;
    }
#endif

};


// ----------------------------------------------------------------------------

void Segment_Voxel_Mapper::process()
{
    // Reset segment counter.
    size_t  number_of_segments = 0;
    // Clear the output volume index gid lookup table.
    output().segment_index_gid_lookup.clear();
    // Update the neurons from the input object.
    Neurons neurons(input().neurons());

    //! Define the cell target that a mapping is created for.
    if (cell_target.size() != 0)
    {
        neurons = neurons & cell_target;
    }

    Neurons::const_iterator neurons_end = neurons.end(); 

    //! Calculate the number of segments.
    for (Neurons::const_iterator i = neurons.begin(); i != neurons_end; ++i)
    {
        number_of_segments += i->number_of_points() - 1;
    }

    //! Reserve memory for estimated number of segments.
    output().segment_index_gid_lookup.reserve(number_of_segments);

    //! Construct the mapping.
    log_message("Segment Volume Mapping: Constructing...", 
        INFORMATIVE_LEVEL, __FILE__, __LINE__);
    for (Neurons::const_iterator i = neurons.begin(); i != neurons_end; ++i)
    {
        //std::clog << "Neuron a" << i->gid << " (morphology " 
        //        << i->morphology().name() << ")" << std::endl;
        insert_neuron(* i);
    }

    log_message("Segment Volume Mapping: Constructed.", 
        INFORMATIVE_LEVEL, __FILE__, __LINE__);
}

// ----------------------------------------------------------------------------

inline Micron3 Segment_Voxel_Mapper::cone_volume(
                                Micron r1, Micron r2, Micron h)
{
    return ((float) M_PI) * h * ( r1*r1 + r1*r2 + r2*r2) / 3.0f;
}
    
// ----------------------------------------------------------------------------

inline
void Segment_Voxel_Mapper::insert_small_segment(const Segment & segment)
{
/*
#ifndef NDEBUG
    std::cout << "Segment_Voxel_Mapper::insert_small_segment() with id " 
    << segment.id() << std::endl;
    segment.print();
#endif
*/
    const Vector_3D<Micron> & position_begin = segment.begin().center();
    const Vector_3D<Micron> & position_end = segment.end().center();
    const Micron & radius_begin = segment.begin().radius();
    const Micron & radius_end = segment.end().radius();
    
    // begin and end of the segment relative to minimum of volume box
    Vector_3D<Micron> begin_relative_to_box (position_begin - minimum), 
    end_relative_to_box   (position_end - minimum);
    // vector from segment begin to end
    Vector_3D<Micron> segment_vector = end_relative_to_box 
                                     - begin_relative_to_box;
    // length of segment
    Micron length = segment_vector.length();
    
    // check if segment length is positive
    if (length > 0)
    {
        // two times the number of voxels that would fit in seg. length
        Count step_number = 
        Pixel_Coordinate(ceil(2 * length * voxel_size_reciprocal));
        Micron step_length = length / (Micron)step_number;
        Vector_3D<Micron> step = segment_vector / (Micron)step_number;
        Micron volume(0);
        Vector_3D<Pixel_Coordinate> current_voxel (
                      (Pixel_Coordinate)(floor(begin_relative_to_box.x() 
                              * voxel_size_reciprocal) - 2),
                      (Pixel_Coordinate)(floor(begin_relative_to_box.y() 
                              * voxel_size_reciprocal) - 2),
                      (Pixel_Coordinate)(floor(begin_relative_to_box.z()
                              * voxel_size_reciprocal) - 2));
        
        //Pixel_Coordinate last_x = UNDEFINED_PIXEL_COORDINATE, 
        //                 last_y = UNDEFINED_PIXEL_COORDINATE, 
        //                 last_z = UNDEFINED_PIXEL_COORDINATE;
        
        for (Count i = 0; i < step_number ; i++)
        {
            Vector_3D<Micron> pos = begin_relative_to_box 
            + (step * (i + 0.5f)); 
            Pixel_Coordinate x = Pixel_Coordinate(pos.x() 
                                                  * voxel_size_reciprocal);
            Pixel_Coordinate y = Pixel_Coordinate(pos.y()
                                                  * voxel_size_reciprocal);
            Pixel_Coordinate z = Pixel_Coordinate(pos.z()
                                                  * voxel_size_reciprocal);
            
            // check if inside output volume
            if (safe_mode == false || 
                (0 <= x && 0 <= y && 0 <= z && 
                 x < (Pixel_Coordinate) output().resolution().x() && 
                 y < (Pixel_Coordinate) output().resolution().y() &&
                 z < (Pixel_Coordinate) output().resolution().z()))
            {
                if (radius_begin != radius_end)
                {
                    // interpolate radius between segment begin and end
                    Micron radius1p = (radius_begin * ( step_number - i)
                                       + radius_end * i) / step_number; 
                    Micron radius2p = (radius_begin * (step_number - 1 - i)
                                       + radius_end * (i + 1)) / step_number; 
                    volume = cone_volume(radius1p, 
                                         radius2p, 
                                         step_length);
                }
                else
                {
                    volume = cone_volume(radius_begin, 
                                         radius_end, 
                                         step_length);
                }


                std::cout << "s " << segment;

                //if (output().volume_in_voxel(x,y,z) <= voxel_volume - volume)
                //{
                // check if the volume would already fill the voxel completely
                // if this is not the same voxel as before
                if (current_voxel != Vector_3D<Pixel_Coordinate>(x,y,z))
                {
                    //// if the last voxel was filled more than possible
                    //// adjust for this
                    //if (last_x != UNDEFINED_PIXEL_COORDINATE)
                    //{
                    //    if (output().operator()(last_x, last_y, last_z).
                    //            back().volume > voxel_volume)
                    //    {
                    //        output().operator()(last_x, last_y, last_z).
                    //            back().volume = voxel_volume;
                    //    }
                    //}
                    // add a the current part of the segment to a new voxel
                    output().operator()(x, y, z).push_back(
                        Segment_Voxel_Occupancy(
                                current_global_segment_index, volume));
                }
                else
                {
                    // add the current part of the segment to the same voxel
                    output().operator()(x, y, z).back().volume += volume;
                }
                //last_x = x;
                //last_y = y;
                //last_z = z;
                //}
            }
            current_voxel = Vector_3D<Pixel_Coordinate>(x, y, z);
        } // for i to step number
        //// if the last voxel was filled more than possible
        //// adjust for this
        //if (last_x != UNDEFINED_PIXEL_COORDINATE)
        //{
        //    if (output().operator()(last_x, last_y, last_z).
        //        back().volume > voxel_volume)
        //    {
        //        output().operator()(last_x, last_y, last_z).
        //        back().volume = voxel_volume;
        //    }
        //}
    } // if length > 0
}    
    
// ----------------------------------------------------------------------------

}

#endif

