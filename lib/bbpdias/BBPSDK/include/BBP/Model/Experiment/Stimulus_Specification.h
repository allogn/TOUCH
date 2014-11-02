/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Authors:    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_STIMULUS_H
#define BBP_STIMULUS_H

#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

// ----------------------------------------------------------------------------

typedef std::string             Stimulus_Pattern;

// ----------------------------------------------------------------------------

//! Specification of stimulus protocol injected into the microcircuit.
/*!
    \ingroup Experiment_Specification
*/
class Stimulus_Specification
{
public:
    /*!
        \name Get functions.
        Get read access to specification.
    */
    //@{

    //! Get name of the stimulus.
    inline const Label  &       label() const;
    //! Get brief description of the stimulus protocol.
    inline const Text &         description() const;
    //! Get stimulus pattern being presented.
    inline Stimulus_Pattern     pattern() const;
    //! Get onset of stimulus presentation.
    inline Millisecond          onset() const;
    //! Get length of stimulus presentation.
    inline Millisecond          duration() const;
    //@}

    /*!
        \name Set functions.
        Get write access to specification.
    */
    //@{
    //! Set name of the stimulus.
    inline void                 label(Label stimulus_label);
    //! Set brief description of the stimulus protocol.
    inline void                 description(Text stimulus_description);
    //! Set stimulus pattern being presented.
    inline void                 pattern(Stimulus_Pattern type);
    //! Set onset of stimulus presentation
    inline void                 onset(Millisecond start);
    //! Set length of stimulus presentation.
    inline void                 duration(Millisecond length);
    //@}

private:
    Label                       _label;
    Text                        _description;
    Stimulus_Pattern            _pattern;
    Millisecond                 _onset;
    Millisecond                 _duration;
};

// ----------------------------------------------------------------------------

inline std::ostream & operator << 
(std::ostream & lhs, const Stimulus_Specification & rhs)
{
    lhs << "Stimulus" << std::endl
        << "____________________________________" << std::endl
        << "label: " << rhs.label() << std::endl
        << "description: " << rhs.description() << std::endl
        << "pattern: " << rhs.pattern() << std::endl
        << "onset: " << rhs.onset() << std::endl
        << "duration: " << rhs.duration() << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

const Label & Stimulus_Specification::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------

const Text & Stimulus_Specification::description() const
{
    return _description;
}

// ----------------------------------------------------------------------------

Stimulus_Pattern Stimulus_Specification::pattern() const
{
    return _pattern;
}

// ----------------------------------------------------------------------------

Millisecond Stimulus_Specification::onset() const
{
    return _onset;
}

// ----------------------------------------------------------------------------

Millisecond Stimulus_Specification::duration() const
{
    return _duration;
}

// ----------------------------------------------------------------------------

void Stimulus_Specification::label(Label stimulus_label)
{
    _label = stimulus_label;
}

// ----------------------------------------------------------------------------

void Stimulus_Specification::description(Text stimulus_description)
{
    _description = stimulus_description;
}

// ----------------------------------------------------------------------------

void Stimulus_Specification::pattern(Stimulus_Pattern type)
{
    _pattern = type;
}

// ----------------------------------------------------------------------------

void Stimulus_Specification::onset(Millisecond start)
{
    _onset = start;
}

// ----------------------------------------------------------------------------

void Stimulus_Specification::duration(Millisecond length)
{
    _duration = length;
}

// ----------------------------------------------------------------------------

}

#endif



////! depolarizing step current relative to the cell's firing threshold
//class Threshold_Stimulus
//  : public Stimulus_Specification
//{
//public:
//  Percentage                  percent_less;
//};
//
//
////! poisson distributed exponential postsynaptic potentials
//class NPoisson_Stimulus
//  : public Stimulus_Specification
//{
//public:
//    float AmpStart
//  Hertz   frequency;
//    Weight    weight
//    Count number_of_synapses;
//};
//
///*
//Stimulus_Specification poissonBackground
//{
//          Mode Current   
//       Pattern NPoisson  
//      AmpStart 0.000000  
//        Lambda 1.000000  
//        Weight 0.500000  
//    NumOfSynapses 10         
//         Delay 100.000000
//      Duration 900.000000
//}
//*/
//
////! brief current pulse
//class Pulse_Stimulus
//  : public Stimulus_Specification
//{
//public:
//{
//        Mode                    Current
//        Pattern                 Pulse
//        Nanoampere                start_current;  //TODO: check if unit is correct!
//      Nanoampere              end_current;
//      Hertz                   frequency;
//        Millisecond               width;
//};
