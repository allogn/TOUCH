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

#ifndef BBP_EXCEPTIONS_H
#define BBP_EXCEPTIONS_H

#include "Exception.h"

// ----------------------------------------------------------------------------
// GENERIC EXCEPTIONS
// ----------------------------------------------------------------------------

namespace bbp
{

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
struct _logic_error_tag              { static const char * const prefix; };
struct _runtime_error_tag            { static const char * const prefix; };
struct _Unsupported_tag              { static const char * const prefix; };
struct _Bad_Data_tag                 { static const char * const prefix; };
struct _Data_Not_Found_tag           { static const char * const prefix; };
struct _Bad_Operation_tag            { static const char * const prefix; };
struct _Bad_Initialization_tag       { static const char * const prefix; };
struct _Bad_Data_Source_tag          { static const char * const prefix; };
struct _Undefined_Data_Source_tag    { static const char * const prefix; };
struct _IO_Error_tag                 { static const char * const prefix; };
struct _Directory_Open_Error_tag     { static const char * const prefix; };
struct _Error_File_Open_tag          { static const char * const prefix; };
struct _File_Access_Error_tag        { static const char * const prefix; };
struct _File_Parse_Error_tag         { static const char * const prefix; };
struct _Symbol_not_found_tag         { static const char * const prefix; };
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

typedef
    Exception<_logic_error_tag, std::logic_error>       logic_error;
typedef
    Exception<_runtime_error_tag, std::runtime_error>   runtime_error;
typedef
    Exception<_Unsupported_tag, std::runtime_error>     Unsupported;
typedef
    Exception<_Bad_Data_tag, std::runtime_error>        Bad_Data;
typedef
    Exception<_Data_Not_Found_tag, std::runtime_error>  Data_Not_Found;
typedef
    Exception<_Bad_Operation_tag, std::runtime_error>   Bad_Operation;
typedef
    Exception<_Bad_Initialization_tag, Bad_Operation>   Bad_Initialization;
typedef
    Exception<_Bad_Data_Source_tag, Bad_Data>           Bad_Data_Source;
typedef
    Exception<_Undefined_Data_Source_tag, Bad_Data>     Undefined_Data_Source;
typedef
    Exception<_IO_Error_tag, std::runtime_error>        IO_Error;
typedef
    Exception<_Directory_Open_Error_tag, IO_Error>      Directory_Open_Error;
typedef
    Exception<_Error_File_Open_tag, IO_Error>           File_Open_Error;
typedef
    Exception<_File_Access_Error_tag, IO_Error>         File_Access_Error;
typedef
    Exception<_File_Parse_Error_tag, IO_Error>          File_Parse_Error;
typedef
    Exception<_Symbol_not_found_tag, IO_Error>          Symbol_not_found;

}
#endif
