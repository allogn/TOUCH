/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_FILTER_H
#define BBP_FILTER_H

#include <typeinfo>
#include <utility>
#include "Control/Filter_Interface.h"
#include "Pipeline.h"
#include "Filter_Data.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Filters a stream of objects with a fixed function.
/*! 
    A filter is a kind of processor that performs a fixed function. These 
    processing units can be chained in a serial stream to construct more 
    complex functions. This is similar to pipelines in automobile construction 
    or on modern graphics cards (GPU). 
    
    The Filter  class template implements the Filter_Interface
    and defines the input and output type of a filter. Filter is still 
    an abstract class and cannot be used directly, but all you need to do is 
    derive from this class and implement the pure virtual process() function 
    declared in the Filter_Interface class. For examples of derived 
    classes see the predefined filters in the Blue Filter library.
    \todo Add support for filter variants for more than one input and output 
    of different types.
    \sa Filter_Interface Pipeline Filter_Data_Interface 
    \sa Filter_Data
    \ingroup Pipeline
*/
template <typename Input, typename Output>
class Filter
    : public virtual Filter_Interface
{
public:
    //! blank filter without assigned input or output
    Filter();
    ////! specifies input and output of the filter
    //Filter(Input & input, Output & output);

    //! deconstructor
    virtual ~Filter();

    inline virtual void start()
    {
        initialized = false;
        Filter_Interface::start();
    }

    //! check if filter is configured
    virtual bool is_configured() const;
    //! check if filter is connected to data
    virtual bool is_connected() const;
    //! print filter result
    virtual void print() const;

    // Redeclared to suppress a SWIG warning, because otherwise there's no
    // pure virtual function in the whole interface
    virtual void process() = 0;

    //! connect two filters
    virtual Pipeline operator + (const Filter_Interface_Ptr & rhs);
    //! connect two filters
    virtual Pipeline operator + (Filter_Interface & rhs);
    //! connect a data object to the filter _output
    Filter & operator + (Filter_Data_Interface & rhs);

    inline Input & input() const
    {
        //! \todo exception handling in non release mode
        return * _input.object;
    }

    inline Output & output() const
    {
        //! \todo exception handling in non release mode
        return * _output.object;
    }

    //! if filter is connected to input data
    bool input_connected() const
    {
        return _input.is_connected();
    }

    //! if filter is connected to output data
    bool output_connected() const
    {
        return _output.is_connected();
    }

    //inline void input(const Input & new_input)
    //{
    //    //! \todo sanity check if input already exists
    //    // this is a copy, not the 
    //    * _input.object = new_input;
    //}

    //inline void output(const Output & new_output)
    //{
    //    //! \todo sanity check if input already exists
    //    * _output.object = new_output;
    //}

    //! access the input object
    /*!
        \sa Filter_Interface
    */
    inline Filter_Data<Input>  * input_ptr();
    //! access the output object
    /*!
        \sa Filter_Interface
    */
    inline Filter_Data<Output> * output_ptr();

    //! assign the filter input object
    /*!
        \sa Filter_Interface
    */
    virtual inline void input_ptr
        (Filter_Data_Interface * new_input_pointer);
    //! assign the filter output object
    /*!
        \sa Filter_Interface
    */
    virtual inline void output_ptr
        (Filter_Data_Interface * new_output_pointer);

protected:
    Filter_Data<Input>  _input;
    Filter_Data<Output> _output;
    bool initialized;

};

// ----------------------------------------------------------------------------

