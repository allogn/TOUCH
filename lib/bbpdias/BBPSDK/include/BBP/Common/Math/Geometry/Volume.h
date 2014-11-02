/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VOLUME_H
#define BBP_VOLUME_H

#include "Tensor.h"
#include "Box.h"
#include "Vector_3D.h"

#ifdef BBP_USE_BOOST_SERIALIZATION
// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning ( push )
# pragma warning ( disable : 4996 )
# pragma warning ( disable : 4267 )
#endif
#include <boost/serialization/base_object.hpp>
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning ( pop )
#endif
#endif

namespace bbp {

// ----------------------------------------------------------------------------

//! Dimensions of a volume in a specified length unit.
/*!
    \ingroup Math Geometry
*/
template <typename Space_Unit>
struct Volume_Dimensions
{
    //! dimensions for each axis
    Space_Unit x, y, z;

#ifdef BBP_USE_BOOST_SERIALIZATION
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & x;
        ar & y;
        ar & z;
    }
#endif
};

// ----------------------------------------------------------------------------

//! 3D Volume data structure with elements of specified type. 
/*!
    The interface wraps a linear vector (array). Volume<double> would be a 
    3D array of double precision floating values.

    \ingroup Math Geometry
*/
template <typename Element_Type,
          typename Space_Unit = Micron, 
          typename Time_Unit = Millisecond,
          Tensor_Memory_Implementation memory = TENSOR_LINEAR_VECTOR>
class Volume
    : public Tensor <Element_Type, memory>,
      public Box <Space_Unit>
{
public:
    Volume(Count x, Count y, Count z);
    Volume();
    ~Volume();

public:

    //! Get time at which that volume snapshot was taken.
    Time_Unit time() const
    {
        return _time;
    }

    //! Set time at which that volume snapshot was taken.
    void time(Time_Unit time)
    {
        _time = time;
    }

    //! Get the dimensions of one voxel inside the volume.
    Space_Unit voxel_size() const
    {
        return _voxel_size;
    }

    //! the dimensions of one voxel inside the volume.
    void voxel_size(Space_Unit size)
    {
        _voxel_size = size;
    }

#ifdef _DOXYGEN_
    //! access element at specified location in the volume
    /*!  \todo Inherited from Tensor and non virtual in that. 
          The implementation just class Tensor::operator(Index, Index, Index) 
          and it complicates wrapping. Commented in real source. */
    Element_Type & operator () (Index x, Index y, Index z);
#endif

    //! resizes the volume
    void resize(Count x, Count y, Count z);
    void resize(Count x, Count y, Count z, Element_Type value);
    //! number of elements in the volume in total
    /*!  \todo Inherited from Tensor and non virtual in that class. 
         The implementation just calls Tensor::size(). 
         Should be here? */
    size_t size();
    //! access resolution of the tensor for all three dimensions
    /*!  \todo Inherited from Tensor and non virtual in that class.
         The implementation just calls Tensor::resolution(). 
         Should it be here? */
    const Tensor_Resolution & resolution() const;

    //! print all elements contained in this volume
    void print();

#ifdef _DOXYGEN_
    //! access voxel with linear addressing
    /*!  \todo Inherited from Tensor and non virtual in that. 
          The implementation just class Tensor::operator(Index) and it 
          complicates wrapping. Commented in real source. */
    Element_Type & operator () (Index linear_index)
    {
        return Tensor<Element_Type, memory>::operator()(linear_index);
    }
#endif

#ifdef _DOXYGEN_
    //! resolve linear addressing of specified voxel
    /*!  \todo Inherited from Tensor and non virtual in that. 
          The implementation just class Tensor::linear_index and it complicates
          wrapping. Commented in real source. */
    Index linear_index(Index x, Index y, Index z)
    {
        return Tensor<Element_Type, memory>::
            linear_index(x, y, z);
    }
#endif


private:
    //! Time at which that volume snapshot was taken.
    Time_Unit _time;

    //! the dimensions of one voxel inside the volume
    Space_Unit _voxel_size;

#ifdef BBP_USE_BOOST_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & this->center;
        ar & voxel_size;
        ar & Box <Space_Unit>::dimensions;
        ar & boost::serialization::base_object <
            Tensor < Element_Type, memory > > (*this);
    }       
#endif
};


template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
std::ostream & operator << 
    (std::ostream & lhs, 
    Volume<Element_Type, Space_Unit, Time_Unit, memory> & rhs);


#ifndef SWIG // ---------------------------------------------------------------

//! constructs a volume with zero dimensions
template <typename Element_Type,
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
Volume<Element_Type, Space_Unit, Time_Unit, memory>::Volume()
: _time(0)
{
    Box<Micron>::dimensions(Vector_3D<Micron>(0.0f, 0.0f, 0.0f));
    _voxel_size = 1.0;
    Box<Micron>::center(Vector_3D<Micron>(0.0f, 0.0f, 0.0f));
}

