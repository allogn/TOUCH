// Useful definitions that depend on BBP SDK
#ifndef BBP_TOOLS_HPP_
#define BBP_TOOLS_HPP_

#include <BBP/Model/Microcircuit/Segment.h>

#include <SpatialIndex.h>

#include "tools.hpp"
#include "mbr.hpp"

namespace dias {

    typedef boost::array<bbp::Micron, 3> micron_array;

    typedef boost::array<bbp::Vertex_Index, 3> triangle_ids;

    typedef bbp::Transform_3D<bbp::Micron> global_transformer;

    typedef bbp::Array<bbp::Vector_3D<bbp::Micron> > vertex_array;

    dias::mbr get_segment_mbr (const bbp::Segment & s,
                               const dias::global_transformer & gtrafo);

    dias::vect get_segment_center (const bbp::Segment & s,
                                   const dias::global_transformer & gtrafo);

    SpatialIndex::Region
    get_transformed_cube_mbr (const micron_array & low,
                              const micron_array & high,
                              const global_transformer & itrafo);

    dias::mbr get_triangle_mbr (const triangle_ids & tids,
                                const vertex_array & vertices,
                                const global_transformer & gtrafo);

    dias::vect get_triangle_center(const dias::triangle_ids & tids,
                                   const dias::vertex_array & vertices,
                                   const dias::global_transformer & gtrafo);
}

#endif
