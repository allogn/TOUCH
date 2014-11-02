/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
        Contributing authors:   Juan Hernando Vieites
                                Sebastien Lasserre
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SYNAPSE_H
#define BBP_SYNAPSE_H

#include <iostream>
#include "Types.h"
#include "Datasets/Structure_Dataset.h"
#include "Neuron_Point.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Neuron;
class Neurons;
class Microcircuit;

typedef boost::shared_ptr<Neuron> Neuron_Ptr;
typedef boost::shared_ptr<const Neuron> Const_Neuron_Ptr;
typedef boost::shared_ptr<Microcircuit> Microcircuit_Ptr;
typedef boost::shared_ptr<Microcircuit> Microcircuit_Ptr;

// ----------------------------------------------------------------------------

//! Interface to classification and structure of a dynamic synapse.
/*!
    The Synapse class provides access to data of a single synapse (Based 
    on Tsodyks/Markram dynamic synapse model.). It will retrieve the data
    from the raw synapse data set loaded for the related microcircuit.
    
    DISCLAIMER: In non debug compilation, for each function call, data access
    member functions won't check if the necessary data has been loaded. The 
    user has to take care of loading the datasets before using the interfaces
    to access these datasets, otherwise the behavior is undefined.

    Synapse dataset storage memory lifetime is linked to the lifetime of the 
    associated Microcircuit instance that manages the datasets.
    \ingroup Microcircuit
*/
class Synapse
{
    friend class Synapses;
    friend class Synapse_Accessor;
    friend std::ostream & operator << (std::ostream &, const Synapse &);
 
protected:
    /*!
        \todo Allow microcircuit extern use for modification support. (TT)
        CAUTION: Do NOT rely on the default constructor, which is subject to 
        change and reserved for microcircuit external modification support!
        (TT)
    */
    //! Creates new synapse and microcircuit that is deallocated with it.
    Synapse();
public:
    //! This should be used instead of default constructor for internal use!
    Synapse(Synapse_Index index);
public:

    // NOTE: Always use this constructor currently! (TT)
    //! Creates an interface to an existing synapse in specified dataset.
    Synapse(Microcircuit & microcircuit, Synapse_Index index);
    //! Synapse dataset is only deallocated if this synapse object created it.
    ~Synapse();

    // SYNAPSE (READ-ONLY) ____________________________________________________

    /*!
        \name Presynaptic
        Get presynaptic information about this synapse.
    */
    //@{
    //! Get internal index of the synapse relative inside the neuron.
    Synapse_Index index() const;
    //! Get presynaptic neuron that signals via this synapse.
    const Neuron &                      presynaptic_neuron() const;
    //! Get presynaptic neuron that signals via this synapse.
    Neuron &                            presynaptic_neuron();
    //! Get absolute synapse location on presynaptic neuron morphology.
    const Vector_3D<Micron>             presynaptic_position() const;
    //! \todo discuss if gid retrieval is needed. (TT)
    //! Get cell GID presynaptic neuron that signals via this synapse.
    Cell_GID                            presynaptic_neuron_gid() const;
    //! Get presynaptic section ID the synapse is located on.
    Section_ID                          presynaptic_section_id() const;
    //! Get presynaptic segment ID the synapse is located on.
    Segment_ID                          presynaptic_segment_id() const;
    //! Get distance inside the segment where the synapse is located on the
    //! presynaptic neuron.
    Segment_Normalized_Distance         presynaptic_segment_distance() const;
    //@}

