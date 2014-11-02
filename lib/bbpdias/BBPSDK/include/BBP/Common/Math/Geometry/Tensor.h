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

#ifndef BBP_TENSOR_H
#define BBP_TENSOR_H

#include <vector>
#include "BBP/Common/Types.h"

#ifdef BBP_USE_BOOST_SERIALIZATION
// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning ( push )
# pragma warning ( disable : 4996 )
# pragma warning ( disable : 4267 )
# pragma warning ( disable : 4512 )
#endif
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning ( pop )
#endif
#endif


namespace bbp {

// ----------------------------------------------------------------------------

//! defines the implementation of the tensor interface
enum Tensor_Memory_Implementation
{
    TENSOR_LINEAR_VECTOR,
    TENSOR_NESTED_VECTORS
};

// ----------------------------------------------------------------------------

//! Discrete 3D Tensor field.
/*!
    Storing three dimensional data in a linear array for storage efficiency 
    and direct use in Volume Rendering engine SPVolRen.
    \ingroup Math Geometry
*/
template <typename T,
          Tensor_Memory_Implementation memory = TENSOR_LINEAR_VECTOR> 
class Tensor;

// ----------------------------------------------------------------------------

//! Voxels in the volume for each dimension
/*!
    \todo Review if this should be replaced with Vector_3D or at least
          provide a restricted interface to it or vice versa. (TT)
*/
class Tensor_Resolution
{
public:
    //! Get x-axis resolution.
    const Count & x() const
    {
        return _x;
    }

    //! Get y-axis resolution.
    const Count & y() const 
    {
        return _y;
    }

    //! Get z-axis resolution.
    const Count & z() const 
    {
        return _z;
    }

    //! Set x-axis resolution.
    void x(const Count & value)
    {
        _x = value;
    }

    //! Set y-axis resolution.
    void y(const Count & value)
    {
        _y = value;
    }

    //! Set z-axis resolution.
    void z(const Count & value)
    {
        _z = value;
    }

    bool operator == (const Tensor_Resolution & rhs) const
    {
        if (_x == rhs._x    && 
            _y == rhs._y    &&
            _z == rhs._z)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool operator != (const Tensor_Resolution & rhs) const
    {
        return ! (operator == (rhs));
    }

#ifdef BBP_USE_BOOST_SERIALIZATION
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & _x & _y & _z;
    }
#endif

private:
    Count _x, _y, _z;
};

// ----------------------------------------------------------------------------

template <typename T>
class Tensor <T, TENSOR_LINEAR_VECTOR>
{
public:
    Tensor(void);
    Tensor(Count x, Count y, Count z);
    ~Tensor(void);

    //! access to the specified location in the container
    T & operator() (Index x, Index y, Index z);

    //! access resolution of the tensor for all three dimensions
    const Tensor_Resolution & resolution() const;
    //! resize the resolution of the tensor for all three dimensions
    void resize(Count x, Count y, Count z);
    //! resize the resolution of the tensor for all three dimensions 
    //! and puts the value value in each element
    void resize(Count x, Count y, Count z, T value);
    //! total number of elements in the container
    size_t size();
    
    //! print all the elements in the tensor
    void print();

    //! forward iterator to visit all the elements in the container
    class iterator;
    //! linear access to the first element in the container
    iterator begin();
    //! indicating the end of the container has been reached
    iterator end();

    //! access voxel with linear addressing
    T & operator () (Index linear_index);
    //! resolve linear addressing of specified voxel
    Index linear_index(Index x, Index y, Index z);

protected:
    //! linear array holding 3d data
    std::vector<T>  elements;   
    //! dimensions of the volume
    Tensor_Resolution tensor_resolution;

    //! initialize all elements with zero
    void initialize();

#ifdef BBP_USE_BOOST_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & tensor_resolution;
        ar & elements;
    }
#endif
};


// ----------------------------------------------------------------------------

