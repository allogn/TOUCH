 /*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Authors:    Thomas Traenkler
                    Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_REPORT_SPECIFICATION_H
#define BBP_REPORT_SPECIFICATION_H

#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {
    
// ----------------------------------------------------------------------------

//! Report specification class.

/*!
    This class stores the attributes of a given report as they appear in
    a BlueConfig file. An object of this type can be used to create a
    specific report reader or writer for data input/output.
    \ingroup Experiment_Specification
*/
class Report_Specification
{
public:
    //! Default constructor
    /* Default attributes: target = "Column"
                           timestep = 0.1, start time = 0, end time = undefined
                           format = hdf5, type = compartment
                           variable = V, unit = mV */
    inline Report_Specification();

    /*!
        \name Get functions
        Get report specification attributes.
    */
    //@{
    //! identifier name of the report
    inline const Label & label() const;

    //! Name of the selection of microcircuit components recorded. 
    /*! The actual interpretation of the label is report specific. For example
        for compartment reports is referes to a cell target. */
    inline const Label & target() const;

    //! Original sampling time interval of recording .
    /*! 1 / timestep = sampling rate (frames/msec) */
    inline Millisecond timestep() const;

    //! Absolute original start time of recording.
    inline Millisecond start_time() const;

    //! Absolute original end time of recording.
    /*! The [start, end) interval is open on the right. This means that
        $(end_time - start_time) / timestep = frame_count$
        Let [0, 1) be the time interval, with timestep 1, there is only one
        frame, not two.
    */
    inline Millisecond end_time() const;

    //! report format (e.g. can be HDF5 file, binary file or network stream)
    inline Report_Format format() const;

    //! report type (e.g. report compartments or report synapses)
    inline Report_Type type() const;

    //! report variable - the variable to be recorded
    inline const Report_Variable & variable() const;

    //! unit specifies the unit of the reported variable
    inline const Unit_Name & unit() const;

    //! URI to access report data from
    inline const URI & data_source() const;
    //@}

    // MODIFY ACCESS FUNCTIONS ------------------------------------------------
    
   /*!
        \name Set functions
        Set report specification attributes.
    */
    //@{
    inline void label(const Label & label);

    inline void target(const Label & target);

    inline void timestep(Millisecond timestep);

    inline void start_time(Millisecond time);

    inline void end_time(Millisecond time);

    inline void format(Report_Format format);

    inline void type(Report_Type type);

    inline void variable(const Report_Variable &variable);

    inline void unit(const Unit_Name &unit);

    inline void data_source(const URI & data_source);

    inline void print() const;
    //@}

private:
    Label            _name;
    Label            _target;

    Millisecond      _timestep;
    Millisecond      _start_time;
    Millisecond      _end_time;

    Report_Format    _format;
    Report_Type      _type;
    Report_Variable  _variable;
    Unit_Name        _unit;

    URI              _data_source;
};

// ----------------------------------------------------------------------------

inline std::ostream & operator << 
(std::ostream & lhs, const Report_Specification & rhs)
{
    lhs << "Report" << std::endl
        << "____________________________________" << std::endl
        << "label: " << rhs.label() << std::endl
        << "target: " << rhs.target() << std::endl
        << "start time: " << rhs.start_time() << std::endl
        << "end time: " << rhs.end_time() << std::endl
        << "timestep: " << rhs.timestep() << std::endl
        << "format: " << rhs.format() << std::endl
        << "type: " << rhs.type() << std::endl
        << "variable: " << rhs.variable() << std::endl
        << "unit: " << rhs.unit() << std::endl
        << "source: " << rhs.data_source() << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

inline Report_Specification::Report_Specification() :
    _target("Column"),
    _timestep(0.1f),
    _start_time(0.0f),
    _end_time(UNDEFINED_MILLISECOND),
    _format(HDF5_FORMAT),
    _type(COMPARTMENT_REPORT),
    _variable("V"),
    _unit("mV")
{}

// ----------------------------------------------------------------------------

const Label & Report_Specification::label() const
{
    return _name;
}

// ----------------------------------------------------------------------------

const Label & Report_Specification::target() const
{
    return _target;
}

// ----------------------------------------------------------------------------

Millisecond Report_Specification::timestep() const
{
    return _timestep;
}

// ----------------------------------------------------------------------------

Millisecond Report_Specification::start_time() const
{
    return _start_time;
}

// ----------------------------------------------------------------------------

Millisecond Report_Specification::end_time() const
{
    return _end_time;
}
    
// ----------------------------------------------------------------------------
    
Report_Format Report_Specification::format() const
{
    return _format;
}

// ----------------------------------------------------------------------------

Report_Type Report_Specification::type() const
{
    return _type;
}

// ----------------------------------------------------------------------------

const Report_Variable & Report_Specification::variable() const
{
    return _variable;
}

// ----------------------------------------------------------------------------

const Unit_Name & Report_Specification::unit() const
{
    return _unit;
}

// ----------------------------------------------------------------------------

const URI & Report_Specification::data_source() const
{
    return _data_source;
}

// ----------------------------------------------------------------------------

void Report_Specification::label(const Label & label) 
{
    _name = label;
}

// ----------------------------------------------------------------------------

void Report_Specification::target(const Label & target)
{
    _target = target;
}

// ----------------------------------------------------------------------------

void Report_Specification::timestep(Millisecond timestep)
{
    _timestep = timestep;
}

// ----------------------------------------------------------------------------

void Report_Specification::start_time(Millisecond time)
{
    _start_time = time;
}

// ----------------------------------------------------------------------------

void Report_Specification::end_time(Millisecond time)
{
    _end_time = time;
}
    
// ----------------------------------------------------------------------------
    
void Report_Specification::format(Report_Format format)
{
    _format = format;
}

// ----------------------------------------------------------------------------

void Report_Specification::type(Report_Type  type)
{
    _type = type;
}

// ----------------------------------------------------------------------------

void Report_Specification::variable(const Report_Variable &variable)
{
    _variable = variable;
}

// ----------------------------------------------------------------------------

void Report_Specification::unit(const Unit_Name &unit)
{
    _unit = unit;
}

// ----------------------------------------------------------------------------

void Report_Specification::data_source(const URI & source)
{
    _data_source = source;
}

// ----------------------------------------------------------------------------

void Report_Specification::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

}
#endif
