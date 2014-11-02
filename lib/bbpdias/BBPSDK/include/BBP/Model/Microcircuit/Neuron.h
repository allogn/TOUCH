/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler
        Contributing authors:   Juan Hernando Vieites
                                Sebastien Lasserre
        
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_NEURON_H
#define BBP_NEURON_H

#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "Types.h"
#include "Exceptions.h"
#include "BBP/Common/Math/Geometry/Orientation.h"
#include "BBP/Common/Math/Geometry/Rotation.h"
#include "Morphology_Type.h"
#include "Electrophysiology_Type.h"

namespace bbp {

class Microcircuit;
class Morphology;
class Neurons;
class Synapses;
class Sections;
class Global_Sections;
class Section;
class Soma;
class Neuron;
class Structure_Dataset;
class Dynamics_Dataset;
typedef boost::shared_ptr<Neuron> Neuron_Ptr;
typedef boost::shared_ptr<Microcircuit> Microcircuit_Ptr;
typedef boost::weak_ptr<Microcircuit> Microcircuit_Weak_Ptr;
typedef boost::shared_ptr<const Neuron> Const_Neuron_Ptr;
typedef boost::shared_ptr<Morphology> Morphology_Ptr;
typedef boost::shared_ptr<const Morphology> Const_Morphology_Ptr;
typedef boost::shared_ptr<Synapses> Synapses_Ptr;

// ----------------------------------------------------------------------------

//! Interface to the physical structure and dynamics of a cortical neuron.
/*!
    The current implementation is specific to cortical neurons. However that
    can be generalized to generic neurons or cells later on.

    Neuron class provides access to neuron specific information of a multi-
    compartment neuron model (e.g. position,  electrophysiology and morphology 
    type) as well as to parts of the neuron (e.g. soma, dendrite, section, 
    compartment).
    \bug Potential bug: Section 0 is no longer always necessarily soma
    if I remember correctly. Review code if this is considered. (TT)
    \todo Review dynamic data functions such as voltage (TT)
    \todo Review if non const accessor functions have sense giving
    that objects are being returned by value and the real soma,
    section, ... is not going to be modified.
    To provide modify access to morphology parts we need to
    do it differently (JH).
    Reply: A link to the dataset in the future may be always necessary
        as I would prefer to move the member attributes outside the 
        Neuron object into a dataset like in synapse to save memory
        if data is not needed. This will also make copies cheaper. 
        (TT)

    \todo Removal requests (JH):
    (Disclaimer: Some of these functions are used in the existing analysis 
    code!) (TT)
    All these functions are currently commented. May be safe to remove them.
    inline Section_ID number_of_sections(
        bool only_sections_with_loaded_voltages = true);
    inline Morphology_Point_ID number_of_points();
    inline const Micron length(Section_ID section_id);
    inline const Micron diameter(Section_ID section_id, 
                                 Compartment_ID compartment_id); current

    \todo Commented out function signatures at the bottom of this file for 
    modification support in the future. Take them somewhere outside for 
    the release. (TT)
    \todo Consider the checks in apical_dendrites() axon(), etc... to be either
    commented or centralized in a single location (e.g. insert_range()) so
    other developers get the purpose of the checks. (TT/JH)
    \ingroup Microcircuit
*/
class Neuron
    //: public Morphology_Interface
    // : public Cell,
    //   Neuron_Electrophysiology,
    //   Neuron_Morphology
{
    friend class Neurons;
    friend class Soma;
    friend class Section;
    friend class Synapses;
    friend class Microcircuit_Reader;
    friend class Neuron_Accessor;
    friend class Microcircuit;
    friend std::ostream & operator << (std::ostream & lhs, const Neuron & rhs);

public:
    // DO NOT USE: Default constructor is reserved for modification support.
	Neuron();
    Neuron(Microcircuit & microcircuit);
    Neuron(Microcircuit & microcircuit, Cell_GID gid);
    Neuron(Microcircuit & microcircuit, Cell_GID gid, Cell_Index index);

/* private: */
/*     Neuron(const Neuron & neuron); */

public:
    // IDENTIFICATION _________________________________________________________

    /*!
        \name Identity
        Get read-only access to identity of the neuron.
    */
    //@{

    //! Get the global identifier of the neuron.
    /*!
        The gid e.g. of a neuron is typcially written as an a and a number 
        e.g. a50 and is translated to this Cell_GID number simply by removing 
        the a, so it is 50 for this example.
        \return number identifying the neuron
    */
    inline Cell_GID                   gid() const;
    //! Get the label of the neuron.
    /*!
        The label is not allowed to contain any spaces. However currently
        this is not enforced.
        \return a byte character string
    */
    inline const Label                label() const;
    //@}

    /*!
        \name Structure: Connectivity
        Get read-only access to structure of the neuron.
    */
    //@{

    // STRUCTURE ______________________________________________________________

    //! Get neurons connected to this cell that it receives input signals from.
    /*! The neuron set returned depends on the synapse dataset currently loaded
        for the relevant microcircuit */
    const Neurons presynaptic_neurons() const;
    //! Get neurons connected to this cell that it sends its output signal to.
    /*! The neuron set returned depends on the synapse dataset currently loaded
        for the relevant microcircuit */
    const Neurons postsynaptic_neurons() const;
    //! Get afferent (incoming) synapses projecting onto this neuron
    const Synapses &             afferent_synapses() const;
    //! Get efferent (outgoing) synapses projecting onto other neurons
    const Synapses &             efferent_synapses() const;
    //@}

    /*!
        \name Structure: Connectivity
        Get full (read and write) access to structure of the neuron.
    */
    //@{

    //! Get neurons connected to this cell that it receives input signals from.
    /*! The neuron set returned depends on the synapse dataset currently loaded
        for the relevant microcircuit */
    Neurons presynaptic_neurons();
    //! Get neurons connected to this cell that it sends its output signal to.
    /*!
        The neuron set returned depends on the synapse dataset currently loaded
        for the relevant microcircuit.
    */
    Neurons postsynaptic_neurons();
    //! Get afferent (incoming) synapses projecting onto this neuron
    Synapses &             afferent_synapses();
    //! Get efferent (outgoing) synapses projecting onto other neurons
    Synapses &             efferent_synapses();
    //@}

    /*!
        \name Structure: Components
        Get read-only access to structure of the neuron.
    */
    //@{

    //! Get a specific section by its index (read-only access).
    /*!
        Disclaimer: In contrast to accessing the Morphology sections
        directly, here the Section is placed in the circuit (i.e.
        rotated and translated to global microcircuit coordinates).
        \param section_id index of the section local to the neuron
        \return object referencing the specified section
        \sa Morphology, Sections, Section
    */
    inline const Section    section(Section_ID section_id) const;
    //! Get cell body (read-only access).
    /*!
        All coordinates are in global coordinates.
        \return central interface object with access to soma data
        \sa Morphology, Soma
    */
    const Soma       soma() const;
    //! Get all neurite sections (excluding soma) in a neuron specific container. (read-only access)
    /*!
        All coordinates are in global coordinates 
        \return container with all neurite sections
        \sa Morphology, Sections, Section
    */
    inline const Sections   neurites() const;

    /** \cond */
//    //! Get all neurite sections (excluding soma) in a global container. (read-only access)
//    //! (read-only access)
//    /*!
//        All coordinates are in global coordinates 
//        \return container with all neurite sections
//        \sa Morphology, Sections, Section
//    */
//    inline const Global_Sections   global_neurites() const;
    /** \endcond */

    //! Get all axon sections in a container (read-only access).
    /*!
        Disclaimer: In contrast to accessing the Morphology sections
        directly, here the Section is placed in the circuit (i.e.
        rotated and translated to global microcircuit coordinates).
        \return container with all axon sections
        \sa Morphology, Sections, Section
    */
    inline const Sections   axon() const;
    //! Get all dendrite sections in a container (read-only access).
    /*!
        Disclaimer: In contrast to accessing the Morphology sections
        directly, here the Section is placed in the circuit (i.e.
        rotated and translated to global microcircuit coordinates).
        \return container with all dendrite sections
        \sa Morphology, Sections, Section
    */
    inline const Sections   dendrites() const;
    //! Get all basal dendrite sections in a container (read-only access).
    /*!
        Disclaimer: In contrast to accessing the Morphology sections
        directly, here the Section is placed in the circuit (i.e.
        rotated and translated to global microcircuit coordinates).
        \return container with all basal dendrite sections
        \sa Morphology, Sections, Section
    */
    inline const Sections   basal_dendrites() const;
    //! Get all apical dendrite sections in a container (read-only access).
    /*!
        Disclaimer: In contrast to accessing the Morphology sections
        directly, here the Section is placed in the circuit (i.e.
        rotated and translated to global microcircuit coordinates).
        \return container with all apical dendrite sections
        \sa Morphology, Sections, Section
    */
    inline const Sections   apical_dendrites() const;
    //! Get all the first order sections in a container (read-only access).
    /*!
        Disclaimer: In contrast to accessing the Morphology sections
        directly, here the Section is placed in the circuit (i.e.
        rotated and translated to global microcircuit coordinates).
        \deprecated Will be removed in future builds and replaced by
        const Sections Soma::children() const.
        \return container with all the sections starting from the soma
        \sa Morphology, Sections, Section
    */
    inline const Sections   first_order_sections() const;
    //! Get morphology parent class of the neuron (volumetric geometry).
    /*!
        \todo This should be inherited by a Morphology interface class
        once it is separated from Morphology_Dataset. (TT)
    */
    inline const Morphology      & morphology() const;
    //@}

    /*!
        \name Structure: Geometry
        Get read-only access to structure of the neuron.
    */
    //@{

    //! Get position of the soma (cell body).
    /*!
        The position returned is the center of the cell body in global
        coordinates with micron length unit.
        \return 3D vector representing the position of the soma in micron
    */
    inline const Vector_3D<Micron>    & position() const;
    //! Get global coordinates for position within a section.
    /*!
        \deprecated This function is legacy. Use Neuron::section(Section_ID) and 
        Section::cross_section(Section_Normalized_Distance) instead.
        \param section_id section index inside the neuron
        \param section_distance normalized position inside section between 0.0 (start) 
               and 1.0 (end) 
        \return global position in three dimensional space with length unit micron
    */
    inline const Vector_3D<Micron> position
        (Section_ID section_id, 
         Section_Normalized_Distance section_distance) const;
    //! Get global coordinates for position within a segment.
    /*!
        \deprecated This function is legacy. Use Neuron::section(Section_ID),
        Section::segment(Segment_ID), and 
        Segment::cross_section(Section_Normalized_Distance) instead.
        \param section_id section index inside the neuron
        \param segment_id segment index inside the neuron
        \param segment_distance normalized position inside segment between 0.0 (start) 
               and 1.0 (end) 
        \return global position in three dimensional space with length unit micron
    */
    inline const Vector_3D<Micron> position
        (Section_ID section_id, 
         Segment_ID segment_id,
         Segment_Normalized_Distance segment_distance) const;

    //! Get orientation of the neuron in space.
    inline const Orientation     & orientation() const;

    //! Get the transformation matrix to global coordinates.
    inline const Transform_3D<Micron> & global_transform() const
    {
        return _global_transform;
    }
//    //! Get section length of specified section.
//    inline const Micron length(Section_ID section_id);
//    //! Get cable diameter at specified position.
//    inline const Micron diameter(Section_ID section_id, 
//                                 Compartment_ID compartment_id);
    // \todo This should move into Morphology interface. (TT)
    // //! number of sections (all or only with loaded voltages)
    // Method commented to avoid link error during the SWIG wraping (SL)
    //inline const Section_ID             number_of_sections(
//      bool only_sections_with_loaded_voltages = true) const;
    //! number of morphology points
    /*!
        \deprecated This function should no longer be used and is removed
        without replacement soon. (TT)
    */
    inline Morphology_Point_ID          number_of_points() const;

    //! Length of path along the branches from soma center to specified point on a neurite.
    /*!
        \param section_id the index of the section of the destination point.
        \param distance distance inside section 0 from start of section.
    */
    Micron path_to_soma(
        Section_ID section_id, Section_Normalized_Distance distance = 0.0);
    //@}

    /*!
        \name Classification
        Get read-only access to class of the neuron.
    */
    //! Get the morphology type (m-type) of this cell.
    /*!
     The morphology (geometrical form) of the cell is classified into 
     different phenomenological classes (m-types):  pyramidal cell, 
     small basket cell, ...)
     \todo This should move into Morphology class. (TT) 
     Reply: Users want to 
     keep it here independently of what the Morphology provides (JH)
     In the end we have to see it from the perspective of NEURON integration:
     If the Morphology_Interface moves into Neuron, we would have to think 
     of a class hierarchy like Cell -> Neuron -> Cortical_Neuron since 
     NEURON can also be used to model e.g. Glial cells, and we would 
     need a Cell_Morphology, Neuron_Morphology (with axon)
     etc class. In that way Morphology cannot be used with aggregation but
     we need to use virtual inheritance. 
     Also we should decouple the morphology dataset from the interface
     so it can be reused without being a pointer. We should throw an
     exception for unloaded data if the library has been compiled
     with a certain flag (e.g. BBP_SECURE_ACCESS), a flag that could be set
     in CMake configuration (ON/OFF) and be the default in debug mode. (TT)
     \return m-type of the neuron
     */
    inline Morphology_Type                    morphology_type() const;
    
    //! Get the electrophysiological type (e-type) of this cell.
    /*!
        The electrical behavior of the cell is classified into different 
        phenomenological classes (e-types).
        \todo This should move into Morphology class. (TT) Reply: Users want to 
        keep it here indenpendently of what the Electrophysiology could 
        provides (JH)
        \return e-type of the neuron
    */
    inline Electrophysiology_Type           electrophysiology_type() const;
    //! Get column this neuron belongs to.
    /*!
        \return global column identifier
        \todo This should move into Cortical_Neuron interface. (TT)
    */
    inline Column_GID                       column() const;
    //! Get global identifier of the minicolumn this neuron belongs to.
    /*!
        A minicolumn is a vertical column of cells with typically 6 layers
        and consists of about 100 cells.

        \todo This should move into Cortical_Neuron interface. (TT)
        \return global minicolumn identifier
    */
    inline Minicolumn_GID                   minicolumn() const;
    //! Get cell layer in which this cell is located.
    /*!
        The neocortical column is a layered structure with typically 6 layers
        (but varies across species and brain regions). The layer numbering 
        starts with 1 for the layer closest to the brain surface and 6 for the 
        layer deepest inside the brain in the rat somatosensory cortex modelled
        in the Blue Brain Project.

        \todo This should move into Cortical_Neuron interface. (TT)
        \return integer number >= 1 representing the layer
    */
    inline Cortical_Layer_Number            layer() const;
    //@}

    // COMPARTMENTS -----------------------------------------------------------

    /*!
        \name Dynamics
        Get read-only access to dynamics of the neuron.
    */
    //@{

    //! Get voltage of the soma (cell body of the neuron)
    /*!
        \todo This should move into Electrophysiology class. (TT)
        \return voltage in millivolts (mV)
    */
    Millivolt voltage() const;
    //! Get voltage at a specific point on the neuron morphology.
    /*!
        \todo This should move into Electrophysiology class. (TT)
        \param section_id section specified by its neuron internal index 
        \param compartment_id section relative index of the compartment
                                 (compartment offset inside the section)
        \return voltage in millivolts (mV)
    */
    Millivolt voltage(Section_ID          section_id,
                      Compartment_ID   compartment_id = 0) const;
    //! Get index of the compartment inside a section at a specific point. 
    /*!
        \todo This should move into Electrophysiology class. (TT)
        \param section_id section specified by its neuron internal index
        \param section_distance point inside the section in normalized distance from 
               the section start (0.0) and section end (1.0)
        \return compartmnet_ID for the section and normalized distance.
    */
    Compartment_ID compartment(
        Section_ID section_id, 
        Section_Normalized_Distance section_distance) const;
    //! Get absolute array index of the compartment in current frame.
    /*!
        This is an optimized function for fastest access to voltages for
        time critical cases.
        \todo This should move into Electrophysiology class. (TT)
        \param section_id section specified by its neuron internal index 
        \param compartment_id section relative index of the compartment
                                 (compartment offset inside the section)
        \return index of the compartment in the active Compartment_Report_Frame
        \sa Compartment_Report_Frame, Section
    */
    Report_Frame_Index compartment_frame_index(
        Section_ID section_id, Compartment_ID compartment_id) const;
    //@}

    //! Returns the index of this neuron within the current simulation buffer.
    /*!
        This index returned can be used to know the mapping offset for the
        sections of this neuron concerning the current compartment report from
        the microcircuit dynamic dataset. If the neuron is not being reported
        or loaded the return value will be UNDEFINED_CELL_INDEX. 
    */
    Cell_Index index() const;


    // STATUS INFORMATION -----------------------------------------------------

    //! Returns whether the morphology for this neuron has been loaded.
    inline bool morphology_loaded() const;

    //! Print neuron status information to the standard output.
    inline void print() const;

protected:
    //! Internal index of the neuron inside the dynamics dataset
    /*! This index is used to access the mapping of the report currently bound
        to the microcircuit this neuron belongs to. If there is no dynamic
        information for this neuron its index will equal to
        UNDEFINED_CELL_INDEX. */
    Cell_Index              _index;

    //! Morphology name.
    /**
       This information, depite seeming redundant, is needed to link
       neurons with their morphologies.
     */
    Label                   _morphology_name;

    //! points to microcircuit associated with this neuron
    Microcircuit_Weak_Ptr   _microcircuit;

    //! Name of the neuron defined in the MVD file (ex. a1565).
    Cell_GID                _gid;
    //! Number of column (currently typically just one).
    Column_GID              _column_gid;
    //! Number of minicolumn.
    Minicolumn_GID          _minicolumn_gid;
    //! Layer number for the neuron.
    Cortical_Layer_Number   _layer_number;
    //! Pointer to the neuron morphology (geometrical form).
    Morphology_Ptr          _morphology;
    //! Morphology type of the neuron.
    Morphology_Type         _morphology_type;
    //! Electrophysiology type of the neuron.
    Electrophysiology_Type  _electrophysiology_type;
    //! Position of the neuron in the absolute reference.
    Vector_3D<Micron>       _position;
    //! Orientation of the neuron.
    Orientation             _orientation;
    //! Cached transformation matrix to global coordinates.
    /*!
        CAUTION: This is set in the Microcircuit_MVD_File_Reader class and is
        not automatically synced with _position and _orientation, so a set 
        function must update this matrix as well.
    */
    Transform_3D<Micron>    _global_transform;
    //! synapses signaling to this from other neurons
    Synapses_Ptr            _afferent_synapses;
    //! synapses signaling from this to other neurons
    Synapses_Ptr            _efferent_synapses;

};

//! Print neuron status information.
inline std::ostream & operator << (std::ostream & lhs, const Neuron & rhs);
////! Print cell GID in form a + gid (e.g. 50 -> "a50").
//inline std::ostream & operator << (std::ostream & lhs, const Cell_GID & rhs);


// ----------------------------------------------------------------------------

}

