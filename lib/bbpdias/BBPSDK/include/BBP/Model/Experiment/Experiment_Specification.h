/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Sebastien Lasserre
                                Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_EXPERIMENT_SPECIFICATION_H
#define BBP_EXPERIMENT_SPECIFICATION_H

#include <iostream>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Targets/Targets.h"
#include "BBP/Model/Experiment/Containers/Connection_Scalings.h"
#include "BBP/Model/Experiment/Containers/Reports_Specification.h"
#include "BBP/Model/Experiment/Containers/Stimuli_Specification.h"
#include "BBP/Model/Experiment/Report_Specification.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Interface to a specification of an experiment.
/*!
    This class specifies experiments like currently done in the BlueConfig
    experiment configuration file.
    \ingroup Experiment_Specification
    \todo The data link information below could be bi-directional
    so should be called dataset link instead of source.
    Also, this is internal information, so it should be hidden
    from the main user interface (maybe in Experiment_Specification). (TT)
*/
class Experiment_Specification
{

public:
    // STATUS INFORMATION -----------------------------------------------------

    //! Prints experiment status information to the standard output.
    //! \todo Method declared but not implemented
    //void print() const;

    // ------------------------------------------------------------------------
    // READ ACCESS FUNCTIONS
    // ------------------------------------------------------------------------

    /*!
        \name Specification (read access)
        Get read access to the experiment specification.
    */
    //@{
 
    // SPECIFICATIONS ---------------------------------------------------------

    //! Date of the experiment
    inline const Date &             date() const;
    //! Time of the experiment
    inline const Time &             time() const;
    //! Version of the experiment in the SVN repository.
    inline const Subversion_Link &  version() const;
    //! Name of the experiment
    inline const Label &            label() const;
    //! Brief description of the experiment's rationale
    inline const Text &             description() const;
    //! Forward skip of the simulation in milliseconds 
    inline Millisecond              forward_skip() const;
    //! Duration of the simulation in milliseconds 
    inline Millisecond              duration() const;
    //! Sampling time interval (timestep / 1000 = sampling rate)
    inline Millisecond              timestep() const;
    //! Set of cells simulated in this experiment
    inline const Label &            circuit_target() const;

    //! Specifications of the stimuli.
    inline const Stimuli_Specification &       stimuli() const;
    //! Specifications of the reports.
    inline const Reports_Specification &       reports() const;
    //! Specifications of the microcircuit synapse connection scalings.
    inline const Connection_Scalings &  connection_scalings() const;
    //@}

    // ------------------------------------------------------------------------

    /*!
        \name Source (read access)
        Get read access to unified resource identifiers of data sources.
    */
    //@{
    //! URI to access the root path of the outputs.
    /*!
        \todo The prefix_source would better be called report_dataset_link,
        while the synapse_source should be called synapse_dataset_link. Also,
        a separation between structure (static) and dynamics (dynamic) data
        sources would make more sense technically. (TT)
    */
    inline const URI & prefix_source() const;
    //! URI to access mvd circuit data from.
    inline const URI & circuit_source() const;
    //! URI to access nrn (synapses) data from.
    inline const URI & synapse_source() const;
    //! URI to access morphology data from.
    inline const URI & morphologies_source() const;
    //! URI to access morho-electrical types data from.
    inline const URI & metypes_source() const;
    //! URI to access meshes data from.
    inline const URI & meshes_source() const;
    //! URI to access user target data from.
    /*!
        \todo Review this, their should only be one target set visible.
        When switching experiments, that could change, so not necessary
        to expose a specific "user" target. (TT)
        This is used inside Experiment_Reader to read particular targets from
        the experiment and nowhere else. (JH)
    */
    inline const URI & user_target_source() const; 
    //@}

    // ------------------------------------------------------------------------
    // WRITE ACCESS FUNCTIONS
    // ------------------------------------------------------------------------

    /*!
        \name Specification (write access)
        Write access to the experiment specification.
    */
    //@{
   
    // SPECIFICATIONS ---------------------------------------------------------

