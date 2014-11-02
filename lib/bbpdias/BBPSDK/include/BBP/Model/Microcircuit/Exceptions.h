/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MICROCIRCUIT_EXCEPTIONS_H
#define BBP_MICROCIRCUIT_EXCEPTIONS_H

#include "BBP/Common/Exception/Exceptions.h"

namespace bbp {
// ----------------------------------------------------------------------------
// MICROCIRCUIT EXCEPTIONS
// ----------------------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
struct _Report_Not_Found_tag { static const char * const prefix; };
struct _Neuron_Not_Found_tag { static const char * const prefix; };
struct _Morphology_Not_Found_tag { static const char * const prefix; };
struct _Mesh_Not_Found_tag { static const char * const prefix; };
struct _Target_Not_Found_tag { static const char * const prefix; };
struct _Section_Not_Found_tag { static const char * const prefix; };
struct _Microcircuit_Access_Error_tag { static const char * const prefix; };
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

typedef Exception<_Report_Not_Found_tag, std::runtime_error> 
	Report_Not_Found;
typedef Exception<_Neuron_Not_Found_tag, std::runtime_error> 
	Neuron_Not_Found;
typedef Exception<_Morphology_Not_Found_tag, std::runtime_error> 
	Morphology_Not_Found;
typedef Exception<_Mesh_Not_Found_tag, std::runtime_error> 
	Mesh_Not_Found;
typedef Exception<_Section_Not_Found_tag, std::runtime_error> 
	Section_Not_Found;
typedef Exception<_Target_Not_Found_tag, std::runtime_error> 
	Target_Not_Found;
typedef Exception<_Microcircuit_Access_Error_tag, std::runtime_error> 
	Microcircuit_Access_Error;

// ----------------------------------------------------------------------------

}
#endif
