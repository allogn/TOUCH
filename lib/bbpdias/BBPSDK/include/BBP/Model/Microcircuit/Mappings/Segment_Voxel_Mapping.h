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

#ifndef BBP_SEGMENT_VOXEL_MAPPING_H
#define BBP_SEGMENT_VOXEL_MAPPING_H

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/Math/Geometry/Volume.h"

#ifdef BBP_USE_BOOST_SERIALIZATION
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning ( push )
# pragma warning ( disable : 4996 )
# pragma warning ( disable : 4267 )
# pragma warning ( disable : 4512 )
#endif
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning ( pop )
#endif
#endif


namespace bbp {

// ----------------------------------------------------------------------------

//! Mapping between volume elements (voxels) and microcircuit neuron segments.

class Segment_Voxel_Mapping 
	: public Volume <Segment_Indices_in_Voxel, Micron, Millisecond>
{

public:
	std::vector <Segment_GID>	segment_index_gid_lookup;

	Segment_Voxel_Mapping() {}

    //! \todo remove this debug code
    ~Segment_Voxel_Mapping()
    {
#ifdef BBP_DEBUG
        std::cout << "Segment_Voxel_Mapping::~Segment_Voxel_Mapping()" << std::endl;
#endif
    }

	inline Segments_in_Voxel segments_in_voxel(Index x, Index y, Index z);
    inline Micron3 volume_in_voxel(Index x, Index y, Index z);
    

#ifdef BBP_USE_BOOST_SERIALIZATION
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	     ar & boost::serialization::base_object < Volume < 
			 Micron, Segment_Indices_in_Voxel > > (*this);
	}
#endif

};


// ----------------------------------------------------------------------------

Segments_in_Voxel Segment_Voxel_Mapping::segments_in_voxel
    (Index x, Index y, Index z)
{
	Segments_in_Voxel voxel_segments;
    size_t number_of_segments = operator()(x,y,z).size();
 
	// iterate over all segments in this voxel
	for (size_t i = 0; i < number_of_segments; ++i)
    {
		voxel_segments.push_back(Voxel_Segment_GID_Volume(
			segment_index_gid_lookup[operator()(x,y,z)[i].segment_index], 
			operator()(x,y,z)[i].volume));
	}
    return voxel_segments;    
}

// ----------------------------------------------------------------------------
Micron3  Segment_Voxel_Mapping::volume_in_voxel
(Index x, Index y, Index z)
{
    Segment_Indices_in_Voxel & v = operator()(x,y,z);
    size_t number_of_segments = v.size();
    Micron3 volume = 0.0f;
    
    // iterate over all segments in this voxel
    for (size_t i = 0; i < number_of_segments; ++i)
    {
        volume += v[i].volume;
    }
    return volume;
}
    
// ----------------------------------------------------------------------------


    
}
#endif
