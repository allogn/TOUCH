/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	Juan Hernando Vieites

*/

#include <boost/filesystem.hpp>

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Experiment/Readers/detail/Compartment_Report_Reader_Impl.h"

#include "report_specialization_selection.h"
#include "File/Compartment_Report_HDF5_File_Reader.h"
#include "File/Compartment_Report_Binary_File_Reader.h"


namespace bbp
{

//-----------------------------------------------------------------------------
// Helper function
/* Returns a valid caller objects if and only if there is a valid report 
   reader specialization for the given final reader type and buffer type */
static inline Base_Report_Specialization_Caller *
search_specialization_caller(const std::string & final_class_typename,
                             const std::string & buffer_element_typename)
/* throw (Unsupported) */
{
    const Specialization_Table::const_iterator instance =
        global_report_reader_specialization_table.find
        (Instance_Type_Names(final_class_typename, buffer_element_typename));
    
    if (instance == global_report_reader_specialization_table.end())
    {
        std::stringstream msg;
        msg << "No final implementation for "
            << final_class_typename << ' ' << buffer_element_typename;
        throw_exception(Unsupported(msg.str()), 
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
    return instance->second.get();
}

//-----------------------------------------------------------------------------

// Specialization table definition
Specialization_Table global_report_reader_specialization_table;

//-----------------------------------------------------------------------------

Compartment_Report_Reader_Impl::pointer
Compartment_Report_Reader_Impl::create_reader(const Report_Specification & specs)
    /* throw (IO_Error, Bad_Data) */
{
    Filepath path = uri_to_filename(specs.data_source());
    if (path != "") 
    {
        if (boost::filesystem::is_directory(path) &&
            boost::filesystem::exists(path) &&
            specs.format() == HDF5_FORMAT) {
            // A directory containing h5 files is expected
            return pointer(new Compartment_Report_HDF5_File_Reader(specs));
        }
        if (specs.format() == BINARY_FORMAT &&
            boost::filesystem::exists(path)) {
            // Both directories and single files are accepted.
            return pointer(new Compartment_Report_Binary_File_Reader(specs));
        }
    }
    else
    {
        // No suitable reader found
        throw_exception(Bad_Data("No suitable compartment report reader for '" + 
                                 specs.data_source() + "' found"),
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
    return Compartment_Report_Reader_Impl::pointer();
}

//-----------------------------------------------------------------------------

void Compartment_Report_Reader_Impl::load_frames_delegate
(const std::string & buffer_element_typename, void * buffer,
 Frame_Number first, Frame_Number last)
    /* throw (IO_Error, Bad_Data) */
{
    search_specialization_caller(typeid(*this).name(),
                                 buffer_element_typename)->
        load_frames(this, buffer, first, last);
}

//-----------------------------------------------------------------------------

bool Compartment_Report_Reader_Impl::load_next_frame_delegate
(const std::string & buffer_element_typename, void * buffer)
    /* throw (IO_Error, Bad_Data) */
{
    return search_specialization_caller(typeid(*this).name(),
                                        buffer_element_typename)->
        load_next_frame(this, buffer);
}

//-----------------------------------------------------------------------------


}
