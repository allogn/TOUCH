/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_PROCESS_INTERFACE_H
#define BBP_PROCESS_INTERFACE_H

#include "Process_Control_Interface.h"
#include "BBP/Common/Exception/Exception.h"

namespace bbp {

// ----------------------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
struct _End_Of_Stream_tag;
struct _Stream_Interruption_tag;
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

//! End of stream exception.
/*!
    If the BBP_NOT_VERBOSE macro is defined, the error messages are 
    suppressed on the console and the user is expected to handle the exceptions
    properly. 
*/
typedef Exception<_End_Of_Stream_tag, std::exception, false>
    End_Of_Stream;

//! Stream interruption exception.
/*!
    If the BBP_NOT_VERBOSE macro is defined, the error messages are 
    suppressed on the console and the user is expected to handle the exceptions
    properly. 
*/
typedef Exception<_Stream_Interruption_tag, std::exception, false>
    Stream_Interruption;

// ----------------------------------------------------------------------------

//! Abstract interface to process stream elements.
/*!
    \ingroup Process_Interface
*/
class Process_Interface
    //: public virtual Process_Control_Interface
{
public:
    Process_Interface() {}
    virtual ~Process_Interface() {}

    //! data processing step
    virtual void process() = 0;
    //virtual void process() throw (End_Of_Stream) = 0;
};

}
#endif