template <typename Output>
class Filter<void, Output>
    : public virtual Filter_Interface
{
public:
    //! blank filter without assigned input or output
    Filter();
    ////! specifies input and output of the filter
    //Filter(Input & input, Output & output);

    //! deconstructor
    virtual ~Filter();

    //! check if filter is configured
    virtual bool is_configured() const;
    //! check if filter is connected to data
    virtual bool is_connected() const;
    //! print filter result
    virtual void print() const;

    // Redeclared to suppress a SWIG warning, because otherwise there's no
    // pure virtual function in the whole interface
    virtual void process() = 0;

    //! connect two filters
    virtual Pipeline operator + (const Filter_Interface_Ptr & rhs);
    //! connect two filters
    virtual Pipeline operator + (Filter_Interface & rhs);
    //! connect a data object to the filter _output
    Filter & operator + (Filter_Data_Interface & rhs);

    inline Output & output() const
    {
        //! \todo exception handling in non release mode
        return * _output.object;
    }

    //! In this case there is no input, so this returns always true.
    bool input_connected() const
    {
        return true;
    }

    //! if filter is connected to output data
    bool output_connected() const
    {
        return _output.is_connected();
    }

    //inline void input(const Input & new_input)
    //{
    //    //! \todo sanity check if input already exists
    //    // this is a copy, not the 
    //    * _input.object = new_input;
    //}

    //inline void output(const Output & new_output)
    //{
    //    //! \todo sanity check if input already exists
    //    * _output.object = new_output;
    //}

    //! access the input object
    /*!
        \sa Filter_Interface
    */
    inline Filter_Data<void> * input_ptr();

    //! access the output object
    /*!
        \sa Filter_Interface
    */
    inline Filter_Data<Output> * output_ptr();

    //! assign the filter input object
    /*!
        \sa Filter_Interface
    */
    virtual inline void input_ptr
        (Filter_Data_Interface * new_input_pointer);
    //! assign the filter output object
    /*!
        \sa Filter_Interface
    */
    virtual inline void output_ptr
        (Filter_Data_Interface * new_output_pointer);

protected:
    /*! \todo This should be not public. I suggest having two function instead
        one that returns the Output object (usefull for warpping and another
        that returns the Filter_Data<Output> by reference. */
    Filter_Data<Output> _output;
    Filter_Data<void> _input;
    bool initialized;
};

// ----------------------------------------------------------------------------

