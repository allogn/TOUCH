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

#ifndef BBP_SPECIALIZATION_SELECTION_H
#define BBP_SPECIALIZATION_SELECTION_H

#include <map>
#include <boost/shared_ptr.hpp>
#include "BBP/Common/Types.h"

/*
  This file contains several implementation classes used in the delegation
  mechanism used to hide implementation details of the various
  Compartment_Report_Reader_Impl final classes while preserving the
  templated function calls for different objects to read.
*/
namespace bbp
{

class Compartment_Report_Reader_Impl;

//-----------------------------------------------------------------------------

/*
  Index class used as key in the table storing specialization callers
*/
struct Instance_Type_Names
{
public:
    Instance_Type_Names(const std::string & final_class,
                        const std::string & template_arg) :
        _final_class_typename(final_class),
        _template_arg_typename(template_arg)
    {}

    bool operator< (const Instance_Type_Names & other) const
    {
        if (_final_class_typename < other._final_class_typename)
            return true;
        if (_final_class_typename > other._final_class_typename)
            return false;
        return _template_arg_typename < other._template_arg_typename;
    }

protected:
    std::string _final_class_typename;
    std::string _template_arg_typename;
};

//-----------------------------------------------------------------------------

/*
  Base class that implements the delegation from Compartment_Report_Reader_Impl
  calls to load_next_frame and load_frames on the proper derived class and
  template specialization.
*/
class Base_Report_Specialization_Caller
{
public :
    virtual ~Base_Report_Specialization_Caller() {}

    virtual void load_frames(Compartment_Report_Reader_Impl * reader,
                             void * buffer,
                             Frame_Number first, Frame_Number last) const
        /* throw (IO_Error, Unsupported) */ = 0;

    virtual bool load_next_frame(Compartment_Report_Reader_Impl * reader, 
                                 void * buffer) const
        /* throw (IO_Error, Unsupported) */ = 0;
};

//-----------------------------------------------------------------------------

/*
  Caller specialization for a (derived class type, buffer type) pair.
*/
template <typename T1, typename T2>
class Report_Specialization_Caller : public Base_Report_Specialization_Caller
{
public :
    void load_frames(Compartment_Report_Reader_Impl * reader, 
                     void * buffer, Frame_Number first, Frame_Number last) const
    {
        // Previous cast to void ensures buffer is pointing to the base address
        // of the most derived type, at the same time the specialization used
        // is also for the most derived type, therefore the reinterpret_cast
        // is totally safe.
        T2 * the_buffer = reinterpret_cast<T2 *>(buffer);
        T1 * the_real_reader = dynamic_cast<T1 *>(reader);
        bbp_assert(the_buffer != 0);
        bbp_assert(the_real_reader != 0);

        the_real_reader->load_frames_impl(the_buffer, first, last);
    }
    
    bool load_next_frame(Compartment_Report_Reader_Impl * reader, 
                         void * buffer) const
    {
        // Previous cast to void ensures buffer is pointing to the base address
        // of the most derived type, at the same time the specialization used
        // is also for the most derived type, therefore the reinterpret_cast
        // is totally safe.
        T2 * the_buffer = reinterpret_cast<T2 *>(buffer);
        T1 * the_real_reader = dynamic_cast<T1 *>(reader);
        bbp_assert(the_buffer != 0);
        bbp_assert(the_real_reader != 0);

        return the_real_reader->load_next_frame_impl(the_buffer);
    }
};

//-----------------------------------------------------------------------------

/*
  Specialization table.
*/
typedef std::map<Instance_Type_Names, 
                 boost::shared_ptr<Base_Report_Specialization_Caller> >
    Specialization_Table;
extern Specialization_Table global_report_reader_specialization_table;

/*
  Helper class that register a specialization in the specialization table when
  constructed. Used as a static variable inside the constructor of classes
  derived from Compartment_Report_Reader_Impl
*/
template <typename Reader_Type, typename Buffer_Type>
struct Report_Specialization_Register
{
    Report_Specialization_Register()
    {
        global_report_reader_specialization_table
            [Instance_Type_Names(typeid(Reader_Type).name(), 
                                 typeid(Buffer_Type).name())].
            reset(new Report_Specialization_Caller<Reader_Type, Buffer_Type>());
    }
};

//-----------------------------------------------------------------------------

}
#endif