template <typename T>
class Tensor <T, TENSOR_NESTED_VECTORS>
{
public:
    Tensor(void);
    Tensor(Count x, Count y, Count z);
    ~Tensor(void);

    //! access to the specified location in the container
    T & operator() (Index x, Index y, Index z);

    //! access resolution of the tensor for all three dimensions
    const Tensor_Resolution & resolution() const;
    //! resize the resolution of the tensor for all three dimensions
    void resize(Count x, Count y, Count z);
    //! resize the resolution of the tensor for all three dimensions 
    //! and puts the value value in each element
    void resize(Count x, Count y, Count z, T value);
    //! total number of elements in the container
    size_t size();
    
    //! print all the elements in the tensor
    void print();

protected:
    //! linear array holding 3d data
    typename std::vector<std::vector<std::vector<T > > >  elements;   
    //! dimensions of the volume
    Tensor_Resolution tensor_resolution;

    //! initialize all elements with zero
    void initialize();

#ifdef BBP_USE_BOOST_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & tensor_resolution;
        ar & elements;
    }
#endif
};



// ----------------------------------------------------------------------------

template <typename T, Tensor_Memory_Implementation memory>
std::ostream & operator << 
    (std::ostream & lhs, Tensor<T, memory> & rhs)
{
    for (Index x = 0; x < rhs.tensor_resolution.x(); ++x)
    {
        for (Index y = 0; y < rhs.tensor_resolution.y(); ++y)
        {
            for (Index z = 0; z < rhs.tensor_resolution.z(); ++z)
            {
                lhs << rhs.operator()(x,y,z) << " ";
            }
            lhs << std::endl;
        }
        lhs << std::endl;
    }
    return lhs;
}

#ifndef SWIG // ---------------------------------------------------------------

template <typename T> inline
T & Tensor<T, TENSOR_LINEAR_VECTOR>::operator () (Index linear_index)
{
    return this->elements[linear_index];
}

// ----------------------------------------------------------------------------

template <typename T> inline
Index Tensor<T, TENSOR_LINEAR_VECTOR>::linear_index(Index x, Index y, Index z)
{
    return (x + 
            y * this->tensor_resolution.x() + 
            z * this->tensor_resolution.x() * this->tensor_resolution.y());
}

// ----------------------------------------------------------------------------

template <typename T> inline
T & Tensor<T, TENSOR_LINEAR_VECTOR>::operator () (Index x, Index y, Index z)
{
    return elements[x + y * tensor_resolution.x() + 
        z* tensor_resolution.x() * tensor_resolution.y()];
}

// ----------------------------------------------------------------------------

template <typename T> inline
T & Tensor<T, TENSOR_NESTED_VECTORS>::operator () (Index x, Index y, Index z)
{
    return elements[x][y][z];
}

// ----------------------------------------------------------------------------

template <typename T> inline
Tensor<T, TENSOR_LINEAR_VECTOR>::Tensor()
{
    tensor_resolution.x(0);
    tensor_resolution.y(0);
    tensor_resolution.z(0);
}

// ----------------------------------------------------------------------------

template <typename T> inline
Tensor<T, TENSOR_NESTED_VECTORS>::Tensor()
{
    tensor_resolution.x(0);
    tensor_resolution.y(0);
    tensor_resolution.z(0);
}

// ----------------------------------------------------------------------------

template <typename T> inline
Tensor<T, TENSOR_LINEAR_VECTOR>::Tensor(Count x, Count y, Count z)
{
    this->resize(x, y, z);
}

// ----------------------------------------------------------------------------

template <typename T> inline
Tensor<T, TENSOR_NESTED_VECTORS>::Tensor(Count x, Count y, Count z)
{
    this->resize(x, y, z);
}

// ----------------------------------------------------------------------------