#include "Morphology.h"
#include "Containers/Sections.h"
//#include "Containers/Global_Sections.h"
#include "Section.h"
#include "Soma.h"
#include "Containers/Synapses.h"
#include "Containers/Neurons.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"

#ifdef BBP_SAFETY_MODE
#  define CHECK_MICROCIRCUIT_POINTER \
    if (_microcircuit.lock().get() == 0)             \
    {                                                \
        throw_exception(Microcircuit_Access_Error(), \
            SEVERE_LEVEL, __FILE__, __LINE__);       \
    }
#else
#  define CHECK_MICROCIRCUIT_POINTER bbp_assert(_microcircuit.lock().get() != 0)
#endif

namespace bbp
{
// ----------------------------------------------------------------------------

inline Millivolt Neuron::voltage() const
{
    CHECK_MICROCIRCUIT_POINTER;

    Compartment_Report_Frame<Millivolt> & voltages = 
        _microcircuit.lock()->dataset().dynamics->compartment_voltages;
    const Compartment_Report_Mapping & mapping  = 
        * voltages.context().mapping();

#ifdef BBP_SAFETY_MODE
    if (voltages.size() <= mapping.section_offset(_index, 0))
    {
        throw_exception(Microcircuit_Access_Error(), 
            SEVERE_LEVEL, __FILE__, __LINE__);
    }
#endif

    return voltages.frame()[mapping.section_offset(_index, 0)];
}

// ----------------------------------------------------------------------------

inline Millivolt Neuron::voltage(
    Section_ID          section_id, 
    Compartment_ID   compartment_id) const
{
    CHECK_MICROCIRCUIT_POINTER;

    Compartment_Report_Frame<Millivolt> & voltages = 
        _microcircuit.lock()->dataset().dynamics->compartment_voltages;
    const Compartment_Report_Mapping & mapping = 
        * voltages.context().mapping();

#ifdef BBP_SAFETY_MODE
    if (voltages.size() <= mapping.section_offset(_index, section_id) +
        compartment_id)
    {
        throw_exception(Microcircuit_Access_Error(), 
            SEVERE_LEVEL, __FILE__, __LINE__);
    }
#endif
    return voltages.frame()[mapping.section_offset(_index, section_id) + 
        compartment_id];
}

// ----------------------------------------------------------------------------

inline const Soma Neuron::soma() const
{
    return Soma(* this);
}

// ----------------------------------------------------------------------------

inline Compartment_ID Neuron::compartment(
    Section_ID                  section_id, 
    Section_Normalized_Distance section_distance) const
{
    CHECK_MICROCIRCUIT_POINTER;

    // Shorthands
    Compartment_Report_Frame<Millivolt> & voltages = 
        _microcircuit.lock()->dataset().dynamics->compartment_voltages;
    const Compartment_Report_Mapping & mapping  = 
        * voltages.context().mapping();

    // Check if distance value out of range.
    bbp_assert (section_distance <= 1.0 &&  section_distance >= 0.0);

    // Get number of compartments in section
    Compartment_Count section_size = mapping.number_of_compartments(
        _index, section_id);

    // If no voltages loaded for this section, take soma voltage.
    if (section_size == 0)
    {
        section_size = mapping.number_of_compartments(_index, 0);
        section_distance = 0.0;
    }
    
    // Check if any voltages loaded for this section.
    bbp_assert (section_size > 0);

    // Return compartment index inside the section.
    return (Compartment_ID) ((section_size-1) * section_distance);
}

// ----------------------------------------------------------------------------

inline Report_Frame_Index Neuron::compartment_frame_index(
    Section_ID          section_id, 
    Compartment_ID   compartment_id) const
{
    CHECK_MICROCIRCUIT_POINTER;

    // Shorthands
    Compartment_Report_Frame<Millivolt> & voltages = 
        _microcircuit.lock()->dataset().dynamics->compartment_voltages;
    const Compartment_Report_Mapping & mapping  = 
        * voltages.context().mapping();

    return mapping.section_offset(_index, section_id) + compartment_id;
}

// ----------------------------------------------------------------------------

inline const Label Neuron::label() const
{
    std::stringstream label;
    label << 'a' << _gid;
    return label.str();
}

// ----------------------------------------------------------------------------

inline Cell_GID Neuron::gid() const
{
    return _gid;
}

// ----------------------------------------------------------------------------

inline Morphology_Type Neuron::morphology_type() const
{
    return _morphology_type;
}

// ----------------------------------------------------------------------------

inline Electrophysiology_Type Neuron::electrophysiology_type() const
{
    return _electrophysiology_type;
}

// ----------------------------------------------------------------------------

inline Column_GID Neuron::column() const
{
    return _column_gid;
}

// ----------------------------------------------------------------------------

inline Minicolumn_GID Neuron::minicolumn() const
{
    return _minicolumn_gid;
}

// ----------------------------------------------------------------------------

inline const Section Neuron::section(Section_ID section_id)  const
{
#ifdef BBP_SAFETY_MODE
    if (_morphology.get() == 0)
    {
        throw_exception(Microcircuit_Access_Error(), 
            SEVERE_LEVEL, __FILE__, __LINE__);
    }
#endif
    bbp_assert(_morphology);
    return Section(_morphology.get(), this, _microcircuit.lock().get(),
                   Rotation_3D<Micron>(_orientation), section_id);
}

// ----------------------------------------------------------------------------

inline Cortical_Layer_Number Neuron::layer() const
{
    return _layer_number;
}

// ----------------------------------------------------------------------------

const Morphology & Neuron::morphology() const
{
#ifdef BBP_SAFETY_MODE
    /*! 
        \todo consider using this code without safety mode turned on
        and load morphology transparently if not loaded already. (TT)
    */
    if (_morphology.get() == 0)
    {
        throw_exception(Morphology_Not_Found("Neuron does not have the"
            "associated morphology loaded"), 
            SEVERE_LEVEL, __FILE__, __LINE__);
    }

#endif
    return * _morphology.get();
}

// ----------------------------------------------------------------------------

inline Cell_Index Neuron::index() const
{
    return _index;
}

// ----------------------------------------------------------------------------

inline bool Neuron::morphology_loaded() const
{
    return _morphology.get() != 0;
}

// ----------------------------------------------------------------------------

inline void Neuron::print() const
{
    std::cout << "Neuron " << * this << std::endl;
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> & Neuron::position() const
{
    return _position;
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> Neuron::position
    (Section_ID                     section_id, 
     Section_Normalized_Distance    section_distance) const
{
    return _position + (Rotation_3D<Micron>(_orientation) * 
                        _morphology->position(section_id, section_distance));
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> Neuron::position
        (Section_ID section_id, 
         Segment_ID segment_id,
         Segment_Normalized_Distance segment_distance) const
{
    return _position + (Rotation_3D<Micron>(_orientation) *
        _morphology->position(section_id, segment_id, segment_distance));
}

// ----------------------------------------------------------------------------

inline const Orientation & Neuron::orientation() const
{
    return _orientation;
}

// ----------------------------------------------------------------------------

inline const Synapses & Neuron::afferent_synapses() const
{
    bbp_assert(_afferent_synapses.get() != 0);
    return * _afferent_synapses.get();
}

// ----------------------------------------------------------------------------

inline const Synapses & Neuron::efferent_synapses() const
{
    bbp_assert(_efferent_synapses.get() != 0);
    return * _efferent_synapses.get();
}

// ----------------------------------------------------------------------------

inline Synapses & Neuron::afferent_synapses()
{
    bbp_assert(_afferent_synapses.get() != 0);
    return * _afferent_synapses.get();
}

// ----------------------------------------------------------------------------

inline Synapses & Neuron::efferent_synapses()
{
    bbp_assert(_efferent_synapses.get() != 0);
    return * _efferent_synapses.get();
}

// ----------------------------------------------------------------------------
//
//inline const Section_ID Neuron::number_of_sections
//  (bool only_sections_with_loaded_voltages) const
//{
//// \todo review this
////    if (only_sections_with_loaded_voltages == true)
////    {
////        if (data.compartment_voltages != 0)
////        {
////            return (Section_ID) data.compartment_voltages->context().
////                    compartment_mapping->number_of_sections(index);
////        }
////    }
////    if (morphology_pointer != 0)
////    {
////        return morphology_pointer->number_of_sections();
////    }
////    else
////    {
////        std::string error("Number of sections cannot be evaluated - no "
////                          "morphology loaded.");
////        std::cerr << error << std::endl;
////        throw std::runtime_error(error);
////    }
//    return UNDEFINED_SECTION_ID;
//}

// ----------------------------------------------------------------------------

inline Morphology_Point_ID Neuron::number_of_points() const
{
    bbp_assert(_morphology.get() != 0);
    return _morphology->number_of_points();
}

// ----------------------------------------------------------------------------

inline const Sections Neuron::neurites() const
{
    CHECK_MICROCIRCUIT_POINTER;

    bbp_assert(_morphology.get() != 0);
    Sections neurites(_morphology.get(), this, _microcircuit.lock().get());
    if (_morphology->axon_offset() < _morphology->number_of_sections() - 1)
    {
        neurites.insert_range(_morphology->axon_offset(), 
                              _morphology->number_of_sections() - 1);
    }
    else if ((_morphology->number_of_sections() 
              - _morphology->axon_offset()) == 1)
    {
        neurites.insert(_morphology->axon_offset());
    }
    return neurites;
}

// ----------------------------------------------------------------------------

//inline const Global_Sections Neuron::global_neurites() const
//{
//    bbp_assert(_morphology.get() != 0);
//    Global_Sections sections(*_microcircuit);
//    sections.insert_range
//        (Section_GID(gid(), 1), 
//         Section_GID(gid(), _morphology->number_of_sections() - 1));
//    return sections;
//}

// ----------------------------------------------------------------------------

inline const Sections Neuron::axon() const
{
    CHECK_MICROCIRCUIT_POINTER;

    bbp_assert(_morphology.get() != 0);
    Sections axon(_morphology.get(), this, _microcircuit.lock().get());
    if (_morphology->axon_offset() < _morphology->basal_dendrite_offset() - 1)
    {
        axon.insert_range(_morphology->axon_offset(), 
                          _morphology->basal_dendrite_offset() - 1);
    }
    else if ((_morphology->basal_dendrite_offset() 
              - _morphology->axon_offset()) == 1)
    {
        axon.insert(_morphology->axon_offset());
    }
    return axon;
}

// ----------------------------------------------------------------------------

inline const Sections Neuron::dendrites() const
{
    CHECK_MICROCIRCUIT_POINTER;

    bbp_assert(_morphology.get() != 0);
    Sections dendrites(_morphology.get(), this, _microcircuit.lock().get());
    if (_morphology->basal_dendrite_offset() 
        < _morphology->number_of_sections() - 1)
    {
        dendrites.insert_range(_morphology->basal_dendrite_offset(),
                               _morphology->number_of_sections() - 1);
    }
    else if ((_morphology->number_of_sections() 
              - _morphology->basal_dendrite_offset()) == 1)
    {
        dendrites.insert(_morphology->basal_dendrite_offset());
    }
    return dendrites;
}

// ----------------------------------------------------------------------------

inline const Sections Neuron::basal_dendrites() const
{
    CHECK_MICROCIRCUIT_POINTER;

    bbp_assert(_morphology.get() != 0);
    Sections basal(_morphology.get(), this, _microcircuit.lock().get());
    if (_morphology->number_of_sections(APICAL_DENDRITE) != 0)
    {
        if (_morphology->basal_dendrite_offset() 
            < _morphology->apical_dendrite_offset() - 1)
        {
            basal.insert_range(_morphology->basal_dendrite_offset(),
                               _morphology->apical_dendrite_offset() - 1);
        }
        else if ((_morphology->apical_dendrite_offset() 
                  - _morphology->basal_dendrite_offset()) == 1)
        {
            basal.insert(_morphology->basal_dendrite_offset());
        }
    }
    else
    {
        if (_morphology->basal_dendrite_offset() 
            < _morphology->number_of_sections() - 1)
        {
            basal.insert_range(_morphology->basal_dendrite_offset(),
                               _morphology->number_of_sections() - 1);
        }
        else if ((_morphology->number_of_sections() 
                  - _morphology->basal_dendrite_offset()) == 1)
        {
            basal.insert(_morphology->basal_dendrite_offset());
        }
    }
    return basal;
}

// ----------------------------------------------------------------------------

inline const Sections Neuron::apical_dendrites() const
{
    CHECK_MICROCIRCUIT_POINTER;

    bbp_assert(_morphology.get() != 0);
    Sections apical(_morphology.get(), this, _microcircuit.lock().get());
    if (_morphology->number_of_sections(APICAL_DENDRITE) != 0)
    {
        if (_morphology->apical_dendrite_offset() 
            < _morphology->number_of_sections() - 1)
        {
            apical.insert_range(_morphology->apical_dendrite_offset(),
                                _morphology->number_of_sections() - 1);
        }
        else if ((_morphology->number_of_sections() 
                  - _morphology->apical_dendrite_offset()) == 1)
        {
            apical.insert(_morphology->apical_dendrite_offset());
        }
    }
    return apical;
}

// ----------------------------------------------------------------------------

inline const Sections Neuron::first_order_sections() const
{
    CHECK_MICROCIRCUIT_POINTER;

    bbp_assert(_morphology.get() != 0);
    Sections result(_morphology.get(), this, _microcircuit.lock().get());

    const Sections & sections = _morphology->first_order_sections();
    for (Sections::const_iterator i = sections.begin(); 
         i != sections.end();
         ++i)
    {
        result.insert(i->id());
    }
    return result;
}

// ----------------------------------------------------------------------------


inline std::ostream & operator << (std::ostream & lhs, const Neuron & rhs)
{
    lhs << rhs.label() << ' '
        << rhs._morphology_name << ' ';
    if (rhs._morphology.get())
        lhs << rhs._morphology->origin() << ' ';
    lhs << rhs.column() << ' '
        << rhs.minicolumn() << ' '
        << rhs.layer() << ' '
        << rhs.morphology_type().name() << ' '
        << rhs.electrophysiology_type().name() << ' '
        << rhs.position() << ' '
        << rhs.orientation().rotation;
    return lhs;
}

// ----------------------------------------------------------------------------

}
#undef CHECK_MICROCIRCUIT_POINTER

#endif

    ////! Set absolute position of the soma (cell body) of the neuron in micron.
 //   inline void       position(const Vector_3D<Micron> & value);
 //   //! Set neuron orientation in space.
 //   inline void       orientation(const Orientation & value);
    ////! Set neuron morphology (volumetric geometry).
 //   // WRONG: We cannot set the Morphology by reference, argument must be
 //   // Morphology_Ptr.
 //   inline void       morphology(const Morphology & value);

    ////! Set the global identifier of the neuron.
    ///*!
    //  \param value global cell identifier 
    //*/
    //inline void       gid(Cell_GID value);
    ////! Set the label of the neuron.
    ///*!
    //  \param value character string without spaces labeling the neuron
    //*/
    //inline void       label(Label value);

 //   //! Set column this neuron belongs to.
    ///*!
    //  \param value global identifier of a column
    //*/
 //   inline void       column(Column_GID value);
 //   //! Set minicolumn identifier this neuron belongs to.
    ///*!
    //  A minicolumn is a vertical column of cells with typically 6 layers
    //  and consists of about 100 cells.
    //  \param value global identifier of a minicolumn
    //*/
    //inline void       minicolumn(Minicolumn_GID value);
 //   //! Set cell layer in which this cell is located.
    ///*!
    //  The neocortical column is a layered structure with typically 6 layers
    //  (but varies across species and brain regions). The layer numbering 
    //  starts with 1 for the layer closest to the brain surface and 6 for the 
    //  layer deepest inside the brain in the rat somatosensory cortex modelled
    //  in the Blue Brain Project.
    //  \param value integer number >= 1 representing the layer
    //*/
 //   inline void       layer(Cortical_Layer_Number value);
 //   //! Set the electrophysiological type (e-type) of this cell.
    ///*!
    //  The electrical behavior of the cell is classified into different 
    //  phenomenological classes (e-types).
    //*/
 //   inline void       electrophysiology_type(Electrophysiology_Type value);
 //   //! Set the morphology type (m-type) of this cell.
    ///*!
    //  The morphology (geometrical form) of the cell is classified into 
    //  different phenomenological classes (m-types).
    //*/
 //   inline void       morphology_type(Morphology_Type value);
    //// MODIFY ACCESS __________________________________________________________

 //   // \todo The signature of all methods within this section need in 
 //   // depth review (JH).

 //   //! Get cell body (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology sections
    //  directly, here the Section is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return interface object to soma (cell body) data of this neuron
    //  \sa Morphology, Soma
    //*/
 //   inline Soma      soma();

    ////! Get all neurite sections (excluding soma) in a container
    ////! (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology soma 
    //  directly, here the Soma is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return container with all neurite sections (excluding soma)
    //  \sa Morphology, Sections, Section
    //*/
 //   inline Sections  neurites();

 //   //! Get all axon sections in a container (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology sections
    //  directly, here the Section is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return container with all axon sections
    //  \sa Morphology, Sections, Section
    //*/
    //inline Sections  axon();

 //   //! Get all dendrite sections in a container (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology sections
    //  directly, here the Section is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return container with all dendrite sections
    //  \sa Morphology, Sections, Section
    //*/
    //inline Sections  dendrites();

 //   //! Get all basal dendrite sections in a container (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology sections
    //  directly, here the Section is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return container with all basal dendrite sections
    //  \sa Morphology, Sections, Section
    //*/
 //   inline Sections  basal_dendrites();

 //   //! Get all apical dendrite sections in a container (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology sections
    //  directly, here the Section is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return container with all apical dendrite sections
    //  \sa Morphology, Sections, Section
    //*/
 //   inline Sections  apical_dendrites();

    ////! Get a specific section by its index (with modify access).
 //   /*! 
    //  Disclaimer: In contrast to accessing the Morphology sections
    //  directly, here the Section is placed in the circuit (i.e.
    //  rotated and translated to global microcircuit coordinates).
    //  \return specified section
    //  \sa Morphology, Sections, Section
    //*/
 //   inline Section & section(Section_ID section_id);

