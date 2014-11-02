/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando Vieites
                                
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VECTOR_3D_H
#define BBP_VECTOR_3D_H

#include "BBP/Common/Exception/Exception.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


namespace bbp {

// ----------------------------------------------------------------------------

//! 3D vector class template with specifiable space unit.
/*!
    Note the x/z plane defines the horizontal plane in this coordinate space
    in the simulation y is the upward axis.

    \ingroup Math Geometry
*/
template <typename Length_Unit>
class Vector_3D
{
public:
    inline Vector_3D(void);
    //! Construct initialized 3D vector.
    inline Vector_3D(Length_Unit x, Length_Unit y, Length_Unit z);
    //! Construct initialized 3D vector.
    inline Vector_3D(const Length_Unit vector[3]);
    //! Construct initialized 2D vector (z=0).
    inline Vector_3D(Length_Unit x, Length_Unit y);
    
    /*!
        \name Get functions.
        Read access to vector components.
    */
    //@{
    //! Get x coordinate.
    inline const Length_Unit & x() const;
    //! Get y coordinate.
    inline const Length_Unit & y() const;
    //! Get z coordinate.
    inline const Length_Unit & z() const;
    //@}

    /*!
        \name Set functions.
        Write access to vector components.
    */
    //@{
    //! Set x coordinate.
    inline Length_Unit & x();
    //! Set y coordinate.
    inline Length_Unit & y();
    //! Set z coordinate.
    inline Length_Unit & z();

    //! Set x coordinate.
    inline void x(Length_Unit value)
    {
        _vector[0] = value;
    }
    
    //! Set y coordinate.
    inline void y(Length_Unit value)
    {
        _vector[1] = value;
    }
    
    //! Set z coordinate.
    inline void z(Length_Unit value)
    {
        _vector[2] = value;
    }
    //@}

    /*!
        \name Operators.
        Operations defined on a 3D vector.
    */
    //@{
    //! Product by scalar.
    inline Vector_3D                operator * (Length_Unit rhs) const;
    //! Division by scalar.
    inline Vector_3D                operator / (Length_Unit rhs) const;
    //! Dot product.
    inline Length_Unit              operator * (const Vector_3D & rhs) const;
    //! Vector product.
    inline Vector_3D<Length_Unit>   operator ^ (const Vector_3D & rhs) const;
    //! Addition between this vector and a scalar.
    inline Vector_3D                operator + (Length_Unit rhs) const;
    //! Substraction between this vector and a scalar.
    inline Vector_3D                operator - (Length_Unit rhs) const;
    //! Addition between two vectors.
    inline Vector_3D                operator + (const Vector_3D & rhs) const;
    //! Substraction of two vectors.
    inline Vector_3D                operator - (const Vector_3D & rhs) const;
    //! Addition of another vector to this vector.
    inline Vector_3D &          operator += (const Vector_3D & rhs);
    //! Substraction of another vector to this vector.
    inline Vector_3D &          operator -= (const Vector_3D & rhs);
    //! Division by scalar.
    inline Vector_3D &          operator /= (Length_Unit rhs);
    //! Compare two vectors for equality.
    inline bool                 operator == (const Vector_3D & rhs) const;
    //! Compare two vectors for inequality.
    inline bool                 operator != (const Vector_3D & rhs) const;
    //! Assign a vector to this vector.
    Vector_3D &                 operator = (const Vector_3D & rhs);

    //! Length of the vector.
    inline Length_Unit          length() const;
    //! Normalize length of vector.
    inline void                 normalize();
    //! Reset vector to reset vector (0,0,0).
    inline void                 reset();
    //@}

    //! Get read-only raw array access.
    const Length_Unit * vector() const;
    //! Print vector components.
    inline void         print();

protected:
    Length_Unit _vector[3];

private :
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & x();
        ar & y();
        ar & z();
    }

};

// ----------------------------------------------------------------------------