    /*!
        \name Postsynaptic
        Get postsynaptic information about this synapse.
    */
    //@{
    //! Get postsynaptic neuron that receives via this synapse.
    const Neuron &                      postsynaptic_neuron() const;
    //! Get postsynaptic neuron that receives via this synapse.
    Neuron &                            postsynaptic_neuron();
    //! Get absolute synapse location on postsynaptic neuron morphology.
    const Vector_3D<Micron>             postsynaptic_position() const;    
    //! Get cell GID of postsynaptic neuron that receives via this synapse.
    Cell_GID                            postsynaptic_neuron_gid() const;
    //! Get postsynaptic section ID the synapse is located on.
    Section_ID                          postsynaptic_section_id() const;
    //! Get postsynaptic segment ID the synapse is located on.
    Segment_ID                          postsynaptic_segment_id() const;
    //! Get distance inside the segment where the synapse is located on the
    //! postsynaptic neuron.
    Segment_Normalized_Distance         postsynaptic_segment_distance() const; 
    //@}

    
    /*!
        \name Structure and Dynamics
        Get access to structure dynamics of this synapse.
    */
    //@{
    //! Get type id of the synapse (inhibitory < 100, excitatory >= 100).
    Synapse_Type                type() const;

    //! Get signaling delay from presynaptic soma to postsynaptic segment.
    /*!
        This includes the axonal delay, synaptic cleft and receptor delays.
        \since version 1.0.0, beta build 9
    */
    Millisecond                 delay() const;
    //! Get absolute synaptic efficacy.
    Millivolt                   efficacy() const;
    //! Get decay time constant of the postsynaptic membrane potential.
    /*!
        This variable is currently not used in BGLIB (2008-01-28). (TT/JK)
        \since version 1.0.0, beta build 9
    */
    Millisecond                 decay() const;
    //! Get conductance of the ion channels triggered by the synapse.
    /*!
        This does not take into account the synaptic scalings at this moment!
        \bug This does not take into account the synaptic scalings. (TT)
    */
    Nanosiemens                 conductance() const;
    //! Get utilization constant (neuro-transmitter release probability).
    Probability                 utilization() const;
    //! Get depression time constant.
    Millisecond_Time_Constant   depression() const;
    //! Get facilitation time constant.
    Millisecond_Time_Constant   facilitation() const;

    //! Get spine length, distance between the surface of the synapse segments.
    /*!
        \since version 1.0.0, beta build 9
    */
    Micron                      spine_length() const;
    //@}

    // SYNAPSE (MODIFIABLE) ___________________________________________________

    ///
    //  \name Classification
    //  Get modify access to classification of the synapse.
    ///
    ////@{
    //// Set type id of the synapse (inhibitory < 100, excitatory >= 100).
    //inline void         type(Synapse_Type new_type);
    ////@}
    //   
    //   /
    //  \name Dynamics
    //  Get read-only access to dynamics of the synapse.
    ///
    //   //@{
    //// Set conductance of the ion channels triggered by the synapse.
    //   inline void         conductance(Nanosiemens value);
    //   // Set utilization (neuro transmitter release probability).
    //   inline void         utilization(Probability value);
    //   // Set depression time constant.
    //inline void         depression(Millisecond_Time_Constant time_constant);
    //   // Set facilitation time constant.
    //   inline void         facilitation(Millisecond_Time_Constant time_constant);
    //// Set absolute synaptic efficacy.
    //inline void         efficacy(Millivolt voltage);
    //   // Set decay time constant of the postsynaptic membrane potential.
    ///
    //  This variable is currently not used in BGLIB (2008-01-28). (TT/JK)
    ///
    //   inline void            decay(Millisecond time_constant);
    //// Set signaling delay.
    //   inline void         delay(Millisecond delay);
    //   //@}

    //   // PRESYNAPTIC (MODIFIABLE) _______________________________________________

    //   /
    //  \name Connectivity
    //  Get modify access to connectivity of the synapse.
    ///
    //   //@{
    //   // Get presynaptic neuron that signals via this synapse (modifiable).
    //   Neuron &               presynaptic_neuron();
    //   // Set presynaptic neuron that signals via this synapse.
    //   /
    //     \todo Unimplemented. 
    //   /
    //   inline void            presynaptic_neuron(Neuron & neuron);
    // \todo Access to position on morphology (Neuron_Point) here. (TT).

    //   // POSTSYNAPTIC (MODIFIABLE) ______________________________________________

