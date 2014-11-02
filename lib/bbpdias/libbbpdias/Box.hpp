#ifndef BOX_HPP_
#define BOX_HPP_

#include "Vertex.hpp"

#include <SpatialIndex.h>

namespace dias {

/*
* Axis Aligned Box
* Structure and Basic Geometric Functions
* N-Dimensional unless Commented on the function
*
* Author: Farhan Tauheed
*/
class Box
{
public:
    Vertex low;
    Vertex high;

    Box()
        {

        }

    Box(const Vertex &low, const Vertex &high)
        {
            this->low  = low;
            this->high = high;
        }

    Box& 		   operator  =(const Box &rhs);
    bool 		   operator ==(const Box &rhs) const;
    friend std::ostream & operator << (std::ostream & lhs,const Box & rhs);

    static void getAllVertices(const Box &b, Vertex vertices[]);
    static bigSpaceUnit volume(const Box &b);
    static void center(const Box &b, Vertex &center);
    static spaceUnit length(const Box &b,const int dimension);
    static bool overlap (const Box &b1,const Box &b2);
    static bool enclose (const Box &bigger,const Box &smaller);
    static bool enclose (const Box &b,const Vertex &v);
    static void combine (const Box &b1,const Box &b2,Box &combined);
    static void coveringBox (const Vertex &center,const Vertex &radialVector, Box &box);
    static void boundingBox (Box& bb,const std::vector<Vertex> &vertices);
    static void randomBox   (const Box& world, const float percentageVolume,Box& random);
    static bigSpaceUnit overlapVolume(const Box&b1,const Box &b2);

    static dias::Box make_box (const std::vector<std::string> & coords);

    SpatialIndex::Region as_region (void) const;
};

}

#endif
