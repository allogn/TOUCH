/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Sebastien Lasserre
                                Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_EXPERIMENT_H
#define BBP_EXPERIMENT_H

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "Experiment_Specification.h"
#include "BBP/Model/Microcircuit/Targets/Targets.h"
#include "BBP/Model/Experiment/Connection_Scaling.h"
//#include "BBP/Model/Experiment/Containers/Reports.h"
#include "Compartment_Report_Stream_Reader.h"

namespace bbp {

class Microcircuit;
typedef boost::shared_ptr<Microcircuit> Microcircuit_Ptr;
class Experiment;
typedef boost::shared_ptr<Experiment> Experiment_Ptr;

// ----------------------------------------------------------------------------

//! Interface to an experiment with a microcircuit.
/*!
    In the NEURON multi-compartment simulator version used in the 
    Blue Brain Project. Makes all the information available related
    to an experiment, such as the microcircuit model, stimuli, and 
    recordings (reports) specified. 

    \todo Add load here and open blue config also to microcircuit. (TT)
    \todo Provide a interface to replace a target label in the BlueConfigFile 
    during the loading using Experiment_Reader / Blue_Config_File_Reader. 
    (SL - 22/01/2008)
    \todo Add Stimulus class to Experiment for stimulus interfacing 
    with the simulator (TT)
    \todo Report container class for report frames (TT)
    \ingroup Experiment Laboratory
*/
class Experiment
    : public Experiment_Specification,
      private boost::noncopyable
{
    friend class Microcircuit;
    friend class Microcircuit_Reader;
    friend class Experiment_Reader;
    friend class Experiment_Accessor;
    friend std::ostream & operator << 
                (std::ostream &out, const Experiment & experiment);

public:
    //! Construct a new experiment.
    Experiment()
        : _is_open(false)
    {}

    //! Construct an experiment object and open experiment data.
    Experiment(const URI & source)
    {
        open(source);
    }

    /*!
        \name Components (read access)
        Read access to the experiment components.
    */
    //@{
    //! Get read access to microcircuit for this experiment.
    inline const Microcircuit &     microcircuit() const;
    //! Get pointer with read access to microcircuit for this experiment.
    inline Const_Microcircuit_Ptr   microcircuit_ptr() const;
    //// Get read access to experiment stimuli.
    //inline const Stimuli &        stimuli() const;
    //// Get read access to experiment recordings.
    //inline const Reports &        reports() const;
    //@}

    /*!
        \name Targets (read access)
        Read access to microcircuit targets.
    */
    //@{
    //! Get a cell target by label.
    /*!
        \deprecated Will be removed in favour of Targets::cell_target().
        Use Experiment::targets() to get access to that container.
    */
    inline Cell_Target             cell_target(const Label & target_name) const
        /* throw (Target_Not_Found) */;


    /*! \todo: user_targets should be labeled targets() and the other
        one should go protected as default_targets(). (TT) */

    //! Get read access to default targets for the microcircuit.
    inline const Targets &          targets() const;
    //! Get read access to user targets for the microcircuit.
    inline const Targets &          user_targets() const;
    //@}

    /*!
        \name Components (write access)
        Read and write access to the experiment components.
    */
    //@{
    //! Get full access to microcircuit for this experiment.
    inline Microcircuit &     microcircuit();
    //! Get pointer with full access to microcircuit for this experiment.
    inline Microcircuit_Ptr   microcircuit_ptr();

    //// Get full access to experiment stimuli.
    //inline Stimuli &        stimuli();
    //// Get full access to experiment recordings.
    //inline Reports &        reports();

    //@}
    //! \todo Implementation missing (JH)
    // /*!
    //     \name Targets (write access)
    //     Write access to microcircuit targets.
    // */
    // //@{
    // // \todo: user_targets should be labeled targets() and the other
    // // one should go protected as default_targets(). (TT)
    // //! Get full access to targets for the microcircuit.
    // inline Targets &          targets();
    // //! Get full access to user targets for the microcircuit.
    // inline Targets &          user_targets();
    //@}

//! \todo Implementation of this functions missing. Commenting out (JH)
//    /*!
//        \name Targets (write access)
//        Write access to microcircuit targets.
//    */
//    //@{
//    // \todo: user_targets should be labeled targets() and the other
//    // one should go protected as default_targets(). (TT)
//    //! Get full access to targets for the microcircuit.
//    inline Targets &          targets();
//    //! Get full access to user targets for the microcircuit.
//    inline Targets &          user_targets();
//    //@}

    /*!
        \name Dataset
        Connection and loading functions for an experiment dataset.
    */
    //@{
    //! Opens a experiment from the specified file
    /*!
        Necessary resources are allocated. User targets are loaded but no
        other data is loaded. If an experiment had already been opened it
        is closed before opening a new one. Whenever an exception occurs
        this object is left in a state equivalent to the default
        construction.
    */
    void open(const URI & source); /* throw something? */
    //! Opens a experiment from the specified file and substitutes path prefix.
    /*!
        Necessary resources are allocated. User targets are loaded but no
        other data is loaded. If an experiment had already been opened it
        is closed before opening a new one. Whenever an exception occurs
        this object is left in a state equivalent to the default
        construction.
        \param source URI specifying the source of the experiment data
        \param original_uri_prefix prefix of the URI to be replaced
        \param replacement_uri_prefix string to be used instead as URI prefix
    */
    void open(const URI & source, 
              const std::string & original_uri_prefix,
              const std::string & replacement_uri_prefix); 
        /* throw something? */
    
    //! Closes the current experiment.
    /*!
        Deallocating all resources and memory currently allocated.
    */
    void close();

    //! Check if the experiment data source is opened.
    bool is_open() const
    {
        return _is_open;
    }
    // //! Frees memory allocated by the experiment and its components
    //void clear()
    //@}
    
    // STATUS INFORMATION -----------------------------------------------------
    
    //! Prints experiment status information to the standard output.
    inline void print() const;

private:
    Microcircuit_Ptr                _microcircuit;
    Targets                         _user_targets;
    //Reports                       _reports;
    //Stimuli                       _stimuli;
    bool                            _is_open;
};

inline std::ostream & operator << (std::ostream & out,
    const Experiment & experiment)
{
    return out << static_cast<Experiment_Specification>(experiment);
}

// ----------------------------------------------------------------------------

//! Class that has access to Experiment protected attributes.
class Experiment_Accessor
{
protected:
    