    //   // Get postsynaptic neuron that receives via this synapse (modifiable).
    //   Neuron &               postsynaptic_neuron();
    //   // Set postsynaptic neuron that receives via this synapse.
    //   /
    //     \todo Unimplemented. 
    //   /
    //   inline void            postsynaptic_neuron(Neuron & neuron);
    // \todo Access to position on morphology (Neuron_Point) here. (TT).
    //   //@}

    /*!
        \name Status
        Get status information about the synapse.
    */
    //@{
    //! Get information about loaded synapse attributes that can be accessed.
    /*!
        Before accessing or manipulating synapse attributes through this 
        interface, make sure that the associated synapse dataset is configured
        to store these objects or has these loaded already. Otherwise a call
        to a get/set function to an unloaded attribute will cause undefined
        behavior, in the best case the application crashes, in the worst you
        will not notice the problem. Therefore, test your code in debug
        mode first, since there the access is guarded by an assertion.
        The reason the loaded attributes are not checked in production mode
        is performance. Either use debug or care about the attributes loaded,
        which you can retrieve with this function before accessing.
    */
    Synapse_Attributes_Flags            attributes_loaded() const;
    //! Print the synapse information to the standard output.
    void                                print() const;
    //@}

    /*!
        \name Legacy
        These functions are deprecated and will be removed in the future, 
        consider using alternatives.
    */
    //@{
    //! Returns whether the presynaptic neuron has been loaded or not.
    /*!
        \deprecated This function is redundant since a Neuron_Not_Found
        exception is thrown by presynaptic_neuron(). Please adapt your code.
    */
    bool                                presynaptic_neuron_available() const;

    //! Returns whether the postsynaptic neuron is available or not.
    /*!
        \deprecated This function is redundant since a Neuron_Not_Found
        exception is thrown by postsynaptic_neuron(). Please adapt your code. 
    */
    bool                                postsynaptic_neuron_available() const;
    //@}

 //   //! Get presynaptic neuron pointer that signals via this synapse 
 //   //! (modifiable).
 //   Neuron_Ptr   presynaptic_neuron_ptr();
 //   //! Set presynaptic section ID the synapse is located on.
 //   void         presynaptic_section_id(Section_ID section_id);
 //   //! Set presynaptic segment ID the synapse is located on.
 //   void         presynaptic_segment_id(Segment_ID segment_id);
 //   //! Set distance inside the segment where the synapse is located on the
 //   //! presynaptic neuron.
 //   void         presynaptic_segment_distance(
 //                         Segment_Normalized_Distance segment_distance);

 //   //! Get postsynaptic neuron pointer that receives via this synapse 
 //   //! (modifiable).
 //   Neuron_Ptr   postsynaptic_neuron_ptr();
 //   //! Set postsynaptic section ID the synapse is located on.
 //   void         postsynaptic_section_id(Section_ID section_id);
 //   //! Set postsynaptic segment ID the synapse is located on.
 //   void         postsynaptic_segment_id(Segment_ID segment_id);
 //   //! Set distance inside the segment where the synapse is located on the
 //   //! postsynaptic neuron.
 //   void         postsynaptic_segment_distance(
 //                         Segment_Normalized_Distance segment_distance);

protected:
    //! Get presynaptic neuron pointer that signals via this synapse.
    Neuron_Ptr                          presynaptic_neuron_ptr();
    //! Get presynaptic neuron pointer that signals via this synapse.
    Const_Neuron_Ptr                    presynaptic_neuron_ptr() const;
    //! Get postsynaptic neuron pointer that receives via this synapse.
    Neuron_Ptr                          postsynaptic_neuron_ptr();
    //! Get postsynaptic neuron pointer that receives via this synapse.
    Const_Neuron_Ptr                    postsynaptic_neuron_ptr() const;

private:
    Microcircuit_Ptr  _microcircuit;
    Synapse_Index     _index;
};

std::ostream & operator << (std::ostream & lhs, const Synapse & rhs);

// ----------------------------------------------------------------------------

class Synapse_Accessor
{
public:
    static Synapse_Index & synapse_index(Synapse & synapse)
    {
        return synapse._index;
    }
};

}