template <typename Length_Unit>
std::ostream & operator << 
    (std::ostream & lhs, const Vector_3D<Length_Unit> & rhs)
{
        lhs << "(" << rhs.x() << ", " << rhs.y() << ", " << rhs.z() << ")";
        return lhs;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit>::Vector_3D(void)
{
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit>::Vector_3D(const Length_Unit position[3])
{
    memcpy(_vector, position, sizeof(Length_Unit) * 3);
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit>::Vector_3D(Length_Unit x, Length_Unit y, Length_Unit z)
{
    this->x() = x;
    this->y() = y;
    this->z() = z;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit>::Vector_3D(Length_Unit x, Length_Unit y)
{
    this->x() = x;
    this->y() = y;
    z() = 0;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::
operator * (Length_Unit rhs) const
{
    Vector_3D<Length_Unit> new_vector;

    new_vector.x() = x() * rhs;
    new_vector.y() = y() * rhs;
    new_vector.z() = z() * rhs;

    return new_vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::
operator / (Length_Unit rhs) const
{
    bbp_assert(rhs != 0.0);
    Vector_3D<Length_Unit> new_vector(
        x() / rhs,
        y() / rhs,
        z() / rhs);
    return new_vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Length_Unit Vector_3D<Length_Unit>::operator * (const Vector_3D & rhs) const
{
    return ( x() * rhs.x() + 
             y() * rhs.y() + 
             z() * rhs.z() ); 
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::
operator ^ (const Vector_3D & v) const
{
    return Vector_3D<Length_Unit>(
         y() * v.z() - v.y() * z(),
        -x() * v.z() + v.x() * z(),
         x() * v.y() - v.x() * y());
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Length_Unit Vector_3D<Length_Unit>::length() const
{
    return std::sqrt( (* this) * (* this) );
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::operator + 
    (const Length_Unit rhs) const
{
    Vector_3D<Length_Unit> new_vector(
        x() + rhs,
        y() + rhs,
        z() + rhs);
    return new_vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::operator - 
    (const Length_Unit rhs) const
{
    Vector_3D<Length_Unit> new_vector(
        x() - rhs,
        y() - rhs,
        z() - rhs);
    return new_vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::operator + 
    (const Vector_3D<Length_Unit> & rhs) const
{
    Vector_3D<Length_Unit> new_vector(
        x() + rhs.x(),
        y() + rhs.y(),
        z() + rhs.z());
    return new_vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> & Vector_3D<Length_Unit>::operator += 
    (const Vector_3D<Length_Unit> & rhs)
{
    return (*this = *this + rhs);
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> Vector_3D<Length_Unit>::operator - 
    (const Vector_3D<Length_Unit> & rhs) const
{
    Vector_3D<Length_Unit> new_vector(
        x() - rhs.x(),
        y() - rhs.y(),
        z() - rhs.z());
    return new_vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> & Vector_3D<Length_Unit>::operator -= 
    (const Vector_3D<Length_Unit> & rhs)
{
    return (*this = *this - rhs);
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
bool Vector_3D<Length_Unit>::operator == 
    (const Vector_3D<Length_Unit> & rhs) const
{
    return (
        this->x() == rhs.x() && 
        this->y() == rhs.y() && 
        this->z() == rhs.z());
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
bool Vector_3D<Length_Unit>::operator != 
    (const Vector_3D<Length_Unit> & rhs) const
{
    return (
        this->x() != rhs.x() ||
        this->y() != rhs.y() || 
        this->z() != rhs.z());
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> & Vector_3D<Length_Unit>::operator = 
    (const Vector_3D<Length_Unit> & rhs)
{
    _vector[0] = rhs._vector[0];
    _vector[1] = rhs._vector[1];
    _vector[2] = rhs._vector[2];

    return *this;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
void Vector_3D<Length_Unit>::reset()
{
    memset(_vector, 0, sizeof(Length_Unit) * 3);
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
const Length_Unit & Vector_3D<Length_Unit>::x() const
{
    return _vector[0];
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Length_Unit & Vector_3D<Length_Unit>::x()
{
    return _vector[0];
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
const Length_Unit & Vector_3D<Length_Unit>::y() const
{
    return _vector[1];
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Length_Unit & Vector_3D<Length_Unit>::y()
{
    return _vector[1];
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
const Length_Unit & Vector_3D<Length_Unit>::z() const
{
    return _vector[2];
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Length_Unit & Vector_3D<Length_Unit>::z()
{
    return _vector[2];
}


// ----------------------------------------------------------------------------

template <typename Length_Unit>
const Length_Unit * Vector_3D<Length_Unit>::vector() const
{
    return _vector;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
void Vector_3D<Length_Unit>::normalize()
{
    * this /= length();
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
Vector_3D<Length_Unit> & Vector_3D<Length_Unit>::operator /= (Length_Unit rhs)
{
    // Prevent division by zero in debug mode.
    bbp_assert(rhs != 0.0);

    x() /= rhs;
    y() /= rhs;
    z() /= rhs;
    return * this;
}

// ----------------------------------------------------------------------------

template <typename Length_Unit>
void Vector_3D<Length_Unit>::print()
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

}
#endif
