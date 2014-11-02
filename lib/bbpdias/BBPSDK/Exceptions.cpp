/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler

*/

#include "BBP/Model/Microcircuit/Exceptions.h"

extern "C" {
inline void bbp_termination_callback()
{
    std::string verbose_level;
#ifndef WIN32
    bool found = bbp::getenv("BBP_VERBOSE_LEVEL", verbose_level);
#else
    verbose_level = "4";
    bool found = true;
#endif
    fprintf(stderr, "\nAborted.\n"
            "More verbose output maybe available setting the "
            "environmental variable BBP_VERBOSE_LEVEL to a number higher"
            " than the current one (%s)\n\n", 
            !found ? "variable unset, defaulting to 0." : 
                      verbose_level.c_str());

#ifdef __GNUC__
    __gnu_cxx::__verbose_terminate_handler();
#else
    //! \bug This snippet doesn't work if there is no active exception.
    try {
        throw;
    } catch (std::exception &exc) {
        fprintf(stderr, "terminate called after throwing a `%s'\n", 
            bbp::demangled_type_name(exc).c_str());
        fprintf(stderr, "  what(): %s\n", exc.what()); 
    } catch (...) {
        fprintf(stderr, "terminate called after throwing some exception\n");
    }
#endif
    abort();
}
}

// ----------------------------------------------------------------------------

namespace bbp
{

struct Termination_Init
{
    Termination_Init()
    {
        std::set_terminate(bbp_termination_callback);
    }
} _dummy_global_std_terminate_initialization;

// ----------------------------------------------------------------------------

// Exception texts for use in the Exception class template.
typedef const char * const Prefix;
Prefix _logic_error_tag::prefix =           "Logic error";
Prefix _runtime_error_tag::prefix =         "Runtime error";
Prefix _Unsupported_tag::prefix =           "Unsupported operation";
Prefix _Bad_Data_tag::prefix =              "Bad data";
Prefix _Data_Not_Found_tag::prefix =        "Data not found";
Prefix _Bad_Operation_tag::prefix =         "Bad operation";
Prefix _Bad_Initialization_tag::prefix =    "Bad initialization";
Prefix _Bad_Data_Source_tag::prefix =       "Bad data source";
Prefix _Undefined_Data_Source_tag::prefix = "Undefined data source";
Prefix _IO_Error_tag::prefix =              "Input/output error";
Prefix _Directory_Open_Error_tag::prefix =  "Directory open error";
Prefix _Error_File_Open_tag::prefix =       "File open error";
Prefix _File_Access_Error_tag::prefix =     "File access error";
Prefix _File_Parse_Error_tag::prefix =      "File parser error";
Prefix _Neuron_Not_Found_tag::prefix =      "Neuron not found";
Prefix _Morphology_Not_Found_tag::prefix =  "Morphology not found";
Prefix _Mesh_Not_Found_tag::prefix =        "Mesh not found";
Prefix _Target_Not_Found_tag::prefix =      "Target not found";
Prefix _Report_Not_Found_tag::prefix =      "Report not found";
Prefix _Section_Not_Found_tag::prefix =     "Section not found";
Prefix _Symbol_not_found_tag::prefix =      "Symbol not found";
Prefix _Microcircuit_Access_Error_tag::prefix = 
    "Microcircuit dataset access error";
// ----------------------------------------------------------------------------

}