// ----------------------------------------------------------------------------

//! constructs a volume with the specified dimensions
template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
Volume<Element_Type, Space_Unit, Time_Unit, memory>
    ::Volume(Count x, Count y, Count z)
: _time(0)
{
    resize(x,y,z);
    _voxel_size = 1.0;
    this->Box<Micron>::center(Vector_3D<Micron>(0.0f, 0.0f, 0.0f));
}

// ----------------------------------------------------------------------------

template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
Volume<Element_Type, Space_Unit, Time_Unit, memory>::~Volume(void)
{
#ifdef BBP_DEBUG
    //! \todo remove this debug output
    std::cout << "Tensor::~Tensor()" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

//template <typename Element_Type, 
//          typename Space_Unit, 
//          typename Time_Unit,
//          Tensor_Memory_Implementation memory>
//inline
//Element_Type & Volume<Element_Type, Space_Unit, Time_Unit, memory>::operator () 
//(Index x, Index y, Index z)
//{
//    return Tensor<Element_Type, memory>::operator()(x, y, z);
//}

// ----------------------------------------------------------------------------

template <typename Element_Type,
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
inline
const Tensor_Resolution & 
Volume<Element_Type, Space_Unit, Time_Unit, memory>::resolution() const
{
    return Tensor<Element_Type, memory>::resolution();
}

// ----------------------------------------------------------------------------

template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
inline
void Volume<Element_Type, Space_Unit, Time_Unit, memory>::
resize(Count x, Count y, Count z, Element_Type value)
{
#ifdef _DEBUG
    std::cout << "Volume resized to " 
        << x << " " << y << " " << z << std::endl;
#endif
    Tensor<Element_Type, memory>::resize(x,y,z, value);
    this->dimensions(
        Vector_3D<Micron>(x * _voxel_size, 
                          y * _voxel_size, 
                          z * _voxel_size));
}

// ----------------------------------------------------------------------------

template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
inline
void Volume<Element_Type, Space_Unit, Time_Unit, memory>::
resize(Count x, Count y, Count z)
{
#ifdef _DEBUG
    std::cout << "Volume resized to " 
        << x << " " << y << " " << z << std::endl;
#endif
    Tensor<Element_Type, memory>::resize(x,y,z);

    this->dimensions(
        Vector_3D<Micron>(x * _voxel_size, 
                          y * _voxel_size, 
                          z * _voxel_size));
}

// ----------------------------------------------------------------------------

template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
void Volume<Element_Type, Space_Unit, Time_Unit, memory>::print()
{
    Tensor<Element_Type, memory>::print();
}

// ----------------------------------------------------------------------------

template <typename Element_Type,
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
inline
size_t Volume<Element_Type, Space_Unit, Time_Unit, memory>::size()
{
    return Tensor<Element_Type, TENSOR_LINEAR_VECTOR>::size();
}

// ----------------------------------------------------------------------------

template <typename Element_Type, 
          typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
std::ostream & operator << 
(std::ostream & lhs, Volume<Element_Type, Space_Unit, Time_Unit, memory> & rhs)
{
    lhs << "Volume ("
        << rhs.Box<Space_Unit>::dimensions().x() << ", "
        << rhs.Box<Space_Unit>::dimensions().y() << ", "
        << rhs.Box<Space_Unit>::dimensions().z() << ") = { ";

    for (Count z = 0; z < rhs.resolution().z(); ++z)
    for (Count y = 0; y < rhs.resolution().y(); ++y)
    for (Count x = 0; x < rhs.resolution().x(); ++x)
    {
        lhs << rhs(x,y,z) << " ";
    }
    lhs << "}" << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Space_Unit, 
          typename Time_Unit,
          Tensor_Memory_Implementation memory>
std::ostream & operator << 
(std::ostream & lhs, Volume<Byte, Space_Unit, Time_Unit, memory> & rhs)
{
    lhs << "Volume ("
        << rhs.Box <Space_Unit>::dimensions().x() << ", "
        << rhs.Box <Space_Unit>::dimensions().y() << ", "
        << rhs.Box <Space_Unit>::dimensions().z() << ") = { ";

    for (Count z = 0; z < rhs.resolution().z(); ++z)
    for (Count y = 0; y < rhs.resolution().y(); ++y)
    for (Count x = 0; x < rhs.resolution().x(); ++x)
    {
        lhs << Count (rhs(x,y,z)) << " ";
    }
    lhs << "}" << std::endl;
    return lhs;
}

#endif // SWIG ----------------------------------------------------------------

}
#endif
