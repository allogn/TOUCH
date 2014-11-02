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

#ifndef BBP_MICROCIRCUIT_TYPES_H
#define BBP_MICROCIRCUIT_TYPES_H

#include <vector>


#include <boost/lexical_cast.hpp>

#include <stdexcept>
#include "Exceptions.h"

#include "BBP/Common/Types.h"
#include "BBP/Common/Math/Geometry/Circle.h"
namespace bbp {

// workaround for Windows 32 bit platform
#ifdef WIN32
#undef min
#undef max
#endif

// MICROCIRCUIT SPECIFICATION _________________________________________________

/*
    These types are biological structure identifiers that uniquely identify
    the components of a microcircuit such as a column, minicolumn, cell, or
    section.
*/

//! Global identifier of a cortical column (consists of minicolumns).
typedef boost::uint16_t                         Column_GID;
//! Global identifier of a minicolumn (consists of cells) starting at 0.
/*!
 minicolumn GID's start from 0 (so 0-99 if circuit has 100 minicolumns)
*/
typedef boost::uint16_t                         Minicolumn_GID;
//! Global identifier of a cell (typically a neuron) starting from 1.
/*!
    Neuron GID's start from 1 (so 1-10K in a 10K circuit)
*/
typedef boost::uint32_t                         Cell_GID;
//! Local identifier of a section inside a neuron starting at 0.
typedef boost::uint16_t                         Section_ID;
//! Specifies a sample point on a neuron morphology.
/*!
    DISCLAIMER: This needs to be the same as Segment_ID!
    \sa Segment Segment_ID
*/
typedef boost::uint16_t                         Morphology_Point_ID;

//! Global identifier of a point on a neuron morphology.
struct Morphology_Point_GID
{
    Cell_GID                    cell;
    Morphology_Point_ID         point;
};

//! Specifies a segment inside a neuron section.
/*!
    Segment ids start with 0 for each section and go up to number
    of points in the section - 1.    
*/
typedef boost::uint16_t                             Segment_ID;
//! Local identifier of a synapse inside a neuron
typedef boost::uint16_t                             Synapse_ID;
//! \todo this should become a struct. (TT)
//! Synapse global identifier.
typedef std::pair<Cell_GID, Synapse_ID>             Synapse_GID;
//! Normalized distance within a section between 0.0 (begin) and 1.0 (end).
typedef Normalized_Real_Number                      Section_Normalized_Distance;
//! Normalized distance within a segment between 0.0 (begin) and 1.0 (end).
typedef Normalized_Real_Number                      Segment_Normalized_Distance;
//! Number of the layer in cortex a cortical cell is located in.
/*!
    Cortical layer numbers start from 1, typically up to 6, with one
    being closest to the skull, 6 deepest inside the brain.
*/
typedef boost::uint16_t                             Cortical_Layer_Number;

//! A segment point is a circle with a diameter in micron
/*! \todo This should not be named Segment_Cross_Section
    but instead Segment_Cross_Section should become a class
    that is microcircuit aware. (TT)
*/
typedef Circle<Micron> Segment_Cross_Section;

// ----------------------------------------------------------------------------

//! Global identifier for a segment inside a section of a neuron.
/*!
    \sa Section
*/
struct Segment_GID
{
    Cell_GID        cell_gid;
    Section_ID      section_id;
    Segment_ID      segment_id;

    Segment_GID() {}
    
    Segment_GID(Cell_GID       cell_gid, 
                Section_ID     section_id, 
                Segment_ID     segment_id)
       : cell_gid(cell_gid), section_id(section_id), segment_id(segment_id)
    {}

    bool operator == (const Segment_GID & rhs) const
    {
        if (cell_gid == rhs.cell_gid &&
            section_id == rhs.section_id &&
            segment_id == rhs.segment_id)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool operator != (const Segment_GID & rhs) const
    {
        return !(this->operator ==(rhs));
    }

    void print() const
    {
        std::cout << "cell " << cell_gid << ", "
                  << "section " << section_id << ", "
                  << "segment " << segment_id << std::endl;
    }

#ifdef BBP_USE_BOOST_SERIALIZATION
    template <class Archive>
    void serialize (Archive & ar, const unsigned int version)
    {
        ar & (cell_gid) & section_id & segment_id;
    }
#endif
};

// ----------------------------------------------------------------------------

//! Global identifier of a neuron morphology section.
/*!
   Additional arithmetic operations are provided in order to use this gid
   as a template parameter of Indexed_Container to create a global Section
   container.
   \cond SHOW_IN_DEVELOPER_REFERENCE
   Overflows in arithmentic operations shouldn't be a concern regarding
   Index_Set and Interval_Set implementation.
   \endcond SHOW_IN_DEVELOPER_REFERENCE
*/
struct Section_GID
{
    Section_GID() {}
    Section_GID(Cell_GID cell_gid, Section_ID section_id)
        : cell_gid(cell_gid), section_id(section_id)
    {}

    Section_GID & operator ++ ()
    {
        ++section_id;
        return *this;
    }

    Section_GID operator ++ (int) const
    {
        Section_GID gid;
        gid.cell_gid = cell_gid;
        gid.section_id = section_id + 1;
        return gid;
    }

    Section_GID & operator -- ()
    {
        --section_id;
        return *this;        
    }

    Section_GID operator -- (int) const
    {
        Section_GID gid;
        gid.cell_gid = cell_gid;
        gid.section_id = section_id - 1;
        return gid;
    }
    
    Section_GID operator + (Section_ID increment) const
    {
        Section_GID gid;
        gid.cell_gid = cell_gid;
        gid.section_id = section_id + increment;
        return gid;
    }