template <typename T> inline
Tensor<T, TENSOR_LINEAR_VECTOR>::~Tensor(void)
{
#ifdef BBP_DEBUG
    //! \todo remove this debug output
    std::cout << "Tensor::~Tensor()" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

template <typename T> inline
Tensor<T, TENSOR_NESTED_VECTORS>::~Tensor(void)
{
#ifdef BBP_DEBUG
    //! \todo remove this debug output
    std::cout << "Tensor::~Tensor()" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

template <typename T> inline 
const Tensor_Resolution & Tensor<T, TENSOR_LINEAR_VECTOR>::resolution() const
{
    return tensor_resolution;
}

// ----------------------------------------------------------------------------

template <typename T> inline 
const Tensor_Resolution & Tensor<T, TENSOR_NESTED_VECTORS>::resolution() const
{
    return tensor_resolution;
}

// ----------------------------------------------------------------------------

template <typename T>
class Tensor<T, TENSOR_LINEAR_VECTOR>::iterator 
: public std::iterator<std::forward_iterator_tag, T>
{
    typename std::vector<T>::iterator current;

public:

    iterator (typename std::vector<T>::iterator current)
        : current(current)
    {
    }

    iterator()
    {
    }

    ~iterator ()
    {
    }

    
    iterator & operator = (const iterator & rhs)
    {
        current = rhs.current;
        return (*this);
    }

    // ==
    bool operator == (const iterator & rhs) const
    {
        return (current == rhs.current);
    }

    // !=
    bool operator != (const iterator & rhs) const
    {
        return (current != rhs.current);
    }

    // ++prefix
    iterator & operator ++ ()
    {
        ++current;
        return * this;
    }

    // postfix++
    const iterator operator ++ (int)
    {
        iterator tmp(* this);
        current++;
        return tmp;
    }

    // *
    T & operator * ()
    {
        return * current;
    }

    // ->
    T * operator -> ()
    {
        return & (* current);
    }
};

// ----------------------------------------------------------------------------

template <typename T> inline
typename Tensor<T, TENSOR_LINEAR_VECTOR>::iterator 
    Tensor<T, TENSOR_LINEAR_VECTOR>::begin()
{
    return iterator(elements.begin());
}

// ----------------------------------------------------------------------------

template <typename T> inline
typename Tensor<T, TENSOR_LINEAR_VECTOR>::iterator 
    Tensor<T, TENSOR_LINEAR_VECTOR>::end()
{
    return iterator(elements.end());
}

// ----------------------------------------------------------------------------

template <typename T> inline
void Tensor<T, TENSOR_LINEAR_VECTOR>::resize
                    (Count x, Count y, Count z, T value)
{
    // if volume zero, free memory
    if (x == 0 || y == 0|| z == 0)
    {
        elements.clear();
        tensor_resolution.x(0);
        tensor_resolution.y(0);
        tensor_resolution.z(0);
    }
    // else reallocate contiguous memory
    else
    {
        elements.resize(x * y * z, value);
        tensor_resolution.x(x);
        tensor_resolution.y(y);
        tensor_resolution.z(z);
    }
}

// ----------------------------------------------------------------------------

template <typename T> inline
void Tensor<T, TENSOR_NESTED_VECTORS>::resize
                    (Count x, Count y, Count z, T value)
{
    // if volume zero, clear memory (does not free, swap would be needed)
    if (x == 0 || y == 0|| z == 0)
    {
        elements.clear();
        tensor_resolution.x(0);
        tensor_resolution.y(0);
        tensor_resolution.z(0);
    }
    // else reallocate 3D memory
    else
    {
        elements.resize(x);
        typename std::vector<std::vector<std::vector<T> > >::iterator i_end =
            elements.end();
        for (typename std::vector<std::vector<std::vector<T> > >::iterator i =
            elements.begin(); i != i_end; ++i)
        {
            i->resize(y);
            typename std::vector<std::vector<T> >::iterator j_end =
                i->end();
            for (typename std::vector<std::vector<T> >::iterator j =
                i->begin(); j != j_end; ++j)
            {
                i->resize(z, value);
            }
        }

        tensor_resolution.x(x);
        tensor_resolution.y(y);
        tensor_resolution.z(z);
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline
void Tensor<T, TENSOR_LINEAR_VECTOR>::resize(Count x, Count y, Count z)
{
    // if volume zero, free memory
    if (x == 0 || y == 0|| z == 0)
    {
        elements.clear();
        tensor_resolution.x(0);
        tensor_resolution.y(0);
        tensor_resolution.z(0);
    }
    // else reallocate contiguous memory
    else
    {
        elements.resize(x * y * z);
        tensor_resolution.x(x);
        tensor_resolution.y(y);
        tensor_resolution.z(z);
    }
}

// ----------------------------------------------------------------------------

template <typename T> inline
void Tensor<T, TENSOR_NESTED_VECTORS>::resize(Count x, Count y, Count z)
{
    // if volume zero, clear memory (does not free, swap would be needed)
    if (x == 0 || y == 0|| z == 0)
    {
        elements.clear();
        tensor_resolution.x(0);
        tensor_resolution.y(0);
        tensor_resolution.z(0);
    }
    // else reallocate 3D memory
    else
    {
        elements.resize(x);
        typename std::vector<std::vector<std::vector<T> > >::iterator i_end =
            elements.end();
        for (typename std::vector<std::vector<std::vector<T> > >::iterator i =
            elements.begin(); i != i_end; ++i)
        {
            i->resize(y);
            typename std::vector<std::vector<T> >::iterator j_end =
                i->end();
            for (typename std::vector<std::vector<T> >::iterator j =
                i->begin(); j != j_end; ++j)
            {
                i->resize(z);
            }
        }

        tensor_resolution.x(x);
        tensor_resolution.y(y);
        tensor_resolution.z(z);
    }
}

// ----------------------------------------------------------------------------

template <typename T> inline
void Tensor<T, TENSOR_LINEAR_VECTOR>::initialize()
{
    std::fill(elements.begin(), elements.end(), 0);
}

// ----------------------------------------------------------------------------

template <typename T> inline
void Tensor<T, TENSOR_NESTED_VECTORS>::initialize()
{
    Count & x_size = tensor_resolution.x();
    Count & y_size = tensor_resolution.y();

    for (Count x = 0; x < x_size; ++x)
    for (Count y = 0; y < y_size; ++y)
    {
        std::fill(elements[x][y].begin(), elements[x][y].end(), 0);
    }
}

// ----------------------------------------------------------------------------

template <typename T> inline
size_t Tensor<T, TENSOR_LINEAR_VECTOR>::size()
{
    return elements.size();
}

// ----------------------------------------------------------------------------

template <typename T> inline
size_t Tensor<T, TENSOR_NESTED_VECTORS>::size()
{
    return tensor_resolution.x() 
         * tensor_resolution.y()
         * tensor_resolution.z();
}

// ----------------------------------------------------------------------------

template <typename T>
void Tensor<T, TENSOR_LINEAR_VECTOR>::print()
{
    for (Index x = 0; x < tensor_resolution.x(); ++x)
    {
        for (Index y = 0; y < tensor_resolution.y(); ++y)
        {
            for (Index z = 0; z < tensor_resolution.z(); ++z)
            {
                std::cout << Tensor<T, TENSOR_LINEAR_VECTOR>::operator
                    ()(x,y,z) << " / ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
void Tensor<T, TENSOR_NESTED_VECTORS>::print()
{
    for (Index x = 0; x < tensor_resolution.x(); ++x)
    {
        for (Index y = 0; y < tensor_resolution.y(); ++y)
        {
            for (Index z = 0; z < tensor_resolution.z(); ++z)
            {
                std::cout << Tensor<T, TENSOR_LINEAR_VECTOR>::operator
                    ()(x,y,z) << " / ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

#endif // SWIG ----------------------------------------------------------------

}
#endif

