/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_BOX_H
#define BBP_BOX_H

#include "Vector_3D.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Axis aligned box (cuboid geometry).
/*!
    \ingroup Math Geometry
*/
template <typename Space_Unit>
class Box
    // : public Geometry
{
public:
    //! Create uninitialized box object.
    Box()
    {}

    //! Create initialized box object.
    Box(Space_Unit center_x, Space_Unit center_y, Space_Unit center_z,
        Space_Unit dimension_x, Space_Unit dimension_y, Space_Unit dimension_z)
        : _center(center_x, center_y, center_z),
          _dimensions(dimension_x, dimension_y, dimension_z)
    {}

    //! center point of the volume
    const Vector_3D<Space_Unit> & center() const
    {
        return _center;
    }

    //! length of the sides of the volume
    const Vector_3D<Space_Unit> & dimensions() const
    {
        return _dimensions;
    }

    //! center point of the volume
    void center(const Vector_3D<Space_Unit> & new_value)
    {
        _center = new_value;
    }

    //! length of the sides of the volume
    void dimensions(const Vector_3D<Space_Unit> & new_value)
    {
        _dimensions = new_value;
    }

    ////! Create initialized box object.
    //Box(Space_Unit minimum_x, Space_Unit minimum_y, Space_Unit minimum_z,
    //    Space_Unit maximum_x, Space_Unit maximum_y, Space_Unit maximum_z)
    //    : center(minimum_x + (maximum_x - minimum_x) / 2,
    //             minimum_y + (maximum_y - minimum_y) / 2,
    //             minimum_z + (maximum_z - minimum_z) / 2),
    //      dimensions(maximum_x - minimum_x, 
    //             maximum_y - minimum_y, 
    //             maximum_z - minimum_z)
    //{}

    //! Get minimum boundary coordinates of the box volume for each axis.
    inline Vector_3D<Space_Unit> minimum();
    //! Get maximum boundary coordinates of the box volume for each axis.
    inline Vector_3D<Space_Unit> maximum();

    //! Writes the data structure serially to a stream archive.
    template <class Archive>
    void serialize (Archive & ar, const unsigned int version)
    {
        ar & _center & _dimensions;
    }

private:
    //! center point of the volume
    Vector_3D<Space_Unit> _center;
    //! length of the sides of the volume
    Vector_3D<Space_Unit> _dimensions;
};


// ----------------------------------------------------------------------------

template <typename Space_Unit>
Vector_3D<Space_Unit> Box<Space_Unit>::minimum()
{
    return Vector_3D<Space_Unit>(center() - (dimensions() / 2));
}

// ----------------------------------------------------------------------------

template <typename Space_Unit>
Vector_3D<Space_Unit> Box<Space_Unit>::maximum()
{
    return Vector_3D<Space_Unit>(center() + (dimensions() / 2));
}

// ----------------------------------------------------------------------------

}
#endif