    static URI & prefix_source(Experiment & experiment)
    {
        return experiment._prefix_source;
    }
    
    static URI & circuit_source(Experiment & experiment)
    {
        return experiment._circuit_source;
    }
    
    static URI & synapse_source(Experiment & experiment)
    {
        return experiment._synapse_source;
    }
    
    static URI & morphologies_source(Experiment & experiment)
    {
        return experiment._morphologies_source;
    }
    
    static URI & metypes_source(Experiment & experiment)
    {
        return experiment._metypes_source;
    }
    
    static URI & meshes_source(Experiment & experiment)
    {
        return experiment._meshes_source;
    }
    
    static URI & user_target_source(Experiment & experiment)
    {
        return experiment._user_target_source;
    }
    
    static Label & date(Experiment & experiment)
    {
        return experiment._date;
    }
    
    static Label & time(Experiment & experiment)
    {
        return experiment._time;
    }
    
    static Subversion_Link & subversion_link(Experiment & experiment)
    {
        return experiment._subversion_link;
    }
    
    static Label & label(Experiment & experiment)
    {
        return experiment._label;
    }
    
    static Text & description(Experiment & experiment)
    {
        return experiment._description;
    }  
    
    static Millisecond & duration(Experiment & experiment)
    {
        return experiment._duration;
    }  
    
    static Millisecond & forward_skip(Experiment & experiment)
    {
        return experiment._forward_skip;
    }  

    static Millisecond & timestep(Experiment & experiment)
    {
        return experiment._timestep;
    }  
    
    static Targets & user_targets(Experiment & experiment)
    {
        return experiment._user_targets;
    } 
    
    static Label & circuit_target(Experiment & experiment)
    {
        return experiment._circuit_target;
    }  
    
    static Reports_Specification & report_specs(Experiment & experiment)
    {
        return experiment._reports_specification;
    } 

    //static Connection_Scalings & 
    //    connection_scalings(Experiment & experiment)
    //{
    //    return experiment._connection_scaling_factors;
    //}  
        
};

// ----------------------------------------------------------------------------

}
#include "BBP/Model/Microcircuit/Microcircuit.h"

namespace bbp
{

// ----------------------------------------------------------------------------

Microcircuit & Experiment::microcircuit()
{
    bbp_assert(_microcircuit.get() != 0);
    return *_microcircuit.get();
}

// ----------------------------------------------------------------------------

const Microcircuit & Experiment::microcircuit() const
{
    bbp_assert(_microcircuit.get() != 0);
    return *_microcircuit.get();
}

// ----------------------------------------------------------------------------

Microcircuit_Ptr Experiment::microcircuit_ptr()
{
    bbp_assert(_microcircuit.get() != 0);
    return _microcircuit;
}

// ----------------------------------------------------------------------------

Const_Microcircuit_Ptr Experiment::microcircuit_ptr() const
{
    bbp_assert(_microcircuit.get() != 0);
    return _microcircuit;
}

// ----------------------------------------------------------------------------

const Targets & Experiment::targets() const
{
    bbp_assert(_microcircuit.get() != 0);
    return _microcircuit->targets();
}

// ----------------------------------------------------------------------------

const Targets & Experiment::user_targets() const
{
    return _user_targets;
}

// ----------------------------------------------------------------------------
//
//const Reports & Experiment::reports() const
//{
//    return _reports;
//}

// ----------------------------------------------------------------------------

Cell_Target Experiment::cell_target(const Label & target_name) const
{
    try {
        return _user_targets.cell_target(target_name);
    } catch (...) {
        if (_microcircuit.get())
        {
            return _microcircuit->cell_target(target_name);
        }
        else
        {
            throw;
        }
    }
}

// ----------------------------------------------------------------------------

void Experiment::print() const
{
    std::cout << *this << std::endl;
}

// ----------------------------------------------------------------------------

}
#endif