// ----------------------------------------------------------------------------

#include "BBP/Model/Microcircuit/Datasets/Synapse_Dataset.h"
#include "BBP/Model/Microcircuit/Containers/Neurons.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"

namespace bbp
{

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & lhs, const Synapse & rhs)
{
    lhs << "Synapse " << rhs.index() << " = {";
    if (rhs.attributes_loaded() & SYNAPSE_CONNECTION)
        lhs << " pre neuron(" << rhs.presynaptic_neuron_gid()
            << ") post neuron(" << rhs.postsynaptic_neuron_gid()
            << ")";
    if (rhs.attributes_loaded() & SYNAPSE_TYPE)
        lhs << " type(" << rhs.type().description() << ")";
    if (rhs.attributes_loaded() & SYNAPSE_DYNAMICS_CONSTANTS)
        lhs << " conductance(" << rhs.conductance() << ")"
            << " utilization(" << rhs.utilization() << ")"
            << " depression(" << rhs.depression() << ")"
            << " faciliation(" << rhs.facilitation() << ")"
            << " decay(" << rhs.decay() << ")"
            << " delay(" << rhs.delay() << ")";
    if (rhs.attributes_loaded() & SYNAPSE_POSITION &&
        rhs.presynaptic_neuron_ptr() != 0 &&
        rhs.presynaptic_neuron_ptr()->morphology_loaded() &&
        rhs.postsynaptic_neuron_ptr() != 0 &&
        rhs.postsynaptic_neuron_ptr()->morphology_loaded())
    {
        Section_ID pre_section = rhs.presynaptic_section_id();
        Section_ID post_section = rhs.postsynaptic_section_id();
        Segment_ID pre_segment = rhs.presynaptic_segment_id();
        Segment_ID post_segment = rhs.postsynaptic_segment_id();
        lhs << " pre position(" << rhs.presynaptic_position() << ")"
            << " pos position(" << rhs.postsynaptic_position() << ")"
            << " pre location(sec " << pre_section << " seg " << pre_segment
            << " width " << rhs.presynaptic_neuron().morphology().diameter
                              (pre_section, pre_segment, 0) << ")"
            << " post location(sec " << post_section << " seg " << post_segment
            << " width " << rhs.postsynaptic_neuron().morphology().diameter
                              (post_section, post_segment, 0) << ")"
            << " spine length(";
        if (rhs.postsynaptic_section_id() == 0)
            lhs << "unknown, soma connection)";
        else 
        {
            Micron length = rhs.spine_length();
            lhs << (length > 2.5 ? "!!!" : "") << length 
                << (length > 2.5 ? "!!!" : "") << ")";
        }
    }
    lhs << " }";
    return lhs;
}

// ----------------------------------------------------------------------------

inline Synapse::Synapse()
    : _microcircuit(new Microcircuit())
{   
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
    _index = synapses.insert();
}

// ----------------------------------------------------------------------------

inline Synapse::Synapse(Synapse_Index index)
    : _index(index)
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
    _index = synapses.insert();
}

// ----------------------------------------------------------------------------

inline Synapse::Synapse(Microcircuit & microcircuit, 
                        Synapse_Index index)
    : _microcircuit(microcircuit._shared_this), _index(index)
{
}

// ----------------------------------------------------------------------------

inline Synapse::~Synapse()
{
    // Synapse dataset storage memory lifetime is linked to the
    // lifetime of the associated Microcircuit instance.
}

// ----------------------------------------------------------------------------

inline Synapse_Attributes_Flags Synapse::attributes_loaded() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
    return synapses.attributes();
}

// ----------------------------------------------------------------------------

inline void Synapse::print() const
{
    std::cout << * this << std::endl;
}

// ----------------------------------------------------------------------------

inline Synapse_Index Synapse::index() const
{
    return _index;    
}

// ----------------------------------------------------------------------------

inline Synapse_Type Synapse::type() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
#ifdef BBP_SAFETY_MODE
	if (! synapses.types())
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.types() != 0);
    return Synapse_Type(synapses.type(_index));
}

