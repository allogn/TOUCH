/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_CONNECTION_SCALING_H
#define BBP_CONNECTION_SCALING_H

#include <vector>

namespace bbp {

// ----------------------------------------------------------------------------

//! Fixed point real number (e.g. 2.3).
/*!
    This is currently encoded as a text string, however this can change
    at any moment.
    \ingroup Experiment_Specification
*/
typedef float			Scaling_Factor;

// ----------------------------------------------------------------------------

//! Multiplicative scaling of synapse conductance of specified connections.
/*!
    \todo Check with Srikanth and Sean for exact multiplication procedure
    and apply to microcircuit synapse conductances! (TT)
    \ingroup Experiment_Specification
*/
class Connection_Scaling
{
public:
    //! Get the name (label) of this connection scaling
	const Label &       label() const
    {
        return _label;
    }
    //! Get the source cell target of connections to be scaled.
	const Label & 	    source() const
    {
        return _source;
    }
    //! Get the destination cell target of connections to be scaled.
    const Label & destination() const
    {
        return _destination;
    }
    //! Get multiplicative factor for scaling the synapse conductance.
    /*!
        The synapse conductance is scaled.
        \todo The scaling factor should be applied to the Synapse interface
        class. (TT)
        \sa Synapse
    */
    const Scaling_Factor & factor() const
    {
        return _factor;
    }

    //! Set the name (label) of this connection scaling.
	void label(const Label & new_value)
    {
        _label = new_value;
    }
    //! Set the source cell target of connections to be scaled.
	void source(const Label & new_value)
    {
        _source = new_value;
    }
    //! Set the destination cell target of connections to be scaled.
    void destination(const Label & new_value)
    {
        _destination = new_value;
    }
    //! Set multiplicative factor for scaling the synapse conductance.
    /*!
        The synapse conductance is scaled.
        \todo The scaling factor should be applied to the Synapse interface
        class. (TT)
        \sa Synapse
    */
    void factor(Scaling_Factor new_value)
    {
        _factor = new_value;
    }

private:
    Label			    _label;
    //! The source cell target of connections to be scaled.
	Label   		    _source, 
    //! The destination cell target of connections to be scaled.
					    _destination;
    //! Multiplicative factor for scaling the synapse conductance.
    /*!
        The synapse conductance is scaled.
        \todo The scaling factor should be applied to the Synapse interface
        class. (TT)
        \sa Synapse
    */
    Scaling_Factor      _factor;
};

// ----------------------------------------------------------------------------

}
#endif