    Section_GID operator - (Section_ID decrement) const
    {
        Section_GID gid;
        gid.cell_gid = cell_gid;
        gid.section_id = section_id - decrement;
        return gid;
    }

    std::ptrdiff_t operator - (Section_GID gid) const
    {
        if (cell_gid == gid.cell_gid)
            return section_id - gid.section_id;
        else
            return std::numeric_limits<std::ptrdiff_t>::max();
    }

    bool operator > (const Section_GID & gid) const
    {
        if (cell_gid == gid.cell_gid)
            return section_id > gid.section_id;
        else
            return cell_gid > gid.cell_gid;        
    }

    bool operator < (const Section_GID & gid) const
    {
        if (cell_gid == gid.cell_gid)
            return section_id < gid.section_id;
        else
            return cell_gid < gid.cell_gid;
    }

    bool operator <= (const Section_GID & gid) const
    {        
        return *this < gid || *this == gid;

    }

    bool operator >= (const Section_GID & gid) const
    {        
        return *this > gid || *this == gid;

    }

    bool operator == (const Section_GID & gid) const
    {
        return cell_gid == gid.cell_gid && section_id == gid.section_id;
    }

    Cell_GID    cell_gid;
    Section_ID  section_id;
};

// ----------------------------------------------------------------------------

//! Global identifier of a point on a neuron morphology segment.
struct Segment_Point_GID
    : public Segment_GID
{
    Segment_Normalized_Distance distance;
};

// ----------------------------------------------------------------------------

//! Global identifier of a point on neuron morphology section.
struct Section_Point_GID : public Section_GID
{
	//! Create uninitialized section point identifier.
    Section_Point_GID() {}
    //! Create and initialize section point identifier.
	Section_Point_GID(Cell_GID cell_gid, 
                      Section_ID section_id, 
                      Section_Normalized_Distance distance)
                      : Section_GID(cell_gid, section_id),
                        distance(distance)
    {}

	//! Normalized (0.0-1.0) distance within the section.
    Section_Normalized_Distance distance;
};

// ----------------------------------------------------------------------------

/*!
    \todo Check what this is storing, before has been 
    Voxel_Segment_GID_Volume.
*/
struct Voxel_Segment_GID_Volume
{
    //! Global identifier of the segment.
    Segment_GID segment;
    //! Volume in cubic micron that this segment occupies in the voxel.
    /*!
        \todo Double check if we store abs. volume or percentage. (TT)
    */
    Micron3 volume;

    Voxel_Segment_GID_Volume() {}

    Voxel_Segment_GID_Volume(Cell_GID cell_gid, Section_ID section_id, 
        Segment_ID segment_id, Micron3 volume)
    :   segment(cell_gid, section_id, segment_id), volume(volume)
    {}
    
    Voxel_Segment_GID_Volume(Segment_GID segment, Micron3 volume)
    :   segment(segment), volume(volume)
    {}
};

inline std::ostream & operator <<
    (std::ostream & lhs, Voxel_Segment_GID_Volume & rhs)
{
    lhs << "segment gid: neuron a" << rhs.segment.cell_gid << ", section " 
        << rhs.segment.section_id << ", segment " << rhs.segment.segment_id
        << ", volume: " << rhs.volume;
    return lhs;
}

// ----------------------------------------------------------------------------

typedef size_t Segment_Index;

//! specifies the voxel volume percentage occupied by this segment
struct Segment_Voxel_Occupancy
{
    //! id idenfifying the segment
    /*!
        \bug This is probably no longer correct since segment id is local to
        section now. (TT)
    */
    Segment_Index     segment_index;
    //! voxel volume occupied by this segment
    Micron3            volume;

