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

#ifndef BBP_FILTER_INTERFACE_H
#define BBP_FILTER_INTERFACE_H

#include <boost/shared_ptr.hpp>
#include "Process_Interface.h"
#include "BBP/Common/Types.h"
#include "BBP/Common/Patterns/shared_ptr_helper.h"

namespace bbp  {

class Filter_Data_Interface;
class Filter_Interface;
class Pipeline;

typedef boost::shared_ptr<Filter_Interface> Filter_Interface_Ptr;

// ----------------------------------------------------------------------------

enum End_Of_Stream_Mode
{
    STOP_PIPELINE,
    WAIT_FOR_NEW_DATA
};

// ----------------------------------------------------------------------------

//! Exception indicating an attempt to connect incompatible stream data types.
class Stream_Data_Incompatible
    : public std::runtime_error
{
public:
    Stream_Data_Incompatible()
        : std::runtime_error("Stream data types incompatible.")
    {}

    ~Stream_Data_Incompatible() throw () {}
};

// ----------------------------------------------------------------------------

//! Abstract interface to stream filters.
/*!
    The filter interface class provides the stream class with the necessary
    information to run and configure the stream. It invokes the process()
    function of the filters in serial order and during configuration checks 
    for IO compatibilty of neighbouring filters ("Does the output type of the
    previous filter match the input type of the filter next in the stream?").
    \todo Check inheritance of Process_Interface and Process_Control_Interface
    for implications also with regard to public/private. (TT)
    \ingroup Pipeline
*/
class Filter_Interface
      : public virtual Process_Control_Interface,
        public virtual Process_Interface
{
    friend class Filter_Data_Interface;
    friend class Pipeline;

public:
    Filter_Interface() {}
    virtual ~Filter_Interface() {}

    //! is_configured if filter is configured
    virtual bool is_configured() const = 0;
    //! if filter is connected to data
    virtual bool is_connected() const = 0;
    //! if filter is connected to input data
    virtual bool input_connected() const = 0;
    //! if filter is connected to output data
    virtual bool output_connected() const = 0;

    //! print filter result
    /*!
        \sa Filter Pipeline
    */
    virtual void print() const { std::cout << "filter interface"; }
    //! filter name 
    virtual Label name() const { return "Stream filter"; }

    //! connect data stream to filter output
    /*!
        \todo That function cannot be virtual since some strange template
        compiler error occurs then. The full beauty follows: (TT)
        1>filter.h(78) : error C2440: 'specialization' : cannot convert from 'int' to 'bbp::Tensor_Memory_Implementation'
        1>Conversion to enumeration type requires an explicit cast (static_cast, C-style cast or function-style cast)
        1>This diagnostic occurred in the compiler generated function 'bbp::Filter_Interface &bbp::Filter<Input,Output>::operator +(bbp::Filter_Data_Interface &)'
        1>with
        1>[
        1>Input=bbp::Microcircuit,
        1>Output=bbp::Volume<bbp::Micron,bbp::Millivolt>
        1>]
    */
    Filter_Interface & operator + (Filter_Data_Interface & rhs);
    //! connect filter output to input of specified filter 
    virtual Pipeline operator + (const Filter_Interface_Ptr & rhs) = 0;
    //! connect filter output to input of specified filter 
    virtual Pipeline operator + (Filter_Interface & rhs) = 0;

    //! access the filter input pointer 
    /*!
        \sa Filter Pipeline
    */
    virtual Filter_Data_Interface * input_ptr() = 0;
    //! access the filter output pointer.
    /*!
        \sa Filter Pipeline
    */
    virtual Filter_Data_Interface * output_ptr() = 0;

    //! assign the filter input pointer 
    /*!
        \sa Filter Pipeline
    */
    virtual void input_ptr(Filter_Data_Interface * input) = 0;
    //! Assign the filter output pointer.
    /*!
        \sa Filter Pipeline
    */
    virtual void output_ptr(Filter_Data_Interface * output) = 0;

};

// ----------------------------------------------------------------------------

inline Filter_Interface_Ptr operator +(const Filter_Interface_Ptr & filter,
                                       Filter_Data_Interface & data)
{
    *filter + data;
    return filter;
}


// ----------------------------------------------------------------------------

inline Filter_Interface & Filter_Interface::
operator + (Filter_Data_Interface & rhs)
{
#ifndef NDEBUG
    std::cout << std::endl << "Connect filter + stream data" 
        << std::endl << "__________________________________" 
        << std::endl << std::endl;
#endif
    output_ptr(& rhs);
#ifndef NDEBUG
    std::cout << "Connect stream data of type " << typeid(& rhs ).name()
        << " to filter output." << std::endl;
#endif
    return * this;
}

// ----------------------------------------------------------------------------

// \todo implement this in abstract way (virtual)
//Filter_Interface group(
//  Filter_Interface & first, Filter_Interface & second)
//{
//  Filter_Data<Data_Type, Data_Context, true> group;
//  group.objects.push_back(first);
//  group.objects.push_back(second);
//  return group;
//}

// ----------------------------------------------------------------------------

}
#endif
