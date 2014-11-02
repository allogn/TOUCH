/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors:    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_ORIENTATION_H
#define BBP_ORIENTATION_H

#include <iostream>
#include "BBP/Common/Types.h"
#include "Vector_3D.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Orientation in 3D space (direction + rotation).
/*!
    \ingroup Math Geometry
*/
class Orientation
{
public:
    // Create orientation initialized pointing up and 0 degrees rotation.
    Orientation()
    {
        up();
    }

    // Create orientation initialized to specified value.
    Orientation(Degree rotation, const Vector_3D<float> & axis) :
        rotation(rotation),
        axis(axis)
    {}

public:
    //! Right hand thumb rule rotation around the axis defined by the vector.
    Degree    rotation;
    Vector_3D<float> axis;

public:
    //! Resets the orientation to up (0,1,0) axis 0 rotation.
    void up();
    //! Prints orientation to the console.
    void print();
};


// ----------------------------------------------------------------------------

inline
void Orientation::up()
{
    axis.x() = 0.0;
    axis.y() = 1.0;
    axis.z() = 0.0;
    rotation = 0.0;
}

// ----------------------------------------------------------------------------

inline
void Orientation::print()
{
    std::cout << "Orientation: axis (" 
        << axis.x() << ", " << axis.y() << ", " << axis.z() << ") " ;
    std::cout << "angle (" << rotation << ")" << std::endl;
}

// ----------------------------------------------------------------------------

#ifdef WIN32
#undef max
#endif

#ifdef SWIG
#define const %constant
#endif
#ifndef SWIG
const Orientation UNDEFINED_ORIENTATION = Orientation(
    0.0f,
    Vector_3D<float>(std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max())
    );
#endif
#ifdef SWIG
#undef const
#endif
}

#endif
