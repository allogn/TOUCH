/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SHARED_ARRAY_SLICE_DEALLOCATOR
#define BBP_SHARED_ARRAY_SLICE_DEALLOCATOR

#include <boost/shared_array.hpp>

/** \cond SHOW_IN_DEVELOPER_REFERENCE */

namespace bbp
{

//! Deleter doing nothing for passing to boost::shared_array<T> ctor.
inline void shared_array_non_deleter(const void *) {}

/*!
   Deallocator helper class for boost shared_array. 
   
   Tha main interest of this deallocator is to enable a shared array to be
   referenced at some position in the middle while accounting for that in the
   reference count. So the following safe:

   \code
   boost::shared_array<float> chunck;
   {
       boost::shared_array<float> buffer(new float[n]);
       chunk = boost::shared_array<float>
           (&buffer[p], shared_array_slice_deallocator(buffer));
   }
   \endcode
   
 */
template<typename T>
class shared_array_slice_deallocator
{
public:
    shared_array_slice_deallocator(boost::shared_array<T> &array) throw () :
        _array(array)
    {}

    // This operation needs to do nothing
    void operator()(T *p)
    {}

protected:
    boost::shared_array<T> _array;
};

}

/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

#endif
