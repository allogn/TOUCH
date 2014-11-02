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

#ifndef BBP_FILTER_DATA_INTERFACE_H
#define BBP_FILTER_DATA_INTERFACE_H

#include "Filter_Interface.h"

namespace bbp {

namespace RTNeuron { class SimulationDataMapper; }

class Filter_Data_Interface;
typedef boost::shared_ptr<Filter_Data_Interface> Filter_Data_Interface_Ptr;

// ----------------------------------------------------------------------------

//! Interface to stream data elements.
/*!
    \ingroup Filter_Interface
*/
class Filter_Data_Interface
{
    //enum Stream_Data_Link;
    template <typename Input, typename Output> 
    friend class Filter;
    friend class Filter_Interface;
    friend class Pipeline;
    friend class RTNeuron::SimulationDataMapper;


public:
    Filter_Data_Interface(const Label name = "") 
        : label(name), connected(false) {}
    virtual ~Filter_Data_Interface() {}

    //! ask if data is connected to the stream data object
    bool is_connected() const { return connected; }
    //! print stream data object status information
    inline virtual void print() const {}
    //! print stream data object status information to specified output stream
    inline virtual void print(std::ostream & rhs __attribute__((__unused__))) const {}
    
    //! return name of data object
    inline Label name() const;
    //! set name of data object
    inline void name(Label name);
    
    //! connect data object to input of a filter
    inline Filter_Interface & operator + (Filter_Interface & rhs);
    //! group two data objects
    virtual Filter_Data_Interface operator + (Filter_Data_Interface & rhs __attribute__((__unused__)))
    {
        throw std::logic_error("Not implemented");
    }
    
protected:
    //! name of the data object
    Label label;
    //! if data is connected to the object
    bool connected;
};

inline std::ostream & operator << 
    (std::ostream & lhs, Filter_Data_Interface & rhs);

// ----------------------------------------------------------------------------

inline Label Filter_Data_Interface::name() const
{
    return label;
}

// ----------------------------------------------------------------------------

inline void Filter_Data_Interface::name(Label name)
{
    label = name;
}

// ----------------------------------------------------------------------------

inline std::ostream & operator << 
(std::ostream & lhs, Filter_Data_Interface & rhs)
{
    rhs.print(lhs);
    return lhs;
}

// ----------------------------------------------------------------------------

inline Filter_Interface & Filter_Data_Interface::
operator + (Filter_Interface & rhs)
{

#ifndef NDEBUG
    std::cout << std::endl << "Connect stream data + filter" 
        << std::endl << "__________________________________" 
        << std::endl << std::endl;
#endif

    rhs.input_ptr(this);

#ifndef NDEBUG
    std::cout << "Connect stream data of type " << typeid(* this).name()
        << " to filter input." << std::endl;
#endif

    try
    {

#ifndef NDEBUG
        std::cout << "Connect stream data of type " << typeid(* this).name()
            << " implicitly to filter output if possible." << std::endl;
#endif

        rhs.output_ptr(this);
    }
    catch(Stream_Data_Incompatible &)
    {

#ifndef NDEBUG
        std::cout << "Connecting stream data to filter output"
            " is not possible (incompatible types)." << std::endl;
#endif

    }
    return rhs;
}

// ----------------------------------------------------------------------------

}
#endif
