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

#ifndef BBP_CIRCLE_H
#define BBP_CIRCLE_H

#include "Vector_3D.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Circle geometry in three-dimensional space
/*!
    \ingroup Math Geometry
*/
template <typename Space_Unit>
class Circle
    // : public Geometry
{
public:
    //! standard constructor (does not initialize data)
    Circle() {}
    //! Construct initialized circle.
    Circle( Space_Unit x, 
            Space_Unit y, 
            Space_Unit z, 
            Space_Unit radius)
    : 
        _center(x, y, z), 
        _diameter(radius * 2) 
    {}

    //! Construct initialized circle.
    Circle(const Vector_3D<Space_Unit> & center, 
           Space_Unit                    diameter)
    : 
        _center(center), 
        _diameter(diameter) 
    {}

    //! Get circle center.
    inline const Vector_3D<Space_Unit> & center() const;
    //! Get circle diameter.
    inline const Space_Unit & diameter() const;
    //! Get circle radius.
    inline const Space_Unit radius() const;

    //! Set circle center.
    inline void center(const Vector_3D<Space_Unit> & center);
    //! Set circle diameter.
    inline void diameter(Space_Unit new_diameter);
    //! Set circle radius.
    inline void radius(Space_Unit new_radius);

    //bool operator == (const Circle<Space_Unit> & rhs);

private:
    Vector_3D<Space_Unit>   _center;
    Space_Unit              _diameter;
    //! \todo Add orientation here. (TT)
};



// ----------------------------------------------------------------------------

template <typename Space_Unit>
const Space_Unit Circle<Space_Unit>::radius() const
{
    return _diameter / 2;
}

// ----------------------------------------------------------------------------

template <typename Space_Unit>
const Space_Unit & Circle<Space_Unit>::diameter() const
{
    return _diameter;
}

// ----------------------------------------------------------------------------

template <typename Space_Unit>
void Circle<Space_Unit>::radius(Space_Unit new_radius)
{
    _diameter = new_radius * 2;
}

// ----------------------------------------------------------------------------

template <typename Space_Unit>
void Circle<Space_Unit>::diameter(Space_Unit new_diameter)
{
    _diameter = new_diameter;
}

// ----------------------------------------------------------------------------

template <typename Space_Unit>
const Vector_3D<Space_Unit> & Circle<Space_Unit>::center() const
{
    return _center;
}

// ----------------------------------------------------------------------------

template <typename Space_Unit>
void Circle<Space_Unit>::center(const Vector_3D<Space_Unit> & point)
{
    _center = point;
}

// ----------------------------------------------------------------------------

//template <typename Space_Unit>
//bool Circle<Space_Unit>::operator == (const Circle<Space_Unit> & rhs)
//{
//    return (_center == rhs._center && 
//            _diameter == rhs._diameter);
//}


// ----------------------------------------------------------------------------

}
#endif