// ----------------------------------------------------------------------------

inline Nanosiemens Synapse::conductance() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
#ifdef BBP_SAFETY_MODE
	if (! synapses.conductances())
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.conductances() != 0);
    return synapses.conductances()[_index];
}

// ----------------------------------------------------------------------------

inline Probability Synapse::utilization() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
#ifdef BBP_SAFETY_MODE
	if (! synapses.utilizations())
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.utilizations() != 0);
    return synapses.utilizations()[_index];
}


// ----------------------------------------------------------------------------

inline Millisecond_Time_Constant Synapse::depression() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
#ifdef BBP_SAFETY_MODE
	if (! synapses.depressions())
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.depressions() != 0);
    return synapses.depressions()[_index];
}

// ----------------------------------------------------------------------------

inline Millisecond_Time_Constant Synapse::facilitation() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
#ifdef BBP_SAFETY_MODE
	if (! synapses.facilitations())
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.facilitations() != 0);
    return synapses.facilitations()[_index];
}

// ----------------------------------------------------------------------------

inline Millivolt Synapse::efficacy() const
{
	Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
#ifdef BBP_SAFETY_MODE
	if (! synapses.efficacies())
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.efficacies() != 0);
    return synapses.efficacy(_index);
}

// ----------------------------------------------------------------------------

