///*
//
//        Ecole Polytechnique Federale de Lausanne
//        Brain Mind Institute,
//        Blue Brain Project
//        (c) 2008. All rights reserved.
//
//        Responsible authors:    Thomas Traenkler
//*/
//
//#if defined(_MSC_VER)
//#pragma once
//#endif

//#ifndef BBP_NEURON_POINT_H
//#define BBP_NEURON_POINT_H
//
//#include "Types.h"
//#include "Microcircuit.h"
//#include "Compartment.h"
//
//namespace bbp {
//
//// ----------------------------------------------------------------------------
//
////! Central interface to data associated with a point on a neuron morphology.
///*!
//    \ingroup Microcircuit
//*/
//class Neuron_Point
//    : protected Section_Point_GID
//{
//public:
//    //! Constructs uninitialized neuron point.
//    Neuron_Point() {}
//    //! Constructs initialized neuron point.
//    Neuron_Point(Microcircuit_Ptr             microcircuit, 
//                 Cell_GID                     cell,
//                 Section_ID                   section, 
//                 Section_Normalized_Distance  distance);
//
//    /*!
//        \name Parent Structures
//        Get parent structures of this point on a neuron.
//    */
//    //@{
//    //! Get parent microcircuit in which this point is located on.
//    inline const Microcircuit &                 parent_microcircuit() const;
//    //! Get parent morphology where this point is located on the morphology.
//    inline const Morphology &                   parent_morphology() const;
//    //! Get parent neuron where this point is located on the morphology.
//    inline const Neuron &                       parent_neuron() const;
//    //! Get parent section where this point is located on the morphology.
//    inline const Section                        parent_section() const;
//    //! Get parent compartment where this point is located on the morphology.
//    inline const Compartment                    parent_compartment() const;
//    //// Get parent segment where this point is located on the morphology.
//    //inline const Segment                        parent_segment() const;
//    //@}
//
//    /*!
//        \name Structure Geometry
//        Get location on the parent structures of this point on a neuron.
//    */
//    //@{
//    //! Get position of the point in global coordinates.
//    inline const Vector_3D<Micron>              position() const;
//    //// Get diameter of point on the morphology.
//    //inline const Micron                         diameter() const;
//    //! Get the normalized distance of the point from the section start.
//    inline const Section_Normalized_Distance    section_distance() const;
//    //// Get the normalized distance of the point from the segment start.
//    //inline const Segment_Normalized_Distance    segment_distance() const;
//    //@}
//
//private:
//    //! Pointer to associated microcircuit.
//    Microcircuit_Ptr            _microcircuit;
//};
//
//inline std::ostream & operator << (std::ostream & lhs, const Morphology & rhs);
//
//
//// ----------------------------------------------------------------------------
//
//inline
//Neuron_Point::Neuron_Point(Microcircuit_Ptr             microcircuit, 
//                           Cell_GID                     cell,
//                           Section_ID                   section, 
//                           Section_Normalized_Distance  distance)
// : Section_Point_GID(cell, section, distance), _microcircuit(microcircuit)
//{
//}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Microcircuit & Neuron_Point::parent_microcircuit() const
//{
//    return * _microcircuit;
//}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Morphology & Neuron_Point::parent_morphology() const
//{
//    return _microcircuit->neuron(cell_gid).morphology();
//}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Neuron & Neuron_Point::parent_neuron() const
//{
//    return _microcircuit->neuron(cell_gid);
//}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Section Neuron_Point::parent_section() const
//{
//    return _microcircuit->neuron(cell_gid).section(section_id);
//}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Compartment Neuron_Point::parent_compartment() const
//{
//    return Compartment(_microcircuit, cell_gid, section_id, 
//        _microcircuit->neuron(cell_gid).compartment(section_id, distance),
//        _microcircuit->neuron(cell_gid).compartment_frame_index(
//            section_id, distance));
//}
//
//// ----------------------------------------------------------------------------
//
////inline 
////const Segment Neuron_Point::parent_segment() const
////{
////    return _microcircuit->neuron(cell_gid).section(section_id).
////        segment(segment_id);
////}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Vector_3D<Micron> Neuron_Point::position() const
//{
//    // TODO: Check if this retrieves segment info. (TT)
//    return _microcircuit->neuron(cell_gid).position(section_id, distance);
//}
//
//// ----------------------------------------------------------------------------
//
////inline 
////const Micron Neuron_Point::diameter() const
////{
////    // TODO: Check if this retrieves segment info. (TT)
////    return _microcircuit->neuron(cell_gid).diameter(section_id, distance);
////}
//
//// ----------------------------------------------------------------------------
//
//inline 
//const Section_Normalized_Distance Neuron_Point::section_distance() const
//{
//    return distance;
//}
//
//// ----------------------------------------------------------------------------
//
////inline 
////const Segment_Normalized_Distance Neuron_Point::segment_distance() const
////{
////    ...
////}
//
//// ----------------------------------------------------------------------------
//
//}
//#endif