    //! Date of the experiment
    inline void                     date(const Date & value);
    //! Time of the experiment
    inline void                     time(const Time & value);
    //! Version of the experiment in the SVN repository.
    inline void                     version(const Subversion_Link & value);
    //! Name of the experiment
    inline void                     label(const Label & value);
    //! Brief description of the experiment's rationale
    inline void                     description(const Text & value);
    //! Forward skip of the simulation in milliseconds 
    inline void                     forward_skip(const Millisecond value);
    //! Duration of the simulation in milliseconds 
    inline void                     duration(const Millisecond value);
    //! Sampling time interval (timestep / 1000 = sampling rate)
    inline void                     timestep(const Millisecond value);
    //! Set of cells simulated in this experiment
    inline void                     circuit_target(const Label & value);
    //! Specifications of the stimuli.
    inline Stimuli_Specification &  stimuli();
    //! Specifications of the reports.
    inline Reports_Specification &  reports();
    //! Specifications of the microcircuit synapse connection scalings.
    inline Connection_Scalings &     connection_scalings();
    //@}

    // ------------------------------------------------------------------------

    /*!
        \name Source (write access)
        Write access to the unified resource identifiers of the data sources.
    */
    //@{
    /*! 
        \todo The prefix_source would better be called report_dataset_link,
        while the synapse_source should be called synapse_dataset_link. Also,
        a separation between structure (static) and dynamics (dynamic) data
        sources would make more sense technically. (TT)
    */
    //! URI to access the root path of the outputs.
    inline void         prefix_source(const URI & value);
    //! URI to access microcircuit composition (mvd) data from.
    inline void         circuit_source(const URI & value);
    //! URI to access neuron synapse (nrn) data from.
    inline void         synapse_source(const URI & value);
    //! URI to access neuron morphology data from.
    inline void         morphologies_source(const URI & value);
    //! URI to access morpho-electrical neuron types data from.
    inline void         metypes_source(const URI & value);
    //! URI to access neuron morphology meshes data from.
    inline void         meshes_source(const URI & value);
    //! URI to access user target data from.
    /*!
        \todo Review this, their should only be one target set visible.
        When switching experiments, that could change, so not necessary
        to expose a specific "user" target. (TT)
        This is used inside Experiment_Reader to read particular targets from
        the experiment and nowhere else. (JH)
    */
    inline void         user_target_source(const URI & value); 
    //@}

    //! Reset all the stored data to empty values.
    void clear()
    {
        _prefix_source          = "";
        _circuit_source         = "";
        _synapse_source         = "";
        _morphologies_source    = "";
        _metypes_source         = "";
        _meshes_source          = "";
        _user_target_source     = "";

        _subversion_link.url     ("");
        _subversion_link.revision("");
        _date                   = "";
        _time                   = "";
        _label                  = "";
        _description            = "";
        _forward_skip           = UNDEFINED_MILLISECOND;
        _duration               = UNDEFINED_MILLISECOND;
        _timestep               = UNDEFINED_MILLISECOND;
        _circuit_target         = "";
        _stimuli_specification.clear();
        _reports_specification.clear();
    }
   
protected:
    friend std::ostream & operator << 
                (std::ostream &out, 
                 const Experiment_Specification & experiment);

    URI     _prefix_source;
    URI     _circuit_source;
    URI     _synapse_source;
    URI     _morphologies_source;
    URI     _metypes_source;
    URI     _meshes_source;
    URI     _user_target_source;

    Subversion_Link                 _subversion_link;
    Date                            _date;
    Time                            _time;
    Label                           _label;
    Text                            _description;
    Millisecond                     _forward_skip;
    Millisecond                     _duration;
    Millisecond                     _timestep;
    Label                           _circuit_target;
    Stimuli_Specification           _stimuli_specification;
    Reports_Specification           _reports_specification;
    Connection_Scalings      _connection_scaling_factors;
};

inline std::ostream & operator << (std::ostream & out, 
    const Experiment_Specification & experiment_specification);


// ----------------------------------------------------------------------------

}
#include "BBP/Model/Microcircuit/Microcircuit.h"