inline Micron Synapse::spine_length() const
{
    /*!
        \todo Check if data needed for spine length is loaded. (TT)
    */
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.presynaptic_neuron_of_synapses() &&
           synapses.presynaptic_section_ids() && 
           synapses.presynaptic_segment_ids() &&
           synapses.presynaptic_segment_distances()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
	if (!(synapses.postsynaptic_neuron_of_synapses() &&
           synapses.postsynaptic_section_ids() &&
           synapses.postsynaptic_segment_ids() &&
           synapses.postsynaptic_segment_distances()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}

#endif

	bbp_assert(synapses.presynaptic_neuron_of_synapses() &&
           synapses.presynaptic_section_ids() && 
           synapses.presynaptic_segment_ids() &&
           synapses.presynaptic_segment_distances());
    bbp_assert(synapses.postsynaptic_neuron_of_synapses() &&
           synapses.postsynaptic_section_ids() &&
           synapses.postsynaptic_segment_ids() &&
           synapses.postsynaptic_segment_distances());


    Section_ID  pre_section = synapses.presynaptic_section_id(_index);
    Segment_ID  pre_segment = synapses.presynaptic_segment_id(_index);
//    Segment_Normalized_Distance pre_distance =
//        synapses.presynaptic_segment_distance(_index) /
//        presynaptic_neuron().morphology().length(pre_section, pre_segment);

    Section_ID  post_section = synapses.postsynaptic_section_id(_index);
    Segment_ID  post_segment = synapses.postsynaptic_segment_id(_index);
//    Segment_Normalized_Distance post_distance  =
//        postsynaptic_segment_distance(_index) /
//        postsynaptic_neuron().morphology().length(post_section, post_segment);

    // This calculation is less accurate than the one below but Kostas takes
    // The first point width and the width for the whole segment.
    /*!
        \todo After validation of synapse coordinates and spine lengths
        decide whether we use the accurate value or the approximation by 
        Kostas touch detection algorithm. (TT)
    */
    return (postsynaptic_position() - presynaptic_position()).length()
        - postsynaptic_neuron().morphology().diameter(post_section,
            post_segment, 0) * (Micron) 0.5
        - presynaptic_neuron().morphology().diameter(pre_section,
            pre_segment, 0) * (Micron) 0.5;
//    return (postsynaptic_position() - presynaptic_position()).norm()
//        - postsynaptic_neuron().morphology().diameter(post_section, 
//            post_segment, post_distance) * (Micron) 0.5
//        - presynaptic_neuron().morphology().diameter(pre_section,
//            pre_segment, pre_distance) * (Micron) 0.5;
}

// ----------------------------------------------------------------------------

inline bool Synapse::presynaptic_neuron_available() const
{
    return _microcircuit->neuron_ptr(presynaptic_neuron_gid()).get() != 0;
}

// ----------------------------------------------------------------------------

inline const Neuron & Synapse::presynaptic_neuron() const
{
    return _microcircuit->neuron(presynaptic_neuron_gid());
}

// ----------------------------------------------------------------------------

inline Neuron & Synapse::presynaptic_neuron()
{
    return _microcircuit->neuron(presynaptic_neuron_gid());
}

// ----------------------------------------------------------------------------

inline Neuron_Ptr Synapse::presynaptic_neuron_ptr()
{
    return _microcircuit->neuron_ptr(presynaptic_neuron_gid());
}

// ----------------------------------------------------------------------------

inline Const_Neuron_Ptr Synapse::presynaptic_neuron_ptr() const
{
    return _microcircuit->neuron_ptr(presynaptic_neuron_gid());
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> Synapse::presynaptic_position() const
{
    /*!
        \todo Check if info needed for synapse positions is 
        loaded in the dataset. (TT)
    */
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.presynaptic_neuron_of_synapses() &&
           synapses.presynaptic_section_ids() && 
           synapses.presynaptic_segment_ids() &&
           synapses.presynaptic_segment_distances()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif

    bbp_assert(synapses.presynaptic_neuron_of_synapses() &&
           synapses.presynaptic_section_ids() && 
           synapses.presynaptic_segment_ids() &&
           synapses.presynaptic_segment_distances());
    
	Section_ID  section = synapses.presynaptic_section_id(_index);
    Segment_ID  segment = synapses.presynaptic_segment_id(_index);
    Segment_Normalized_Distance distance =
        synapses.presynaptic_segment_distance(_index) /
        presynaptic_neuron().morphology().length(section, segment);

    /*!
        \todo Deriving the position each time is not a very efficient solution, 
        could be done during loading instead, but that would require thorough 
        thinking about the data loading dependencies and storage requirements.
        For now, a quick solution is preferred to be able to provide a 
        functional library in the near term. (TT/2008-04-16)
    */
    // Get global synapse position from dataset.
    return presynaptic_neuron().position(section, segment, distance);
}

// ----------------------------------------------------------------------------

inline Cell_GID Synapse::presynaptic_neuron_gid() const
{
    bbp_assert(_microcircuit->dataset().structure->
        synapse.presynaptic_neuron_of_synapses() != 0);
    return _microcircuit->dataset().structure->
        synapse.presynaptic_neuron_of_synapse(_index);
}

// ----------------------------------------------------------------------------

inline Section_ID Synapse::presynaptic_section_id() const
{
    return _microcircuit->dataset().structure->synapse.
        presynaptic_section_id(_index);
}

// ----------------------------------------------------------------------------

inline Segment_ID Synapse::presynaptic_segment_id() const
{
    return _microcircuit->dataset().structure->synapse.
        presynaptic_segment_id(_index);
}

// ----------------------------------------------------------------------------

inline Segment_Normalized_Distance Synapse::
                presynaptic_segment_distance() const
{
    Synapse_Dataset & dataset = _microcircuit->dataset().structure->synapse;
    Section_ID section = dataset.presynaptic_section_id(_index);
    Segment_ID segment = dataset.presynaptic_segment_id(_index);

    //! \bug additional check for cero length segments which is not desirable
    float length = presynaptic_neuron().morphology().length(section, segment);
    if (length == 0)
        return dataset.presynaptic_segment_distance(_index);
    else
    {
        Segment_Normalized_Distance result = 
            dataset.presynaptic_segment_distance(_index) / length;
        if (result <= 1.0f)
            return result;
        else
            return 1.0f;   
    }
}

// ----------------------------------------------------------------------------

inline bool Synapse::postsynaptic_neuron_available() const
{
    return _microcircuit->neuron_ptr(postsynaptic_neuron_gid()).get() != 0;
}

// ----------------------------------------------------------------------------

inline Neuron & Synapse::postsynaptic_neuron()
{
    return _microcircuit->neuron(postsynaptic_neuron_gid());
}

// ----------------------------------------------------------------------------

inline const Neuron & Synapse::postsynaptic_neuron() const
{
    return _microcircuit->neuron(postsynaptic_neuron_gid());
}

// ----------------------------------------------------------------------------

inline Neuron_Ptr Synapse::postsynaptic_neuron_ptr()
{
    return _microcircuit->neuron_ptr(postsynaptic_neuron_gid()); 
}

// ----------------------------------------------------------------------------

inline Const_Neuron_Ptr Synapse::postsynaptic_neuron_ptr() const
{
    return _microcircuit->neuron_ptr(postsynaptic_neuron_gid()); 
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> Synapse::postsynaptic_position() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;
    /*!
        \todo Check if info needed for synapse positions is 
        loaded in the dataset. (TT)
    */
#ifdef BBP_SAFETY_MODE
	if (!(synapses.postsynaptic_neuron_of_synapses() &&
           synapses.postsynaptic_section_ids() &&
           synapses.postsynaptic_segment_ids() &&
           synapses.postsynaptic_segment_distances()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif

    bbp_assert(synapses.postsynaptic_neuron_of_synapses() &&
		   synapses.postsynaptic_section_ids() &&
           synapses.postsynaptic_segment_ids() &&
           synapses.postsynaptic_segment_distances());

    Section_ID  section = synapses.postsynaptic_section_id(_index);
    Segment_ID  segment = synapses.postsynaptic_segment_id(_index);
    Segment_Normalized_Distance distance =
        synapses.postsynaptic_segment_distance(_index) /
        postsynaptic_neuron().morphology().length(section, segment);

    /*!
        \todo Deriving the position each time is not a very efficient solution, 
        could be done during loading instead, but that would require thorough 
        thinking about the data loading dependencies and storage requirements.
        For now, a quick solution is preferred to be able to provide a 
        functional library in the near term. (TT/2008-04-16)
    */
    // Get global synapse position from dataset.
    return postsynaptic_neuron().position(section, segment, distance);
}

// ----------------------------------------------------------------------------

inline Cell_GID Synapse::postsynaptic_neuron_gid() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.postsynaptic_neuron_of_synapses()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(synapses.postsynaptic_neuron_of_synapses() != 0);

    return synapses.postsynaptic_neuron_of_synapse(_index);
}

// ----------------------------------------------------------------------------

inline Section_ID Synapse::postsynaptic_section_id() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.postsynaptic_section_ids()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
 
    return synapses.postsynaptic_section_id(_index);
}

// ----------------------------------------------------------------------------

inline Segment_ID Synapse::postsynaptic_segment_id() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.postsynaptic_segment_ids()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif

    return synapses.postsynaptic_segment_id(_index);
}

// ----------------------------------------------------------------------------

inline Segment_Normalized_Distance Synapse::
    postsynaptic_segment_distance() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

    Section_ID section = synapses.postsynaptic_section_id(_index);
    Segment_ID segment = synapses.postsynaptic_segment_id(_index);

    //! \bug additional check for zero length segments which is not desirable
    Micron length = postsynaptic_neuron().
                        morphology().length(section, segment);
    if (length == 0)
        return synapses.postsynaptic_segment_distance(_index);
    else
        return synapses.postsynaptic_segment_distance(_index) / length;
}