    Segment_Voxel_Occupancy() {}
    Segment_Voxel_Occupancy(Segment_Index segment_index, Micron volume)
        : segment_index(segment_index), volume(volume) {}
    ~Segment_Voxel_Occupancy() {}

#ifdef BBP_USE_BOOST_SERIALIZATION
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & segment_index;
        ar & volume;
    }
#endif

};

inline std::ostream & operator << 
    (std::ostream & lhs, Segment_Voxel_Occupancy & rhs)
{
    lhs << "segment index: " << rhs.segment_index
        << ", volume: " << rhs.volume;
    return lhs;
}

// ----------------------------------------------------------------------------

typedef std::vector<Segment_Voxel_Occupancy>    Segment_Indices_in_Voxel;

inline std::ostream & operator << 
    (std::ostream & lhs, Segment_Indices_in_Voxel & rhs)
{
    for (size_t i = 0; i < rhs.size(); ++i)
    {
        lhs << rhs[i] << " ";
    }
    lhs << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

typedef std::vector<Voxel_Segment_GID_Volume>    Segments_in_Voxel;

inline std::ostream & operator << (std::ostream & lhs, Segments_in_Voxel & rhs)
{
    for (size_t i = 0; i < rhs.size(); ++i)
    {
        lhs << rhs[i] << " ";
    }
    lhs << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

/* Tags used in the hdf5 to specify the type of the section. */
typedef Byte HDF5_Section_Type; 
const HDF5_Section_Type HDF5_SOMA_TYPE     = 1; 
const HDF5_Section_Type HDF5_AXON_TYPE     = 2; 
const HDF5_Section_Type HDF5_DENDRITE_TYPE = 3; 
const HDF5_Section_Type HDF5_APICAL_DENDRITE_TYPE   = 4; 

// ----------------------------------------------------------------------------


//! Classification of neuron substructures (e.g. soma, dendrites, axon).
enum Section_Type
{
    //! neuron cell body
    SOMA                    =   0x00000001,
    //! axon
    AXON                    =   0x00000002,
    //! general or basal dendrite (near to soma)
    DENDRITE                =   0x00000004,
    //! apical dendrite (far from soma)
    APICAL_DENDRITE         =   0x00000008,
    //! 
    UNDEFINED_SECTION_TYPE  =   0xffffffff
};

inline std::ostream & operator << (std::ostream & lhs, Section_Type rhs)
{
    switch (rhs)
    {
        case SOMA:
            return lhs << "soma";
        case AXON:
            return lhs << "axon";
        case DENDRITE:
            return lhs << "dendrite";
        case APICAL_DENDRITE:
            return lhs << "apical dendrite";
        case UNDEFINED_SECTION_TYPE:
        default:
            return lhs << "undefined";
    }
}

//! Section branch order
/*!
    The branch order of a Section is the number of branching points from
    the root (Soma) of the morphology tree to this Section.

    Soma has a branch order of zero, the direct children of the soma are
    first order sections, the children of them are second order and so on.
*/
typedef boost::uint16_t                     Section_Branch_Order; 
//! origin of the morphology reconstruction (e.g. cloned or repaired)
typedef boost::uint16_t                     Morphology_Reconstruction_Origin;

// ----------------------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
//! Index type used inside the compartment report mapping.
typedef size_t Cell_Index;
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

// ----------------------------------------------------------------------------

//! Function to return the size of the array in number of elements (not bytes).
template <typename T>
size_t array_size(const T & a)
{
    return sizeof(a) / sizeof (*a);
}

// ----------------------------------------------------------------------------

//! Names of the electrophysiology type classes.
/*const char * const Electrophysiology_Type_Labels[] = */
/*{*/
/*"cAD",          //!< 0: continuous accommodating cell*/
/*"bAD",          //!< 1: burst accommodating cell*/
/*"dAD",          //!< 2: delayed accommodating cell*/
/*"cFS",          //!< 3: continuous fast spiking cell*/
/*"bFS",          //!< 4: burst fast spiking cell*/
/*"dFS",          //!< 5: delayed fast spiking cell*/
/*"cST",          //!< 6: continuous stuttering cell*/
/*"bST",          //!< 7: burst stuttering cell*/
/*"dST",          //!< 8: delayed stuttering cell*/
/*"cIS",          //!< 9: continuous irregular spiking cell*/
/*"bIS",          //!< 10: burst irregular spiking*/
/*"IBS",          //!< 11: initial bursting cell*/
/*"rBS",          //!< 12: regular bursting cell*/
/*"tBS",          //!< 13: transient bursting cell*/
/*"cAL",          //!< 14: continuous accelerating cell*/
/*"dAL",          //!< 15: delayed accelerating cell*/
/*"bAL",          //!< 16: bursting accelerating cell*/
/*"cNA",          //!< 17: continuous non accommodating cell*/
/*"bNA",          //!< 18: bursting non accommodating cell*/
/*"dNA"           //!< 19: delayed non accommodating cell*/
/*};*/

// ----------------------------------------------------------------------------

//! Classification ID of the electrophysiology (electrical behavior) of a cell.
typedef boost::uint16_t                     Electrophysiology_Type_ID;

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

//! Names of the morphology type classes.
/*!
    All the L* are pyramidal except for L4SS. All the L* are excitatory. 
	Here * is the unix shell globbing operator
	\todo Check if Anirudh's statement above is always true with Sean. (TT)
 */
/*const char * const Morphology_Type_Labels[] =*/
/*{*/
/*"L2PC",     //!< 0: layer II pyramidal cell*/
/*"L3PC",     //!< 1: layer III pyramidal cell*/
/*"L4PC",     //!< 2: layer IV pyramidal cell*/
/*"L4SP",     //!< 3: layer IV star pyramidal cell*/
/*"L4SS",     //!< 4: layer IV spiny stellate cell*/
/*"L5CSPC",   //!< 5: layer V cortico-subcortical cell*/
/*"L5CHPC",   //!< 6: layer V cortico-hemispheric cell*/
/*"L6CTPC",   //!< 7: layer VI cortico-thalamic cell*/
/*"L6CCPC",   //!< 8: layer VI cortico-cortical cell*/
/*"L6CSPC",   //!< 9: layer VI cortico-subcortical cell*/
/*"L6FFPC",   //!< 10: layer 6 fusiform pyramidal cell*/
/*"HC",       //!< 11: Horizontal cell*/
/*"CRC",      //!< 12: Cajal Retzius cell*/
/*"MC",       //!< 13: Martinotti cell*/
/*"BTC",      //!< 14: Bitufted cell*/
/*"DBC",      //!< 15: Double Bouquet cell*/
/*"BP",       //!< 16: Bipolar cell*/
/*"NGC",      //!< 17: Neurogliaform cell*/
/*"LBC",      //!< 18: Large Basket cell*/
/*"NBC",      //!< 19: Nest Basket cell*/
/*"SBC",      //!< 20: Small Basket cell*/
/*"ChC",      //!< 21: Chandelier cell*/
/*"AHC",      //!< 22: Axon Horizontal cell*/
/*"ADC",      //!< 23: Axon Descending cell*/
/*"SAC"       //!< 24: Small Arborizing cell*/
/*};*/

// ----------------------------------------------------------------------------
//! Classification ID of the morphology (form) of a cell.
/*!
    \todo Refine Morphology_Type and Morphology_Reconstruction_Origin types.
*/
typedef boost::uint16_t                     Morphology_Type_ID; 

// ----------------------------------------------------------------------------

//! Classification of the morphology type (shape) of a cell.
// ---------------------------------------------------------------------------

//! Synapse type ( < 100 inhibitory, >= 100 excitatory).
enum Synapse_Type_ID
{
    SYNAPSE_TYPE_I1         = 0,    //!< inhibitory synapse type 1
    SYNAPSE_TYPE_I2         = 3,    //!< inhibitory synapse type 2
    SYNAPSE_TYPE_I3         = 6,    //!< inhibitory synapse type 3
    SYNAPSE_TYPE_E1         = 100,  //!< excitatory synapse type 1
    SYNAPSE_TYPE_E2         = 103,  //!< excitatory synapse type 2
    SYNAPSE_TYPE_E3         = 106,  //!< excitatory synapse type 3
    SYNAPSE_TYPE_PC_TO_MC   = 109   
    //!< excitatory synapse type from pyramidal cell to martinotti cell.
};

// ----------------------------------------------------------------------------

//! Names of the synapse type classes.
const char * const Synapse_Type_Descriptions[] =
{
    "Inhibitory Synapse Type 1",
    "Inhibitory Synapse Type 2",
    "Inhibitory Synapse Type 3",
    "Excitatory Synapse Type 1",
    "Excitatory Synapse Type 2",
    "Excitatory Synapse Type 3",
    "Excitatory Synapse Type Pyramidal to Martinotti cell"
};

// ----------------------------------------------------------------------------

//! inhibitory or excitatory synapse effect on postsynaptic membrane potential
enum Synapse_Effect_Sign
{
    INHIBITORY_SYNAPSE = -1,
    EXCITATORY_SYNAPSE = +1
};

// ---------------------------------------------------------------------------

class Synapse_Type
{
public:
    Synapse_Type()
    {}

    Synapse_Type(Synapse_Type_ID id)
        : _id(id)
    {}

    //! Ask if synapse type is excitatory.
    /*!
        @return true if synapse is of excitatory type
    */
    bool is_excitatory() const
    {
        return _id >= 100;
    }

    //! Ask if synapse type is inhibitory.
    /*!
        @return true if synapse is of inhibitory type
    */
    bool is_inhibitory() const
    {
        return _id <  100;
    }

    //! Get ID of the synapse type.
    Synapse_Type_ID id()
    {
        return _id;
    }

    //! Get description of the synapse type.
    inline const Label description() const;

private:
    Synapse_Type_ID _id;
};

// ----------------------------------------------------------------------------

//! target selection mode used in Blue Hub
enum Universe_Selection_Mode
{
    FULL,
    RANDOM,
    LINEAR,
    TARGET
};

// ----------------------------------------------------------------------------

// UNDEFINED VALUES

#ifdef SWIG
#define const %constant
#endif

//! Undefined column identifier.
const Column_GID                                UNDEFINED_COLUMN_GID  __attribute__((unused)) =
    std::numeric_limits<Column_GID>::max();
//! Undefined minicolumn identifier.
const Minicolumn_GID                            UNDEFINED_MINICOLUMN_GID  __attribute__((unused))=
    std::numeric_limits<Minicolumn_GID>::max();
//! Undefined cell identifier.
const Cell_GID                                  UNDEFINED_CELL_GID = 0;
//! Undefined section identifier.
const Section_ID                                UNDEFINED_SECTION_ID =
    std::numeric_limits<Section_ID>::max();
//! Undefined section identifier.
const Section_Branch_Order                      UNDEFINED_SECTION_BRANCH_ORDER __attribute__((unused)) =
    std::numeric_limits<Section_Branch_Order>::max();
//! Undefined segment identifier.
const Segment_ID                                UNDEFINED_SEGMENT_ID =
    std::numeric_limits<Segment_ID>::max();
//! Undefined segment identifier.
const Segment_Normalized_Distance  UNDEFINED_SEGMENT_NORMALIZED_DISTANCE __attribute__((unused)) =
    std::numeric_limits<Segment_Normalized_Distance>::max();
//! Undefined morphology point identifier.
const Morphology_Point_ID                       UNDEFINED_MORPHOLOGY_POINT_ID __attribute__((unused)) =
    std::numeric_limits<Morphology_Point_ID >::max() ;
//! Morphology undefined (NOT unclassified!).
const Morphology_Type_ID                        UNDEFINED_MORPHOLOGY_TYPE = 
    std::numeric_limits<Morphology_Type_ID>::max();
//! Electrophysiology undefined (NOT unclassified!).
const Electrophysiology_Type_ID                 UNDEFINED_ELECTROPHYSIOLOGY_TYPE __attribute__((unused)) =
    std::numeric_limits<Electrophysiology_Type_ID>::max();
//! Origin of the morphology reconstruction undefined.
const Morphology_Reconstruction_Origin     
UNDEFINED_MORPHOLOGY_RECONSTRUCTION_ORIGIN __attribute__((unused)) =
        std::numeric_limits<Morphology_Reconstruction_Origin>::max();
//! Undefined neuron local synapse identifier.
const Synapse_ID                                UNDEFINED_SYNAPSE_ID __attribute__((unused)) =
    std::numeric_limits<Synapse_ID>::max();
//! undefined synapse type
const Synapse_Type_ID                              UNDEFINED_SYNAPSE_TYPE_ID =
    std::numeric_limits<Synapse_Type_ID>::max();
//! Undefined cortical layer number.
const Cortical_Layer_Number                              UNDEFINED_CORTICAL_LAYER_NUMBER __attribute__((unused)) =
    std::numeric_limits<Cortical_Layer_Number>::max();
const Cell_Index                                UNDEFINED_CELL_INDEX =
    std::numeric_limits<Cell_Index>::max();

#ifdef SWIG
#undef const
#endif

// ----------------------------------------------------------------------------

// ARRAYS

typedef boost::shared_array<Cell_GID>               Cell_GID_Array;
typedef boost::shared_array<Section_ID>             Section_ID_Array;
typedef boost::shared_array<Segment_ID>             Segment_ID_Array;
typedef boost::shared_array<Section_Type>           Section_Type_Array;
typedef boost::shared_array<Section_Branch_Order>   Section_Branch_Order_Array;
typedef boost::shared_array<Section_Normalized_Distance>
    Section_Normalized_Distance_Array;
typedef boost::shared_array<Segment_Normalized_Distance>
    Segment_Normalized_Distance_Array;
typedef boost::shared_array<Morphology_Point_ID>    Morphology_Point_ID_Array;
typedef boost::shared_array<Synapse_Type>           Synapse_Type_Array;


// REPORT SPECIFICATION _______________________________________________________

//! Currently supported report format.
/*!
  
  \todo Actually only HDF5 is supported
*/
enum Report_Format
{
    HDF5_FORMAT = 0, 
    BINARY_FORMAT = 1, 
    ASCII_FORMAT = 2
};
//! Currently supported report types.
/*!
    \todo clarify that SOMA_REPORT is actually a COMPARTMENT_REPORT 
    with a chosen cell target instead of a compartment target and 
    discuss what should be done for that case (TT).
    should we remove SOMA_REPORT (JH) ?
    Reply: The difference is that in the soma report a cell target is passed
    while for a full compartment report a compartment target is passed
    so yes we should remove this and enable different target types in the
    future. (TT)
*/
enum Report_Type
{
    SOMA_REPORT = 0, 
    COMPARTMENT_REPORT = 1,
    SUMMATION_REPORT = 2
};
//! Variable reported i.e: membrane voltage, calcium concentration
typedef Label Report_Variable;
//! Unit name for the reported variable.
typedef Label Unit_Name;
}

/*!
    \todo This should be namespace bbp, not boost, so the boost
    namespace does not show up in documentation. (TT)
    enum to string conversion functions
*/
namespace boost
{
    //! Convert simulation report format to string.
    template<>
    inline std::string lexical_cast(const bbp::Report_Format & format);

    //! Convert simulation report type to string.
    template<>
    inline std::string lexical_cast(const bbp::Report_Type & type);
}

namespace bbp
{
//! type of the value in the simulation buffer
typedef float Simulation_Value ;
//! Local ID of the compartment inside a section of a neuron.
/*!
    The IDs are assigned linearly from the start of the section
    to the end of the section, starting with zero and + 1 for
    each consecutive compartment. This is similar to an index,
    but since this is section internal, it was labeled an ID to
    distinguish from indices inside a neuron or global report
    frame indices.
*/
typedef boost::uint16_t         Compartment_ID;
//! Undefined compartment ID
#ifdef SWIG
#define const %constant
#endif
 const Compartment_ID            UNDEFINED_COMPARTMENT_ID __attribute__((unused))
    = std::numeric_limits<Compartment_ID>::max();
#ifdef SWIG
#undef const
#endif
//! Global identifier of a neuron morphology volume compartment.
struct Compartment_GID
{
    Cell_GID        cell_gid;
    Section_ID      section_id;
    Compartment_ID  compartment_id;

    Compartment_GID() {}
    Compartment_GID(Cell_GID        cell_gid, 
                    Section_ID      section_id, 
                    Compartment_ID  compartment_id)
     : cell_gid(cell_gid),
       section_id(section_id), 
       compartment_id(compartment_id)
    {}
};

// MESH SPECIFICATION _________________________________________________________

typedef boost::uint32_t                         Vertex_Index;
typedef boost::shared_array<Vector_3D<Micron> > Vertex_Array;
typedef boost::shared_array<Vertex_Index>       Vertex_Index_Array;
typedef boost::uint32_t                         Triangle_Index;

// MICROCIRCUIT ATTRIBUTES ____________________________________________________


//!  Flagset used in microcircuit data loading.
/*!
    \sa Microcircuit_Loading_Flags
*/
typedef int Microcircuit_Loading_Flag_Set;

/*
// Specifies the microcircuit dataset attributes and location.   
class Microcircuit_Specification
{
public:        
    void structure(...
};
*/
        
        
                
// ----------------------------------------------------------------------------

//! Flagset used in microcircuit data loading.
/*!
    \todo Review Microcircuit_Loading_Flags and their documentation.(TT)
    \sa Microcircuit_Loading_Flag_Set
*/
enum Microcircuit_Loading_Flags
{
    //! Load neuron composition
    NEURONS              =   0x00000001,        
    //! Load morphologies
    MORPHOLOGIES         =   0x00000002,   
    //! Load mesh vertices and morphologies.
    MESH_VERTICES        =   0x00000004      | 
                             MORPHOLOGIES, 
    //! Load mesh triangles and morphologies.
    MESH_TRIANGLES       =   0x00000008      | 
                             MORPHOLOGIES,
    //! Load mesh triangles and morphologies.
    MESH_TRIANGLE_STRIPS =   0x00000010      | 
                            MORPHOLOGIES,
    //! Load mesh mapping and morphologies.
    MESH_MAPPING         =   0x00000020      | 
                             MORPHOLOGIES, 
    //! Load all data for meshes and morphologies.
    MESHES               =   0x00000040      | 
                             MORPHOLOGIES    | 
                             MESH_VERTICES   | 
                             MESH_TRIANGLES  | 
                             MESH_MAPPING, 
    //! Load which neurons are connected to the synapses.
    SYNAPSES_CONNECTIVITY        =   0x00000080, 
    //! Load the synapse constants conductance, utilization, depression facilitation.
    SYNAPSES_DYNAMICS_CONSTANTS  =   0x00000100,
    //! Load the type of the synapse (inhibitory, excitatory, and more).
    SYNAPSES_TYPE         = 0x00000200, 
    //! Load synapse position information (located where on morphology).
    SYNAPSES_POSITION     = 0x00000400,
    //! Load all information about the microcircuit connectivity.
    SYNAPSES              = SYNAPSES_CONNECTIVITY       |
                            SYNAPSES_DYNAMICS_CONSTANTS |
                            SYNAPSES_TYPE               |
                            SYNAPSES_POSITION,
    /*!
        \brief Load all afferent synapses to the specified cell target. 
        CAUTION: Needs to be used along with attribute flags!
        
        By default only those synapses whose pre and post synaptic neurons
        are in defined in the cell target specified for loading will be loaded.
        If this flag is specified all the afferent synapses (and no efferent)
        will be loaded for that cell target. 
     */
    AFFERENT_SYNAPSES       = 0x00000800,

    //! Load all data about this microcircuit. 
    /*! Unless AFFERENT_SYNAPSES is added only shared synapses will be 
        loaded. */
    MICROCIRCUIT        =   NEURONS             | 
                            MORPHOLOGIES        | 
                            MESHES              | 
                            SYNAPSES
};

// ----------------------------------------------------------------------------

//! Flags for loading synapse attributes.
/*!
    \sa Synapse_Attributes
*/
typedef int Synapse_Attributes_Flags;

//! Synapse attributes.
/*!
    The lower attributes are supersets of the fine grained single attributes
    in the top. Attributes can be combined with bitwise OR operations "|".
    \todo Complete and check this list. (TT)
    \sa Synapse, Synapses, Synapse_Reader, Synapse_Dataset
*/
enum Synapse_Attributes //  : boost::uint64_t
{
    SYNAPSE_NO_ATTRIBUTES                       =    0x00000000,
    //! Neuron presynaptic to the synapse (signaling to receiving 
    //! neuron via that synapse).
    SYNAPSE_PRESYNAPTIC_NEURON                  =    0x00000004,
    //! Neuron postsynaptic to the synapse (receiving from signaling 
    //! neuron through that synapse).
    SYNAPSE_POSTSYNAPTIC_NEURON                 =    0x00000008,
    
    //! Classification of the synapse type
    /*!
        For example classification of excitatory, inhibitory, and more 
        specific types.
    */
    SYNAPSE_TYPE                                =    0x00000400,
    //! Conductance of the synapse in nanosiemens unit.
    SYNAPSE_CONDUCTANCE                         =    0x00000010,
    //! Utilization probability of the synapse (neurotransmitter release).
    SYNAPSE_UTILIZATION                         =    0x00000020,
    //! Dynamic synapse facilitation time constant (short term plasticity).
    SYNAPSE_SHORT_TERM_FACILITATION             =    0x00000040,
    //! Dynamic synapse depression time constant (short term plasticity).
    SYNAPSE_SHORT_TERM_DEPRESSION               =    0x00000080,
    //! Absolute synaptic efficacy in millivolts.
    SYNAPSE_ABSOLUTE_SYNAPTIC_EFFICACY          =    0x10000000,
    //! Transmission delay of a synapse in milliseconds.
    SYNAPSE_DELAY                               =    0x00000100,
    //! Postsynaptic potential decay time constant.
    SYNAPSE_DECAY                               =    0x00000200,
    SYNAPSE_PRESYNAPTIC_MORPHOLOGY_TYPE         =    0x00400000,
    SYNAPSE_PRESYNAPTIC_SECTION_TYPE            =    0x00200000,
    SYNAPSE_PRESYNAPTIC_SECTION_BRANCH_ORDER    =    0x00001000,
    SYNAPSE_PRESYNAPTIC_SECTION                 =    0x00002000,
    SYNAPSE_PRESYNAPTIC_SEGMENT                 =    0x00004000,
    SYNAPSE_PRESYNAPTIC_SEGMENT_DISTANCE        =    0x00008000,

    SYNAPSE_POSTSYNAPTIC_MORPHOLOGY_TYPE        =    0x00800000,
    SYNAPSE_POSTSYNAPTIC_SECTION                =    0x00020000,
    SYNAPSE_POSTSYNAPTIC_SEGMENT                =    0x00040000,
    SYNAPSE_POSTSYNAPTIC_SEGMENT_DISTANCE       =    0x00080000,
    
    //! Connection information about which synapses connect which neurons.
    SYNAPSE_CONNECTION                    = 
        SYNAPSE_PRESYNAPTIC_NEURON         |
        SYNAPSE_POSTSYNAPTIC_NEURON,

    //! Constants characterizing the behavior (dynamics) of the synapse.
    SYNAPSE_DYNAMICS_CONSTANTS            =    
        SYNAPSE_CONDUCTANCE                |
        SYNAPSE_UTILIZATION                |
        SYNAPSE_SHORT_TERM_DEPRESSION      |
        SYNAPSE_SHORT_TERM_FACILITATION    |
        SYNAPSE_DELAY                      |
        SYNAPSE_DECAY,

    //! Location of the synapse on the presynaptic morphology.
    SYNAPSE_PRESYNAPTIC_POSITION            =
        SYNAPSE_PRESYNAPTIC_SECTION            |
        SYNAPSE_PRESYNAPTIC_SEGMENT            |
        SYNAPSE_PRESYNAPTIC_SEGMENT_DISTANCE,

    //! Location of the synapse on the postsynaptic morphology.
    SYNAPSE_POSTSYNAPTIC_POSITION           =
        SYNAPSE_POSTSYNAPTIC_SECTION           |
        SYNAPSE_POSTSYNAPTIC_SEGMENT           |
        SYNAPSE_POSTSYNAPTIC_SEGMENT_DISTANCE,

    //! Location of the synapse on the pre- and postsynaptic morphology.
    SYNAPSE_POSITION                        = 
        SYNAPSE_PRESYNAPTIC_POSITION        |
        SYNAPSE_POSTSYNAPTIC_POSITION,

    //! All information about the synapse (over 1 gigabyte for 10.000 neurons).
    /*!
        Only the attributes in the current version of the synapse
        specification of Blue Builder are included. Note that attributes
        that can be inferred from a loaded neuron morphology are not
        included in the loading to be storage efficient and reduce
        data redundancy. Therefore, morphology type and absolute synaptic efficacy
        are not included
    */
    SYNAPSE_ALL_ATTRIBUTES                      =
        SYNAPSE_CONNECTION                     |
        SYNAPSE_TYPE                            |
        SYNAPSE_DYNAMICS_CONSTANTS              |
        SYNAPSE_POSITION
};

//
//// ----------------------------------------------------------------------------
//
//// Triangle based surface geometry attributes.
//enum Mesh_Attributes
//  : boost::uint64_t
//{
//  //! Points in 3D space. (vertex = 3D vector + w).
//  MESH_VERTICES       =   ...,    
//  //! Triangles in 3D space.
//  MESH_TRIANGLES      =   ...,    
//  //! Mapping between mesh and generic objects or data (e.g. voltages).
//  MESH_MAPPING        =   ...,    
//  //! All mesh attributes (vertices, triangles, and mapping).
//  MESH                =           
//      MESH_VERTICES   |
//      MESH_TRIANGLES  |
//      MESH_MAPPING
//};
//
//// ----------------------------------------------------------------------------
//
//// Attributes of a volumetric shape (e.g. cell membrane).
//enum Morphology_Attributes 
//  : boost::uint64_t
//{
//  MORPHOLOGY_TYPE     =   ...,
//  MORPHOLOGY_SEGMENTS =   ...,   //!< Volumetric skeleton of the morphology.
//    MORPHOLOGY_MESH       =   MESH_ALL,
//  MORPHOLOGY_GEOMETRY =
//      MORPHOLOGY_SEGMENTS |
//      MORPHOLOGY_MESH,
//  MORPHOLOGY =
//      MORPHOLOGY_SEGMENTS |
//      MORPHOLOGY_MESH
//};
//
//// ----------------------------------------------------------------------------
//
//// Attributes of a neuron (nerve cell).
//enum Neuron_Attributes 
//  : boost::uint64_t
//{
//  NEURON_LABEL                    =   ...,
//  NEURON_ELECTROPHYSIOLOGY_TYPE   =   ...,
//  
//  NEURON_IDENTIFIER               =   ...,
//  NEURON_LAYER                    =   ...,
//  NEURON_POSITION                 =   ...,
//  NEURON_ORIENTATION              =   ...,
//  NEURON_COLUMN                   =   ...,
//  NEURON_MINICOLUMN               =   ...,
//
//  NEURON_PROPERTIES,
//  NEURON_DYNAMICS,
//
//  NEURON_STRUCTURE                =
//      NEURON_MORPHOLOGY_TYPE |
//      NEURON_MORPHOLOGY_GEOMETRY
//
//  NEURON_SYNAPSES                 =   SYNAPSE_ALL,
//  NEURON_ALL          =
//      NEURON_SPECIFICATION        |
//      NEURON_MORPHOLOGY           |
//      NEURON_SYNAPSES         
//};
//
//// ----------------------------------------------------------------------------
//
///*
//  Flagset used in microcircuit data loading.
//
//  When several flags are given, datasets will be loaded in the order they 
//  appear here: NEURONS, MORPHOLOGIES, MESHES, MESH_VERTICES, MESH_TRIANGLES,
//  MESH_MAPPING, SYNAPSES_CONNECTIVITY,SYNAPSES_GUDF, CONNECTIONS, SYNAPSES_POSITION_IN_SPACE,
//  SYNAPSES_AFFERENT, SYNAPSES_EFFERENT, SYNAPSES_ALL, SYNAPSES_SHARED.
//*/
//enum Microcircuit_Attributes 
//  : boost::uint64_t
//{
//  CIRCUIT_COMPOSITION,
//  NEURONS,
//  ,
//  NEURON_DYNAMICS,
//  SYNAPSES_CONNECTIVITY,
//
//  NEURONS             =   ALL_NEURON_ATTRIBUTES       ,
//  MORPHOLOGIES        =   ALL_MORPHOLOGY_ATTRIBUTES   ,
//  SYNAPSES            =   SYNAPSE_ALL_ATTRIBUTES      ,
//
//  NEURONS             =   NEURON_STRUCTURE    | 
//                          NEURON_DYNAMICS,
//  MICROCIRCUIT_CONNECTIVITY       =   SYNAPSE_ALL_ATTRIBUTES,
//
//  MICROCIRCUIT        =   MICROCIRCUIT_STRUCTURE  |
//                          MICROCIRCUIT_DYNAMICS,
//};
//
//
//enum Circuit_Attributes
//{
//  IDENTITY,       //!< Identity of cells.
//  GEOMETRY,       //!< Geometry of cells.
//  SYNAPSES_CONNECTIVITY,   //!< Connectivity between cells.
//  CLASSIFICATION, //!< Classification of cell structure and behavior.
//  PROPERTIES,     //!< Properties of cell structure.
//  DYNAMICS        //!< State of cell dynamics.
//};
//
//// Cell geometry.
//enum Cell_Geometry_Aspects
//{
//  CELL_SKELETON
//  CELL_MESH,
//  CELL_SYNAPSE_POSITIONS
//  CELL_SOMA_POSITION
//}
//
//// Classification of cell.
//enum Cell_Classification
//{
//  CELL_ELECTROPHYSIOLOGY_TYPE,
//  CELL_MORPHOLOGY_TYPE,
//  CELL_DATABASE_TYPE
//};
//
//// Classification of cell in a cortical column.
//enum Circuit_Classification
//{
//  CELL_LAYER_NUMBER,
//  CELL_MINICOLUMN_GID,
//  CELL_HYPERCOLUMN_GID
//};
//
//enum Circuit_Structure
//{
//  CIRCUIT_CELL_POSITION,
//  CIRCUIT_CELL_ORIENTATION,
//  CIRCUIT_CONNECTIVITY_STRUCTURE,
//};
//
//
//enum Cell_Structure_Aspects
//{
//  CELL_MORPHOLOGY_STRUCTURE   =   0x0000000000000002,
//  CELL_CONNECTIVITY_STRUCTURE =   
//  ...
//};
//
//enum Cell_Dynamics_Aspects
//{
//  CELL_MEMBRANE_POTENTIAL =   0x0000000000000001      ,
//  CELL_DYNAMICS           =   CELL_MEMBRANE_POTENTIAL
//};
//
//
//
//// Select aspects of the microcircuit.
//enum Circuit_Aspects  : boost::uint64_t
//{
//  CLASSIFICATION          =   CIRCUIT_CLASSIFICATION      | 
//                              CELL_CLASSIFICATION         |
//                              ION_CHANNEL_CLASSIFICATION,
//  STRUCTURE               =   CIRCUIT_STRUCTURE           |
//                              CELL_STRUCTURE              |
//                              ION_CHANNEL_STRUCTURE,
//  DYNAMICS                =   CIRCUIT_LEVEL_DYNAMICS      |
//                              CELL_LEVEL_DYNAMICS         |
//                              ION_CHANNEL_DYNAMICS
//};
//
//
//// ----------------------------------------------------------------------------
//
////  Flagset used in microcircuit data loading.
//typedef int Microcircuit_Loading_Flag_Set;



// LIBRARY INTERNALS __________________________________________________________

// NOTE: these types should be hidden from user visible interfaces!

//! internal array index of the neocortical column
typedef boost::uint16_t        Column_Index;
//! internal array index of the morphology
typedef boost::uint16_t        Morphology_Index;
//! internal array index of a synapse
/*!
    \todo move all internal array indices into implementation files
    not only the Synapse_Index. (TT)
*/
typedef size_t                 Synapse_Index;

#ifdef SWIG
#define const %constant
#endif
//! undefined synapse index
const Synapse_Index            UNDEFINED_SYNAPSE_INDEX =
    std::numeric_limits<Synapse_Index>::max();
//! interal array index inside a report frame (e.g. compartment voltage values)
typedef boost::uint64_t Report_Frame_Index;
const Report_Frame_Index       UNDEFINED_REPORT_FRAME_INDEX =
    std::numeric_limits<Report_Frame_Index>::max();
//! Report number in a list of reports belonging to an experiment
typedef boost::uint16_t         Report_Index;
//! Undefined report number
 const Report_Index              UNDEFINED_REPORT_INDEX __attribute__((unused))
    = std::numeric_limits<Report_Index>::max() ;
#ifdef SWIG
#undef const
#endif

// ----------------------------------------------------------------------------

const Label Synapse_Type::description() const
{
    if (_id == UNDEFINED_SYNAPSE_TYPE_ID)
        return "undefined";
    switch (_id) {
    case SYNAPSE_TYPE_I1:
        return Synapse_Type_Descriptions[0]; break;
    case SYNAPSE_TYPE_I2:
        return Synapse_Type_Descriptions[1]; break;
    case SYNAPSE_TYPE_I3:
        return Synapse_Type_Descriptions[2]; break;
    case SYNAPSE_TYPE_E1:
        return Synapse_Type_Descriptions[3]; break;
    case SYNAPSE_TYPE_E2:
        return Synapse_Type_Descriptions[4]; break;
    case SYNAPSE_TYPE_E3:
        return Synapse_Type_Descriptions[5]; break;
    case SYNAPSE_TYPE_PC_TO_MC: 
        return Synapse_Type_Descriptions[6]; break;
    default:;
    }

    return "unknown synapse ID";
}

// ----------------------------------------------------------------------------

}

#ifndef SWIG
namespace boost
{

template<>
std::string lexical_cast(const bbp::Report_Format & format)
{
    switch (format)
    {
    case bbp::HDF5_FORMAT: return "HDF5"; break;
    case bbp::BINARY_FORMAT: return "BINARY"; break;
    case bbp::ASCII_FORMAT: return "ASCII"; break;
    default: throw bad_lexical_cast();
    }
}

template<>
std::string lexical_cast(const bbp::Report_Type & type)
{
    switch (type)
    {
    case bbp::SOMA_REPORT: return "soma"; break;
    case bbp::COMPARTMENT_REPORT: return "compartment"; break;
    default: throw bad_lexical_cast();
    }
}

}
#endif

#endif
