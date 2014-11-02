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

#ifndef BBP_COMPARTMENT_H
#define BBP_COMPARTMENT_H

#include "Types.h"
#include "Microcircuit.h"

namespace bbp {

class Compartment_Indexer;

// ----------------------------------------------------------------------------

//! Interface to volume compartment of a neuron (simulation sampling unit).
/*!
    For all simulated variables, this is the sampling unit by which the
    simulation of the cable volume is discretized.

    \todo get cell gid section id compartment id from report frame. (TT)
    \ingroup Microcircuit
*/
class Compartment
{
    friend class Compartment_Indexer;

public:
    /** \todo review what to do with default constructor (JH) */
    /** \cond */
//    //! Construct an uninitalized compartment interface in a NEW microcircuit.
//	Compartment();
    /** \endcond */

    //! Construct an initialized compartment interface.
	inline Compartment(const Microcircuit * microcircuit,
                       Report_Frame_Index   report_frame_index,
                       Cell_Index           cell_index);

public:
    //! Get membrane potential in mV for the electrical compartment.
	inline Millivolt voltage() const;

    //! Gets the Compartment_GID for this compartment
    /** This function needs to perform some searches that depend on the size
        of the cell the compartment belongs to. Don't expect constant 
        complexity. */
    inline Compartment_GID gid() const;

    ////! Get position of the volume compartment in global coordinates.
    //inline const Vector_3D<Micron> & position() const;
    ////! Get diameter of the volume compartment.
    //inline const Micron & diameter() const;
    ////! Get length of the volume compartment.
    //inline const Micron & length() const;
    ////! Get parent section of the volume compartment.
    //inline const Section & parent() const;

protected:
    //! The global array index in the compartment report frame.
	Report_Frame_Index	 _report_frame_index;
    /*! Additional cell index inside the report frame to reduce complexity of
        methods different from voltage querying. */
    Cell_Index           _cell_index;
    //! The associated microcircuit.
    const Microcircuit * _microcircuit;
};

inline std::ostream & operator << (std::ostream & out, 
                                   const Compartment & compartment);

// ----------------------------------------------------------------------------

//inline
//Compartment::Compartment()
//{
//    // TODO: For modification support this should allocate memory for
//    // compartments not stored in a microcircuit but created globally. (TT)
//}

// ----------------------------------------------------------------------------

Compartment::Compartment(const Microcircuit * microcircuit,
                         Report_Frame_Index   report_frame_index,
                         Cell_Index           cell_index) :
   _report_frame_index(report_frame_index),
   _cell_index(cell_index),
   _microcircuit(microcircuit)
{}

// ----------------------------------------------------------------------------

Millivolt Compartment::voltage() const
{ 
    Compartment_Report_Frame<Millivolt> & voltages = 
        _microcircuit->dataset().dynamics->compartment_voltages;
    return voltages.frame()[_report_frame_index];
}

// ----------------------------------------------------------------------------

Compartment_GID Compartment::gid() const
{ 
//    Compartment_Report_Frame<Millivolt> & voltages = 
//        _microcircuit->dataset().dynamics->compartment_voltages;
    /** \todo Compartment::gid() unimplemented() */
    std::cout << "Unimplemented " << __FILE__ << ":" << __LINE__ << std::endl;
    return Compartment_GID();
}

// ----------------------------------------------------------------------------

//inline const Vector_3D<Micron> & Compartment::position() const
//{ 
//    ...
//}
//
//// ----------------------------------------------------------------------------
//
//inline const Micron & Compartment::diameter() const
//{ 
//    ...
//}
//
//// ----------------------------------------------------------------------------
//
//inline const Micron & Compartment::length() const
//{ 
//    ...
//}
//
//// ----------------------------------------------------------------------------
//
//inline const Section & Compartment::parent() const
//{ 
//    ...
//}

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & out, 
                                   const Compartment & compartment  __attribute__((__unused__)))
{
    //! \todo Should throw instead or be implemented. (TT)
    out << "Unimplemented " << __FILE__ << ':' << __LINE__ << std::endl;
    return out;
}

}
#endif