// ----------------------------------------------------------------------------

inline Millisecond Synapse::decay() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.decays()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif

    bbp_assert(synapses.decays() != 0);
    return synapses.decay(_index);
}

// ----------------------------------------------------------------------------

inline Millisecond Synapse::delay() const
{
    Synapse_Dataset & synapses = _microcircuit->dataset().structure->synapse;

#ifdef BBP_SAFETY_MODE
	if (!(synapses.delays()))
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif

    bbp_assert(synapses.delays() != 0);
    return synapses.delay(_index);
}

//// ----------------------------------------------------------------------------
//
//inline void Synapse::type(Synapse_Type new_type)
//{
//    bbp_assert(_microcircuit->dataset().structure->synapse.types());
//    _microcircuit->dataset().structure->synapse._types[_index] = new_type;
//}
//// ----------------------------------------------------------------------------
//
//inline void Synapse::conductance(Nanosiemens value)
//{
//    bbp_assert(_microcircuit->dataset().structure->synapse.conductances());
//  _microcircuit->dataset().structure->synapse._conductances[_index] = value;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::utilization(Probability value)
//{
//    bbp_assert(_microcircuit->dataset().structure->synapse.utilizations());
//  _microcircuit->dataset().structure->synapse._utilizations[_index] = value;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::depression(Millisecond_Time_Constant time_constant)
//{
//    bbp_assert(_microcircuit->dataset().structure->synapse.depressions());
//  _microcircuit->dataset().structure->synapse._depressions[_index] = 
//        time_constant;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::facilitation(Millisecond_Time_Constant time_constant)
//{
//    bbp_assert(_microcircuit->dataset().structure->synapse.facilitations());
//  _microcircuit->dataset().structure->synapse._facilitations[_index] = 
//        time_constant;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::efficacy(Millivolt voltage)
//{
//    bbp_assert(_microcircuit->dataset().structure->
//        synapse.efficacies());
//    _microcircuit->dataset().structure->
//        synapse._efficacies[_index] = voltage;
//}
//
//
//// ----------------------------------------------------------------------------
//
//inline Neuron & Synapse::presynaptic_neuron()
//{
//  return _microcircuit->neuron(presynaptic_neuron_gid());
//}
//
//// ----------------------------------------------------------------------------
//
//Neuron_Ptr Synapse::presynaptic_neuron_ptr()
//{
//  return _microcircuit->neuron_ptr(presynaptic_neuron_gid());
//}
//
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::presynaptic_section_id(Section_ID section_id)
//{
//  _microcircuit->dataset().structure->synapse.
//        presynaptic_section_id(_index) = section_id;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::presynaptic_segment_id(Segment_ID segment_id)
//{
//  _microcircuit->dataset().structure->synapse.
//        presynaptic_segment_id(_index) = segment_id;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::presynaptic_segment_distance(Segment_Normalized_Distance 
//                                           segment_distance)
//{
//  _microcircuit->dataset().structure->synapse.
//        presynaptic_segment_distance(_index) = segment_distance;
//}
//
//// ----------------------------------------------------------------------------
//
//inline Neuron & Synapse::postsynaptic_neuron()
//{
//  return _microcircuit->neuron(postsynaptic_neuron_gid());
//}
//
//// ----------------------------------------------------------------------------
//
//inline Neuron_Ptr Synapse::postsynaptic_neuron_ptr()
//{
//  return _microcircuit->neuron_ptr(postsynaptic_neuron_gid());    
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::postsynaptic_section_id(Section_ID section_id)
//{
//  _microcircuit->dataset().structure->synapse.
//        postsynaptic_section_id(_index) = section_id;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::postsynaptic_segment_id(Segment_ID segment_id)
//{
//  _microcircuit->dataset().structure->synapse.
//        postsynaptic_segment_id(_index) = segment_id;
//}
//
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::postsynaptic_segment_distance(Segment_Normalized_Distance 
//                                           segment_distance)
//{
//  _microcircuit->dataset().structure->synapse.
//        postsynaptic_segment_distance(_index) = segment_distance;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::decay(Millisecond decay)
//{
//    bbp_assert(_microcircuit->dataset().structure->
//        synapse.decays());
//    _microcircuit->dataset().structure->
//        synapse.decay(_index) = decay;
//}
//
//// ----------------------------------------------------------------------------
//
//inline void Synapse::delay(Millisecond delay)
//{
//    bbp_assert(_microcircuit->dataset().structure->synapse.delays());
//    _microcircuit->dataset().structure->synapse.
//        delay(_index) = delay;
//}
//
//// ----------------------------------------------------------------------------

}
#endif