template <typename Input>
class Filter<Input, void>
    : public virtual Filter_Interface
{
public:
    //! blank filter without assigned input or output
    Filter();
    ////! specifies input and output of the filter
    //Filter(Input & input, Output & output);

    //! deconstructor
    virtual ~Filter();

    //! check if filter is configured
    virtual bool is_configured() const;
    //! check if filter is connected to data
    virtual bool is_connected() const;
    //! print filter result
    virtual void print() const;

    // Redeclared to suppress a SWIG warning, because otherwise there's no
    // pure virtual function in the whole interface
    virtual void process() = 0;

    //! connect two filters
    virtual Pipeline operator + (const Filter_Interface_Ptr & rhs);
    //! connect two filters
    virtual Pipeline operator + (Filter_Interface & rhs);
    //! connect a data object to the filter _output
    Filter & operator + (Filter_Data_Interface & rhs);

    inline Input & input() const
    {
        //! \todo exception handling in non release mode
        return * _input.object;
    }

    //! if filter is connected to input data
    bool input_connected() const
    {
        return _input.is_connected();
    }

    //! In this case there is no output, so this returns always true.
    bool output_connected() const
    {
        return true;
    }

    //inline void input(const Input & new_input)
    //{
    //    //! \todo sanity check if input already exists
    //    // this is a copy, not the 
    //    * _input.object = new_input;
    //}

    //inline void output(const Output & new_output)
    //{
    //    //! \todo sanity check if input already exists
    //    * _output.object = new_output;
    //}

    //! access the input object
    /*!
        \sa Filter_Interface
    */
    inline Filter_Data<Input>  * input_ptr();
    //! access the output object
    /*!
        \sa Filter_Interface
    */
    inline Filter_Data<void> * output_ptr();

    //! assign the filter input object
    /*!
        \sa Filter_Interface
    */
    inline void input_ptr
        (Filter_Data_Interface * new_input_pointer);
    //! assign the filter output object
    /*!
        \sa Filter_Interface
    */
    inline void output_ptr
        (Filter_Data_Interface * new_output_pointer);

protected:
    /*! \todo This should be not public. I suggest having two function instead
        one that returns the Input object (usefull for warpping and another
        that returns the Filter_Data<Input> by reference. */
    Filter_Data<Input>  _input;
    Filter_Data<void> _output;
    bool initialized;
};

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Filter<Input, Output>::Filter()
: initialized(false)
{
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Filter<Input, Output>::~Filter() {}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
bool Filter<Input, Output>::is_configured() const
{
    if (_input.is_connected() && _output.is_connected())
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
bool Filter<Input, Output>::is_connected() const
{
    if (_input.is_connected() == true &&
        _output.is_connected() == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
inline Filter_Data<Input>  * Filter<Input, Output>::input_ptr()
{
    return & _input;
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
inline Filter_Data<Output> * Filter<Input, Output>::output_ptr()
{
    return & _output;
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
inline void Filter<Input, Output>::
input_ptr(Filter_Data_Interface * new_input_pointer)
{
    Filter_Data<Input> * temp =
        dynamic_cast<Filter_Data<Input> *>(new_input_pointer);
    if (temp != 0)
    {
        _input = * temp;
        _input.connected = true;
    }
    else
    {
#ifndef NDEBUG
        Filter_Data<Input> _;
        std::cout << "Expected: " << typeid(_).name() 
                  << " got: " << typeid(*new_input_pointer).name() << std::endl;
#endif
        throw Stream_Data_Incompatible();
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
inline void Filter<Input, Output>::
output_ptr(Filter_Data_Interface * new_output_pointer)
{
    Filter_Data<Output> * temp = 
        dynamic_cast<Filter_Data<Output> *> (new_output_pointer);
    if (temp != 0)
    {
        _output = * temp;
        _output.connected = true;
    }
    else
    {
        throw Stream_Data_Incompatible();
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Pipeline Filter<Input, Output>::
operator + (Filter_Interface & rhs)
{
    return operator +(Filter_Interface_Ptr(&rhs, shared_ptr_non_deleter));
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Pipeline Filter<Input, Output>::
operator + (const Filter_Interface_Ptr & rhs)
{
    Pipeline pipeline;

    if (rhs->input_ptr()->is_connected() == false)
    {
        if (_output.is_connected() == false)
            throw std::runtime_error("Stream data between filters is missing.");
        else
        {
            pipeline.push_back(*this);
            pipeline.push_back(rhs);
        }
    }
    else
    {
        pipeline.push_back(*this);
        pipeline.push_back(rhs);
    }
    return pipeline;
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Filter<Input, Output> & Filter<Input, Output>::
operator + (Filter_Data_Interface & rhs)
{
    Filter_Data<Output> * temp = dynamic_cast<Filter_Data<Output> *> (& rhs);
    
    if (temp == 0)
    {
        throw Stream_Data_Incompatible();
    }
    else
    {
#ifndef NDEBUG
        std::cout << "Connected filter _output to stream data of type "
            << typeid(_output).name() << std::endl;
#endif
        _output = * temp;
        _output.connected = true;
        return * this;
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
std::ostream & operator << 
(std::ostream & lhs, const Filter<Input, Output> & rhs)
{
    lhs << rhs.name() << std::endl;

    // _input
    if (rhs.input_connected() == true)
    {
        lhs << "Input: " << rhs.input();
    }
    else
    {
        lhs << "Input: -";
    }

    // _output
    if (rhs.output_connected() == true)
    {
        lhs << " Output: " << rhs.output();
    }
    else
    {
        lhs << " Output: -";
    }

    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
void Filter<Input, Output>::print() const
{
    std::cout << * this << std::endl;
}

// ----------------------------------------------------------------------------

template <typename Input>
Filter<Input, void>::Filter()
: initialized(false)
{
}

// ----------------------------------------------------------------------------

template <typename Input>
Filter<Input, void>::~Filter() {}

// ----------------------------------------------------------------------------

template <typename Input>
bool Filter<Input, void>::is_configured() const
{
    if (_input.is_connected())
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename Input>
bool Filter<Input, void>::is_connected() const
{
    if (_input.is_connected() == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename Input>
inline Filter_Data<Input>  * Filter<Input, void>::input_ptr()
{
    return & _input;
}

// ----------------------------------------------------------------------------

template <typename Input>
inline Filter_Data<void> * Filter<Input, void>::output_ptr()
{
    return 0;
}

// ----------------------------------------------------------------------------

template <typename Input>
inline void Filter<Input, void>::
input_ptr(Filter_Data_Interface * new_input_pointer)
{
    Filter_Data<Input> * temp =
        dynamic_cast<Filter_Data<Input> *>(new_input_pointer);
    if (temp != 0)
    {
        _input = * temp;
        _input.connected = true;
    }
    else
    {
#ifndef NDEBUG
        Filter_Data<Input> _;
        std::cout << "Expected: " << typeid(_).name() 
                  << " got: " << typeid(*new_input_pointer).name() << std::endl;
#endif
        throw Stream_Data_Incompatible();
    }
}

// ----------------------------------------------------------------------------

template <typename Input>
inline void Filter<Input, void>::
output_ptr(Filter_Data_Interface * new_output_pointer)
{
    throw Stream_Data_Incompatible();
}

// ----------------------------------------------------------------------------

template <typename Input>
Pipeline Filter<Input, void>::
operator + (Filter_Interface & rhs)
{
    return operator +(Filter_Interface_Ptr(&rhs, shared_ptr_non_deleter));
}

// ----------------------------------------------------------------------------

template <typename Input>
Pipeline Filter<Input, void>::
operator + (const Filter_Interface_Ptr & rhs)
{
    throw std::runtime_error("This is an end filter, cannot connect"
        " another filter to the output this kind of filter.");
}

// ----------------------------------------------------------------------------

template <typename Input>
Filter<Input, void> & Filter<Input, void>::
operator + (Filter_Data_Interface & rhs)
{
    throw Stream_Data_Incompatible();
}

// ----------------------------------------------------------------------------

template <typename Input>
std::ostream & operator << 
(std::ostream & lhs, const Filter<Input, void> & rhs)
{
    lhs << rhs.name() << std::endl;

    // _input
    if (rhs.input_connected() == true)
    {
        lhs << "Input: " << rhs.input();
    }
    else
    {
        lhs << "Input: -";
    }
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Input>
void Filter<Input, void>::print() const
{
    std::cout << * this << std::endl;
}

// ----------------------------------------------------------------------------

template <typename Output>
Filter<void, Output>::Filter()
: initialized(false)
{
}

// ----------------------------------------------------------------------------

template <typename Output>
Filter<void, Output>::~Filter() {}

// ----------------------------------------------------------------------------

template <typename Output>
bool Filter<void, Output>::is_configured() const
{
    if (_output.is_connected())
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename Output>
bool Filter<void, Output>::is_connected() const
{
    if (_output.is_connected() == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename Output>
inline Filter_Data<void>  * Filter<void, Output>::input_ptr()
{
    return 0;
}

// ----------------------------------------------------------------------------

template <typename Output>
inline Filter_Data<Output> * Filter<void, Output>::output_ptr()
{
    return & _output;
}

// ----------------------------------------------------------------------------

template <typename Output>
inline void Filter<void, Output>::
input_ptr(Filter_Data_Interface * new_input_pointer)
{
    throw Stream_Data_Incompatible();
}

// ----------------------------------------------------------------------------

template <typename Output>
inline void Filter<void, Output>::
output_ptr(Filter_Data_Interface * new_output_pointer)
{
    Filter_Data<Output> * temp = 
        dynamic_cast<Filter_Data<Output> *> (new_output_pointer);
    if (temp != 0)
    {
        _output = * temp;
        _output.connected = true;
    }
    else
    {
        throw Stream_Data_Incompatible();
    }
}

// ----------------------------------------------------------------------------

template <typename Output>
Pipeline Filter<void, Output>::
operator + (Filter_Interface & rhs)
{
    return operator +(Filter_Interface_Ptr(&rhs, shared_ptr_non_deleter));
}

// ----------------------------------------------------------------------------

template <typename Output>
Pipeline Filter<void, Output>::
operator + (const Filter_Interface_Ptr & rhs)
{
    Pipeline pipeline;

    if (_output.is_connected() == false)
        throw std::runtime_error("Stream data between filters is missing.");
    else
    {
        pipeline.push_back(*this);
        pipeline.push_back(rhs);
    }
    return pipeline;
}

// ----------------------------------------------------------------------------

template <typename Output>
Filter<void, Output> & Filter<void, Output>::
operator + (Filter_Data_Interface & rhs)
{
    Filter_Data<Output> * temp = dynamic_cast<Filter_Data<Output> *> (& rhs);
    
    if (temp == 0)
    {
        throw Stream_Data_Incompatible();
    }
    else
    {
#ifndef NDEBUG
        std::cout << "Connected filter _output to stream data of type "
            << typeid(_output).name() << std::endl;
#endif
        _output = * temp;
        _output.connected = true;
        return * this;
    }
}

// ----------------------------------------------------------------------------

template <typename Output>
std::ostream & operator << 
(std::ostream & lhs, const Filter<void, Output> & rhs)
{
    lhs << rhs.name() << std::endl;

    // _output
    if (rhs.output_connected() == true)
    {
        lhs << " Output: " << rhs.output();
    }
    else
    {
        lhs << " Output: -";
    }

    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Output>
void Filter<void, Output>::print() const
{
    std::cout << * this << std::endl;
}



// ----------------------------------------------------------------------------

///*! 
//  indicates whether the stream filter has its own, hardwired data _input and 
//  _output objects (built-in) or if they are rather linked dynamically via a 
//  pointer (plugin).
//*/
//typedef enum Stream_Data_Link
//{
//  PLUGIN_DATA,
//  BUILTIN_DATA
//};

// ----------------------------------------------------------------------------

}
#endif