namespace bbp
{

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & out, 
    const Experiment_Specification & experiment)
{
    out << "Experiment" << std::endl
        << "___________________________________" << std::endl
        << "Label: " << experiment._label << std::endl
        << "Description: " << experiment._description << std::endl
        << "Forward_skip: " << experiment._forward_skip << std::endl
        << "Duration: " << experiment._duration << std::endl
        << "Timestep: " << experiment._timestep << std::endl
        << "Version: " << experiment._subversion_link.url() << ':'
        << experiment._subversion_link.revision() << std::endl
        << "Circuit target: " << experiment._circuit_target << std::endl;

    return out;
}

// ----------------------------------------------------------------------------
// READ ACCESS FUNCTIONS
// ----------------------------------------------------------------------------

const Subversion_Link & Experiment_Specification::version() const
{
    return _subversion_link;
}

// ----------------------------------------------------------------------------

const Date & Experiment_Specification::date() const
{
    return _date;
}

// ----------------------------------------------------------------------------

const Time & Experiment_Specification::time() const
{
    return _time;
}

// ----------------------------------------------------------------------------

const Label & Experiment_Specification::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------

const Text & Experiment_Specification::description() const
{
    return _description;
}

// ----------------------------------------------------------------------------

Millisecond Experiment_Specification::forward_skip() const
{
    return _forward_skip;
}

// ----------------------------------------------------------------------------

Millisecond Experiment_Specification::duration() const
{
    return _duration;
}

// ----------------------------------------------------------------------------

Millisecond Experiment_Specification::timestep() const
{
    return _timestep;
}

// ----------------------------------------------------------------------------

const Label & Experiment_Specification::circuit_target() const
{
    return _circuit_target;
}

// ----------------------------------------------------------------------------

const Stimuli_Specification & 
    Experiment_Specification::stimuli() const
{
    return _stimuli_specification;
}

// ----------------------------------------------------------------------------

const Reports_Specification & 
    Experiment_Specification::reports() const
{
    return _reports_specification;
}

// ----------------------------------------------------------------------------

const Connection_Scalings & 
    Experiment_Specification::connection_scalings() const
{
    return _connection_scaling_factors;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::prefix_source() const
{
    return _prefix_source;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::circuit_source() const
{
    return _circuit_source;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::synapse_source() const
{
    return _synapse_source;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::morphologies_source() const
{
    return _morphologies_source;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::metypes_source() const
{
    return _metypes_source;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::meshes_source() const
{
    return _meshes_source;
}

// ----------------------------------------------------------------------------

const URI & Experiment_Specification::user_target_source() const
{
    return _user_target_source;
}

// ----------------------------------------------------------------------------
// WRITE ACCESS FUNCTIONS
// ----------------------------------------------------------------------------

void Experiment_Specification::version(const Subversion_Link & value)
{
    _subversion_link = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::date(const Date & value)  
{
    _date = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::time(const Time & value)  
{
    _time = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::label(const Label & value)  
{
    _label = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::description(const Text & value)  
{
    _description = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::forward_skip(const Millisecond value)  
{
    _forward_skip = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::duration(const Millisecond value)  
{
    _duration = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::timestep(const Millisecond value)  
{
    _timestep = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::circuit_target(const Label & value)
{
    _label = value;
}

// ----------------------------------------------------------------------------

Stimuli_Specification &  Experiment_Specification::stimuli()
{
    return _stimuli_specification;
}

// ----------------------------------------------------------------------------

Reports_Specification & 
Experiment_Specification::reports()
{
    return _reports_specification;
}

// ----------------------------------------------------------------------------

Connection_Scalings & 
Experiment_Specification::connection_scalings()
{
    return _connection_scaling_factors;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::prefix_source(const URI & value)
{
    _prefix_source = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::circuit_source(const URI & value)  
{
    _circuit_source = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::synapse_source(const URI & value)  
{
    _synapse_source = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::morphologies_source(const URI & value)  
{
    _morphologies_source = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::metypes_source(const URI & value)  
{
    _metypes_source = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::meshes_source(const URI & value)  
{
    _meshes_source = value;
}

// ----------------------------------------------------------------------------

void Experiment_Specification::user_target_source(const URI & value)  
{
    _user_target_source = value;
}

// ----------------------------------------------------------------------------

}
#endif
