/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2006-2007. All rights reserved.

                Responsible author:      Thomas Traenkler
                Contributing authors:    Sebastien Lasserre

*/

#include <sstream>
#include <cctype>

#include "BBP/Common/String/String.h"
#include "BBP/Model/Experiment/Experiment.h"
#include "BBP/Model/Experiment/Readers/Parsers/Blue_Config_File_Parser.h"
#include "BBP/Model/Experiment/Readers/File/Blue_Config_File_Reader.h"
#include "BBP/Model/Microcircuit/Exceptions.h"

namespace bbp {

// HELPER FUNCTIONS -----------------------------------------------------------

static void to_lower(std::string & str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

//-----------------------------------------------------------------------------

static Report_Type string_to_report_type(const std::string &str)
    throw (std::bad_cast)
{
    std::string tmp = str;
    to_lower(tmp);
    if (tmp == "compartment")
        return COMPARTMENT_REPORT;
    else if (tmp == "soma")
        return SOMA_REPORT;
    else if (tmp == "summation")
        return SUMMATION_REPORT;
    else
        throw std::bad_cast();
}

//-----------------------------------------------------------------------------

static Report_Format string_to_report_format(const std::string &str)
    throw (std::bad_cast)
{
    std::string tmp = str;
    to_lower(tmp);
    if (tmp == "hdf5")
        return HDF5_FORMAT;
    else if ((tmp == "binary") || (tmp == "bin"))
        return BINARY_FORMAT;
    else if (tmp == "ascii") 
        return ASCII_FORMAT;
    else
        throw std::bad_cast();
}

// ----------------------------------------------------------------------------

Blue_Config_File_Reader::Blue_Config_File_Reader
(Filepath blue_config_filepath)
: _blue_config_filename(blue_config_filepath)
{
}

// ----------------------------------------------------------------------------

Blue_Config_File_Reader::Blue_Config_File_Reader
(Filepath blue_config_filepath,
 const Directory & original_prefix,
 const Directory & replacement_prefix)
: _blue_config_filename(blue_config_filepath),
  _original_prefix(original_prefix),
  _replacement_prefix(replacement_prefix)
{
}

// ----------------------------------------------------------------------------

void Blue_Config_File_Reader::read(Experiment & experiment)
{
    // replace path prefix, e.g. "/bglscratch" with a mapped drive letter
    if (_replacement_prefix.empty() == false)
    {
        _blue_config_filename = replace_prefix(
            _blue_config_filename.file_string(),
            _original_prefix.directory_string(),
            _replacement_prefix.directory_string());
    }

    std::ifstream test;

    test.open(_blue_config_filename.file_string().c_str(), 
        std::ios::in | std::ios::binary);
    if (test.is_open() == false)
    {
        throw_exception(
            File_Access_Error("Blue Config file could not be opened: " +
                              _blue_config_filename.file_string()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    else
    {
        test.close();
    }


    Blue_Config_File_Parser blue_config_parser(
        _blue_config_filename.file_string());
    this->_blue_config_filename = _blue_config_filename.file_string();

    // READ RUN SECTION _______________________________________________________

    std::list<std::string> runList ; 
    try
    {
        blue_config_parser.getSectionNameList("Run",runList);
    }
    catch (bbp::File_Parse_Error)
    {
        throw_exception(
            bbp::File_Parse_Error("Blue_Data::Blue_Config_File_Reader::read"
                                  ": problem to get the section Run"),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    std::list<std::string>::iterator lstItr = runList.begin () ;
    blue_config_parser._runName = (*lstItr) ;
    blue_config_parser.getRunSectionValue("Date", date);
    blue_config_parser.getRunSectionValue("Time", time);
    blue_config_parser.getRunSectionValue("svnPath", svn_url);
    blue_config_parser.getRunSectionValue("Version", svn_revision);
    blue_config_parser.getRunSectionValue("Prefix", directory_prefix);
    blue_config_parser.getRunSectionValue("MorphologyPath", 
        morphologies_directory);
    blue_config_parser.getRunSectionValue("ForwardSkip", forward_skip);
    blue_config_parser.getRunSectionValue("Duration", duration);
    blue_config_parser.getRunSectionValue("Dt", timestep);
    blue_config_parser.getRunSectionValue("Note", description);
    blue_config_parser.getRunSectionValue("TargetFile", target_filename);
    blue_config_parser.getRunSectionValue("OutputRoot", output_directory);
    blue_config_parser.getRunSectionValue("CurrentDir", current_directory);
    blue_config_parser.getRunSectionValue("MeshPath", mesh_directory);
    blue_config_parser.getRunSectionValue("nrnPath", nrn_directory);
    blue_config_parser.getRunSectionValue("CircuitPath", 
        microcircuit_directory);
    blue_config_parser.getRunSectionValue("METypePath", metype_directory);
    blue_config_parser.getRunSectionValue("CircuitTarget", circuit_target);
    
    // replace prefix (e.g. /bglscratch with the mapped drive letter)
    if (_replacement_prefix.empty() == false)
    {
        directory_prefix = replace_prefix(
            Directory(directory_prefix).directory_string(), 
                 _original_prefix.directory_string(),
                 _replacement_prefix.directory_string());
        morphologies_directory = replace_prefix(
            Directory(morphologies_directory).directory_string(),
             _original_prefix.directory_string(), 
             _replacement_prefix.directory_string());
        metype_directory = replace_prefix(
            Directory(metype_directory).directory_string(), 
             _original_prefix.directory_string(),
             _replacement_prefix.directory_string());
        microcircuit_directory = replace_prefix(
            Directory(microcircuit_directory).directory_string(), 
             _original_prefix.directory_string(),
             _replacement_prefix.directory_string());
        nrn_directory = replace_prefix(
            Directory(nrn_directory).directory_string(), 
            _original_prefix.directory_string(),
            _replacement_prefix.directory_string());
        mesh_directory = replace_prefix(
            Directory(mesh_directory).directory_string(), 
             _original_prefix.directory_string(),
             _replacement_prefix.directory_string());
        current_directory = replace_prefix(
            Directory(current_directory).directory_string(), 
             _original_prefix.directory_string(),
             _replacement_prefix.directory_string());
        output_directory = replace_prefix(
            Directory(output_directory).directory_string(), 
             _original_prefix.directory_string(),
             _replacement_prefix.directory_string());
        target_filename = replace_prefix(
            Directory(target_filename).directory_string(), 
             _original_prefix.directory_string(),
             _replacement_prefix.directory_string());
    }

    // fill the experiment using the Experiment_Accessor api
    prefix_source(experiment) = directory_prefix;
    circuit_source(experiment) = microcircuit_directory;
    synapse_source(experiment) = nrn_directory;
    morphologies_source(experiment) = morphologies_directory;
    metypes_source(experiment) = metype_directory;
    meshes_source(experiment) = mesh_directory;
    user_target_source(experiment) = target_filename;
    Experiment_Accessor::date(experiment) = date;
    Experiment_Accessor::time(experiment) = time;
    label(experiment) = blue_config_parser._runName;
    subversion_link(experiment).url(svn_url);
    subversion_link(experiment).revision(svn_revision);
    Experiment_Accessor::description(experiment) = description;
    if (forward_skip != "")
        Experiment_Accessor::forward_skip(experiment) = 
            boost::lexical_cast<bbp::Millisecond>(forward_skip);
    else
        Experiment_Accessor::forward_skip(experiment) = 0;
    Experiment_Accessor::duration(experiment) = 
        boost::lexical_cast<bbp::Millisecond>(duration);
    Experiment_Accessor::timestep(experiment) = 
        boost::lexical_cast<bbp::Millisecond>(timestep);
    Experiment_Accessor::circuit_target(experiment) = circuit_target;
    //connection_scaling(experiment) = 

    // CONNECTION SCALINGS ____________________________________________________

    //! \bug Connection scaling is not taken into account in Synapse class (TT)
    
    // extract the data from the blueConfig file and init the scalings
    std::list<std::string> connection_scaling_list;
    
    // extract the number of scalings from the blueConfig file
    try {
        blue_config_parser.getSectionNameList("Connection", 
            connection_scaling_list) ;
    }
    catch (bbp::File_Parse_Error)
    {
        log_message("No connection scalings found", INFORMATIVE_LEVEL);
    }

    // creation of the connection scalings
    for(std::list<std::string>::iterator name = connection_scaling_list.begin();
        name != connection_scaling_list.end();
        name++)
    {
        Blue_Config_File_Parser & parser = blue_config_parser;

        std::string source, destination, weight;
        try {
            parser.getSectionValue("Connection", *name, 
                "Source", source);
            parser.getSectionValue("Connection", *name, 
                "Destination", destination);
            parser.getSectionValue("Connection", *name, "Weight", weight);
        }
        catch (bbp::File_Parse_Error)
        {
            throw_exception(bbp::File_Parse_Error
                            ("Blue_Data::Blue_Config_File_Reader::read"
                             ": problem reading Connection: " + *name),
                            FATAL_LEVEL, __FILE__, __LINE__);
        }

        Connection_Scaling scaling;

        scaling.label(*name);
        scaling.source(source);
        scaling.destination(destination);

        try 
        {
            scaling.factor(boost::lexical_cast<Scaling_Factor>(weight));
        }
        catch (std::bad_cast &)
        {
            std::stringstream msg;
            msg << "Blue_Data::Blue_Config_File_Reader::read: Reading scaling "
                "factor: "
                << *name << ": bad " << "factor" << "(" << weight << ")";
            throw_exception(File_Parse_Error(msg.str()),
                            FATAL_LEVEL, __FILE__, __LINE__);
        }
        
        experiment.connection_scalings().insert(scaling);
    }

    // STIMULUS SPECIFICATIONS ________________________________________________

    //// extract the data from the blueConfig file and init the stimuli
    //std::list<std::string> stimulus_list;
    //std::string stmp;
    //
    //// extract the number of stimuli from the blueConfig file
    //try {
    //    blue_config_parser.getSectionNameList("Stimulus", 
    //        stimulus_list) ;
    //}
    //catch (bbp::File_Parse_Error)
    //{
    //    log_message("No stimuli found", INFORMATIVE_LEVEL);
    //}

    //// creation of the connection scalings
    //for(std::list<std::string>::iterator name = stimulus_list.begin();
    //    name != stimulus_list.end();
    //    name++)
    //{
    //    Blue_Config_File_Parser & parser = blue_config_parser;

    //    std::string mode, pattern, ampstart, lambda, weight,
    //        num_of_synapses, delay, duration;
    //    try {
    //        parser.getSectionValue("Stimulus", *name, 
    //            "Mode", mode);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "Pattern", pattern);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "AmpStart", ampstart);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "Lambda", lambda);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "Weight", weight);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "NumOfSynapses", num_of_synapses);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "Delay", delay);
    //        parser.getSectionValue("Stimulus", *name, 
    //            "Duration", duration);
    //    }
    //    catch (bbp::File_Parse_Error)
    //    {
    //        throw_exception(bbp::File_Parse_Error
    //                        ("Blue_Data::Blue_Config_File_Reader::read"
    //                         ": problem reading Stimulus: " + *name),
    //                        FATAL_LEVEL, __FILE__, __LINE__);
    //    }

    //    Stimulus_Specification stimulus_spec;

    //    stimulus_spec.label(*name);
    //    stimulus_spec.pattern(pattern);
    //    ...

    //    try 
    //    {
    //        stimulus_spec.onset( boost::lexical_cast<Millisecond>(delay));
    //    }
    //    catch (std::bad_cast &)
    //    {
    //        std::stringstream msg;
    //        msg << "Blue_Data::Blue_Config_File_Reader::read: Reading stimulus "
    //            "specification: "
    //            << *name << ": bad onset (" << string << ")";
    //        throw_exception(File_Parse_Error(msg.str()),
    //                        FATAL_LEVEL, __FILE__, __LINE__);
    //    }
    //    try 
    //    {
    //        stimulus_spec.duration( boost::lexical_cast<Millisecond>(duration));
    //    }
    //    catch (std::bad_cast &)
    //    {
    //        std::stringstream msg;
    //        msg << "Blue_Data::Blue_Config_File_Reader::read: Reading stimulus "
    //            "specification: "
    //            << *name << ": bad duration (" << string << ")";
    //        throw_exception(File_Parse_Error(msg.str()),
    //                        FATAL_LEVEL, __FILE__, __LINE__);
    //    }
    //    
    //    experiment.connection_scalings().insert(scaling_factor);
    //}


    // READ REPORT SECTIONS ___________________________________________________
        
    // extract the data from the blueConfig file and init the reports
    std::list<std::string> report_list;
    std::string stmp;
    
    // extract the number of report from the blueConfig file
    try {
        blue_config_parser.getSectionNameList("Report", report_list) ;
    }
    catch (bbp::File_Parse_Error)
    {
        log_message("No reports found", INFORMATIVE_LEVEL);
    }

    // creation of the reports
    for(std::list<std::string>::iterator name = report_list.begin();
        name != report_list.end();
        name++)
    {
        Blue_Config_File_Parser &parser = blue_config_parser;

        std::string target, type, variable, unit, format, delta, end, start;
        try {
            parser.getSectionValue("Report", *name, "Target", target);
            parser.getSectionValue("Report", *name, "Type", type);
            parser.getSectionValue("Report", *name, "ReportOn", variable);
            parser.getSectionValue("Report", *name, "Unit", unit);
            parser.getSectionValue("Report", *name, "Format", format);
            parser.getSectionValue("Report", *name, "Dt", delta);
            parser.getSectionValue("Report", *name, "StartTime", start);
            parser.getSectionValue("Report", *name, "EndTime", end);
        }
        catch (bbp::File_Parse_Error)
        {
            throw_exception(bbp::File_Parse_Error
                            ("Blue_Data::Blue_Config_File_Reader::read"
                             ": problem reading Report: " + *name),
                            FATAL_LEVEL, __FILE__, __LINE__);
        }

        Report_Specification report;

        report.label(*name);
        report.target(target);
        report.variable(variable);
        report.unit(unit);

#define STRING_TO_ATTRIBUTE_CONVERSION(string, member, conversion, field_name)\
    try {                                                                     \
        report.member(conversion(string));                                    \
    }                                                                         \
    catch (std::bad_cast &)                                                   \
    {                                                                         \
        std::stringstream msg;                                                \
        msg << "Blue_Data::Blue_Config_File_Reader::read: Reading Report: "   \
            << *name << ": bad " << field_name << "(" << string << ")";       \
        throw_exception(File_Parse_Error(msg.str()),                          \
                        FATAL_LEVEL, __FILE__, __LINE__);  \
    }

        STRING_TO_ATTRIBUTE_CONVERSION(format, format, 
                                       string_to_report_format,
                                       "Format");
        STRING_TO_ATTRIBUTE_CONVERSION(type, type, 
                                       string_to_report_type,
                                       "Type");
        STRING_TO_ATTRIBUTE_CONVERSION(start, start_time,
                                       boost::lexical_cast<Millisecond>,
                                       "StartTime");
        STRING_TO_ATTRIBUTE_CONVERSION(end, end_time,
                                       boost::lexical_cast<Millisecond>,
                                       "EndTime");
        STRING_TO_ATTRIBUTE_CONVERSION(delta, timestep,
                                       boost::lexical_cast<Millisecond>,
                                       "Dt");
        report.data_source(output_directory);

        report_specs(experiment).insert(report);
    }
#undef STRING_TO_ATTRIBUTE_CONVERSION
}

//-----------------------------------------------------------------------------

}
