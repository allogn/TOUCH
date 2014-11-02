/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Responsible authors: Garik Suess
                             Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_BINARY_FILE_PARSER_H
#define BBP_COMPARTMENT_REPORT_BINARY_FILE_PARSER_H

#ifdef WIN32
/*
    On the Microsoft Windows 32-bit platform, fstream does not support
    seeking files beyond 2GB, therefore Windows specific code is used
    as a workaround to this issue.
*/
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#undef MIN
#undef MAX
#else
#include <fstream>
#endif

#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

#include "BBP/Common/Types.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Experiment/Mappings/Compartment_Report_Mapping.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Common/System/endian_swap.h"
#include "BBP/Common/System/Time/Timer.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Type for representing a mapping (here of a compartment to a section).
typedef float           Mapping_Item;
//! Type for representing an extra mapping value.
typedef boost::int32_t	Extra_Mapping_Item;

// ----------------------------------------------------------------------------

//! Data structure for one cellinfo entry in the cellinfo header block.
struct Cell_Info
{
    boost::int32_t		gid;
    boost::int32_t		number_of_compartments;
    //! offset of the section mapping from beginning of file
    boost::uint64_t		mapping_offset;
    //! offset of the extra mapping from the beginning of file
    boost::uint64_t		extra_mapping_offset;
    //! offset of the compartment data of frame 1 from beginning of file
    boost::uint64_t		data_offset;
};

// ----------------------------------------------------------------------------

//! Sort cellinfo structure by cell GID.
bool sort_by_gid(const Cell_Info & i, const Cell_Info & j);

// HEADER OFFSET POSITIONS ----------------------------------------------------

/*
    DISCLAIMER: These positions are absolute offsets from file start to 
    identify the positions of the header information in the file. Do not 
    change to maintain backwards compatibility!
*/

//! double value identifying the byte order of the file
/*!
 If identifier read at position 0 matches ARCHITECTURE_IDENTIFIER,
 then the file was writting from native architecture
*/
const double ARCHITECTURE_IDENTIFIER = 1.001;
//! size of char *'s stored in the header (int32_t)
const boost::int32_t SIZE_STRING = 16;

// ----------------------------------------------------------------------------

//! Offsets of the header information in the file.
enum Compartment_Report_Binary_File_Header_Positions
{
    //! position of the double value identifying the byte order of the file
    IDENTIFIER_POSITION = 0,
    //! offset of header information (int32_t) past the architecture identifier
    HEADER_SIZE_POSITION = IDENTIFIER_POSITION + sizeof(double),
    //! version of the reader library? (char *)
    LIBRARY_VERSION_POSITION = 16,
    //! version of the simulator used in the simulation (char *)
    SIMULATOR_VERSION_POSITION = 32,
    //! number of cells in each frame of the report (int32_t)
    TOTAL_NUMBER_OF_CELLS_POSITION = 48,
    //! number of compartments in a frame (int32_t)
    TOTAL_NUMBER_OF_COMPARTMENTS_POSITION = 52,
    //! number of frames in the report (int32_t)
    NUMBER_OF_STEPS_POSITION = 64,
    //! time where the report starts in specified time unit (double)
    TIME_START_POSITION = 72,
    //! time where the report ends in specified time unit (double)
    TIME_END_POSITION = 80,
    //! timestep between two report frames (double)
    DT_TIME_POSITION = 88,
    //! unit of the report data (char *)
    D_UNIT_POSITION = 96,
    //! time unit of the report (char *)
    T_UNIT_POSITION = 112,
    //! size of the mapping (int32_t)
    MAPPING_SIZE_POSITION = 128,
    //! name of the mapping (char *)
    MAPPING_NAME_POSITION = 144,
    //! size of the extra mapping (int32_t)
    EXTRA_MAPPING_SIZE_POSITION = 160,
    //! name of the extra mapping (char *)
    EXTRA_MAPPING_NAME_POSITION = 176,
    //! name of the report (char *)
    REPORT_NAME_POSITION = 192,
    //! length of the header (int32_t)
    HEADER_LENGTH = 1024
};

// ----------------------------------------------------------------------------

//! Offsets of data tokens inside a cell info block inside the file header.
enum Compartment_Report_Binary_File_Cell_Info_Header_Positions
{
    //! Cell_GID (int32_t)
    NUMBER_OF_CELL_POSITION = 0,
    //! number of compartments of this cell (int32_t)
    NUMBER_OF_COMPARTMENTS_POSITION = 8,
    //! data info offset (int32_t)
    DATA_INFO_POSITION = 16,
    //! extra mapping info offset (int32_t)
    EXTRA_MAPPING_INFO_POSITION = 24,
    //! mapping info offset (int32_t)
    MAPPING_INFO_POSITION = 32,
    //! size of the cellinfo block (int32_t)
    SIZE_CELL_INFO_LENGTH = 64
};

// ----------------------------------------------------------------------------

/*!
    \brief Parser for compartment report binary files containing simulation
    data written by the simulator.

    This parser while supporting of reading subtargets of the reported target,
    will internally currently always read a whole frame, regardless of the
    cell target chosen in order to reduce seeks. When frame sizes of the full
    target become to large to fit in memory, either the simulation should be
    run with a smaller target to be reported, or the parser can be refined.
    For now, this is a better solution than a naive implementation that would
    skip the cells not in the subtarget as this would require many seeks and
    degrade reader performance significantly as with the previous
    implementation. Even for 100,000 cells circuits on a workstation with
    2 GB of memory, this is probably not a problem as a single frame is then
    350 compartments * 100,000 cells * sizeof(float) = ~133 MB. For
    efficiency, improving the reading for subtargets should be done in a
    preprocessing step that edits an existing report and writes a new file.
*/
class Compartment_Report_Binary_File_Parser
{
public:
    //! Construct an uninitialized parser (no file opened - use open() ).
    Compartment_Report_Binary_File_Parser();
    //! Construct a parser and open specified file, reading specified target.
    Compartment_Report_Binary_File_Parser(Filepath report_file);

    //! Construct a parser and open specified file, reading specified target.
    Compartment_Report_Binary_File_Parser(Filepath report_file,
                                          const Cell_Target & cells);

    //! Open the compartment report file.
    bool open(Filepath report);
    //! Close the compartment report file.
    void close();

    //! Reads the next report frame in the report file and advances after.
    /*! If the end of the file has been reached, an exception is thrown. */
    bool load_next_frame(Simulation_Value * buffer);
    //! Seek to a specific frame number in the file.
    /*!
        \todo Throw an exception when frame number out of bounds.
     */
    void jump_to_frame(Frame_Number frame_number_to_seek);

    //! Get frame number of the frame next to be read.
    Frame_Number current_framestamp() const;
    //! Number of reported compartments in the frame.
    /*! Takes into account the set cell target. */
    size_t frame_size() const;
    /*!
        \brief Get the compartment report frame mapping that specifies
        to which cell and section a compartment belongs to.
     */
    const Compartment_Report_Mapping & mapping() const;

    //! Get cell target specifying the subset of cells to be read.
    const Cell_Target & cell_target() const;
    //! Set cell target specifying the subset of cells to be read.
    void cell_target(const Cell_Target & cells);

private:
    //! Parse a variable from a raw chunk read from a file.
    /*!
     @param position Is the position in byte (char) from the buffer start.
     */
    template <typename T>
    T parse_var(char * buffer, size_t position);
    //! Parse a string from a raw chunk read from a file.
    std::string parse_string(char * buffer, size_t position);
    //! Parses the header of the report file.
    /*!
        Currently extra mapping is not parsed/supported
        by this parser implementation.
     */
    void parse_header();
    //! Reads and then parses an array from a file, adjusting byte order.
    template<class T>
    void parse_array(T * buffer, size_t number_of_elements);
    //! Seeking in the file with either fstream or WIN32 specific code.
    void seek(boost::uint64_t byte_position_in_file);

private:
    // HEADER INFORMATION -----------------------------------------------------

    //! Size of the header in the report file.
    boost::int32_t		header_size;
    //! Version of the reader?
    std::string			library_version;
    //! Version of the simulator used to run the simulation.
    std::string			simulator_version;
    //! Number of cells reported from.
    boost::int32_t	    number_of_cells;
    //! Number of compartments in a frame (for all cells)
    boost::int32_t		number_of_compartments;
    //! Number of frames in the report.
    boost::int32_t		number_of_frames;
    //! Start time of the report in specified time unit.
    double				start_time;
    //! End time of the report in specified time unit.
    /*!
     \todo Clarify if this includes or excludes the end time.
     */
    double				end_time;
    //! Timestep (dt) between two report frames in specified time unit.
    double				timestep;
    //! Unit of the reported variable, e.g. "mV".
    std::string			report_data_unit;
    //! Time unit of the simulation, e.g. "ms".
    std::string			time_unit;
    //! size of mapping block in header
    boost::int32_t		mapping_size;
    //! e.g. "compartment mapping"
    std::string			mapping_name;
    //! The extra mapping is currently not used.
    boost::int32_t		extra_mapping_size;
    //! The extra mapping is currently not used.
    std::string			extra_mapping_name;
    //! e.g. "soma" or "allCompartments"
    std::string			report_name;

    // READER -----------------------------------------------------------------

    Frame_Number		_current_frame;
    Cell_Target			original_target;
    Cell_Target			subtarget;
    //! If the file is opened, header parsed and ready to read frames.
    bool				initialized;
    bool                subtarget_requested;
    //! If the file has been written from architecture with same byte order.
    bool				native_byte_order;
    Compartment_Report_Mapping  original_mapping,
                                conversion_mapping,
                                subtarget_mapping;
#ifdef WIN32
    HANDLE                      report_file;
    boost::uint64_t             cellinfo_block_offset,
                                mapping_block_offset,
                                extra_mapping_block_offset,
                                data_block_offset;
    BOOL                        result;
#else
    std::ifstream				report_file;
    std::ifstream::off_type		cellinfo_block_offset,
                                mapping_block_offset,
                                extra_mapping_block_offset,
                                data_block_offset;
#endif
    struct Section_Mapping
    {
        Report_Frame_Index	frame_index;
        Compartment_Count	number_of_compartments;
    };
};


// ----------------------------------------------------------------------------

inline bool sort_by_gid(const Cell_Info & i, const Cell_Info & j)
{
    return i.gid < j.gid;
}

// ----------------------------------------------------------------------------

inline Compartment_Report_Binary_File_Parser::
Compartment_Report_Binary_File_Parser()
: _current_frame(0), initialized(false), subtarget_requested(false)
#ifdef WIN32
  , report_file(INVALID_HANDLE_VALUE), result(true)
#endif
{
}

// ----------------------------------------------------------------------------

inline Compartment_Report_Binary_File_Parser::
Compartment_Report_Binary_File_Parser(Filepath report_file)
: _current_frame(0), initialized(false), subtarget_requested(false)
#ifdef WIN32
  , report_file(INVALID_HANDLE_VALUE), result(true)
#endif
{
    open(report_file);
}

// ----------------------------------------------------------------------------

inline Compartment_Report_Binary_File_Parser::
Compartment_Report_Binary_File_Parser(Filepath report_file,
                                      const Cell_Target & cells)
: _current_frame(0), initialized(false), subtarget_requested(false)
#ifdef WIN32
  , report_file(INVALID_HANDLE_VALUE), result(true)
#endif
{
    open(report_file);
    cell_target(cells);
}

// ----------------------------------------------------------------------------

inline bool Compartment_Report_Binary_File_Parser::open(Filepath report)
{
#ifdef WIN32

#ifdef UNICODE
    int wide_string_size = 
        MultiByteToWideChar(CP_ACP, 0, report.string().c_str(), -1, 0, 0);
    LPTSTR temp;
    temp = new TCHAR[wide_string_size];
    if (MultiByteToWideChar(CP_ACP, 0, report.string().c_str(), -1, temp,
        wide_string_size) == 0)
    {
        throw_exception(std::runtime_error("Could not convert ASCII string to"
            " unicode string."), SEVERE_LEVEL, __FILE__, __LINE__);
    }
    report_file = CreateFile(temp, 
                             FILE_READ_DATA, FILE_SHARE_READ, 
                             0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    delete [] (temp);
#else
    report_file = CreateFile(report.string().c_str(), 
                             FILE_READ_DATA, FILE_SHARE_READ, 
                             0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
#endif


    // if file does not exist, throw exception
    if (report_file == INVALID_HANDLE_VALUE)
    {
        std::stringstream msg;
        msg << "Compartment_Report_Binary_File_Parser::open: Error, cannot "
            "open file '" << report.string() << "' for reading";
        std::cerr << msg.str().c_str() << std::endl;
        throw_exception(File_Open_Error(msg.str().c_str()), WARNING_LEVEL,
            __FILE__, __LINE__);
    }
#else
    report_file.open(report.string().c_str(), std::ios::binary);
    if (report_file.is_open() == false)
    {
        std::stringstream msg;
        //! \todo refine exception
        throw_exception(File_Open_Error(msg.str().c_str()), WARNING_LEVEL,
            __FILE__, __LINE__);
    }
#endif
    parse_header();
    return true;
}

// ----------------------------------------------------------------------------

inline bool Compartment_Report_Binary_File_Parser::
load_next_frame(Simulation_Value * buffer)
{
    /*! 
        \todo Review the return values and their use since I throw exceptions
        instead of returning false. (TT)
    */
    if (initialized)
    {
#ifdef WIN32
        if (result == false)
        {
            throw_exception(std::runtime_error("Compartment_Report_Binary"
                "_File_Parser::load_next_frame(Simulation_Value*): File "
                "reading error on Windows."), 
                SEVERE_LEVEL, __FILE__, __LINE__);
            return false;
        }
#else
        if (!report_file.good())
        {
            if (report_file.eof())
            {
                throw_exception(std::runtime_error("Compartment_Report_Binary"
                    "_File_Parser::load_next_frame(Simulation_Value*):Tried "
                    "to read beyond end of file"), 
                SEVERE_LEVEL, __FILE__, __LINE__);
            }
            throw_exception(std::runtime_error("Compartment_Report_Binary_"
                "File_Parser::load_next_frame(Simulation_Value*): File "
                "reading error."), 
                SEVERE_LEVEL, __FILE__, __LINE__);
            return false;
        }
#endif
        ++_current_frame;
        // intermediate store for full frame even when reading subtargets
        // for performance reasons (reducing number of seeks).

        // check whether the user is interested in the full frame or a subset
        if (subtarget_requested)
        {
            Simulation_Value * temp_buffer
                = new Simulation_Value[number_of_compartments];
            parse_array(temp_buffer, number_of_compartments);
            Cell_Index cell_index = 0;
            Report_Frame_Index report_frame_index = 0;
            for(Cell_Target::const_iterator i = subtarget.begin();
                i != subtarget.end(); ++i)
            {
                Report_Frame_Index temp_sec = 
                    conversion_mapping.number_of_sections(cell_index);
                for(Section_ID j = 0; j < temp_sec; ++j)
                {
                    /*! 
                        \todo This code is quite performance critical.
                        While it's easier to read this way I think is preferable
                        to index the cell only once for all its sections and
                        store a reference to the array of offsets and
                        compartment counts outside this loop
                    */
                    if (conversion_mapping.number_of_compartments(cell_index, j)
                        != 0)
                    {
                        Compartment_Count temp = conversion_mapping.
                             number_of_compartments(cell_index, j);
                        for (Compartment_Count k = 0; k < temp; ++k)
                        {
                            buffer[report_frame_index] = 
                                temp_buffer[conversion_mapping.
                                section_offset(cell_index, j) + k];
                            ++report_frame_index;
                        }
                    }
                }
                ++cell_index;
            }
            delete [] temp_buffer;
            return true;
        }
        else
        {
            parse_array(buffer, number_of_compartments);
            return true;
        }
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

inline void Compartment_Report_Binary_File_Parser::parse_header()
{
#ifdef BBP_DEBUG
    Timer timer;
    Timer timer2;
    timer.start("parse_header()");
    timer2.start("parse_header() specs");
#endif

    // PARSE SPECS __________________________________________________________
    char buffer[HEADER_LENGTH + 1];
#ifdef WIN32
	DWORD number_of_bytes_read = 0;
    result = result && ReadFile(report_file, (char*) buffer,
                                HEADER_LENGTH, & number_of_bytes_read, 0);
#else
    //! \todo error checks (TT)
    report_file.read(buffer, HEADER_LENGTH);
#endif
    buffer[HEADER_LENGTH] = 0; // add c string terminator to end of array

    // PARSE HEADER
    double read_identifier = parse_var<double>(buffer, IDENTIFIER_POSITION);
    header_size = parse_var<boost::int32_t>(buffer, HEADER_SIZE_POSITION);
    number_of_cells = parse_var<boost::int32_t>(buffer,
                      TOTAL_NUMBER_OF_CELLS_POSITION);
    number_of_compartments = parse_var<boost::int32_t>(buffer,
                      TOTAL_NUMBER_OF_COMPARTMENTS_POSITION);
    library_version = parse_string(buffer, LIBRARY_VERSION_POSITION);
    simulator_version = parse_string(buffer, SIMULATOR_VERSION_POSITION);
    number_of_frames =
        parse_var<boost::int32_t>(buffer, NUMBER_OF_STEPS_POSITION);
    start_time = parse_var<double>(buffer, TIME_START_POSITION);
    end_time = parse_var<double>(buffer, TIME_END_POSITION);
    timestep = parse_var<double>(buffer, DT_TIME_POSITION);
    report_data_unit = parse_string(buffer, D_UNIT_POSITION);
    time_unit = parse_string(buffer, T_UNIT_POSITION);
    mapping_size = parse_var<boost::int32_t>(buffer, MAPPING_SIZE_POSITION);
    mapping_name = parse_string(buffer, MAPPING_NAME_POSITION);
    extra_mapping_size =
        parse_var<boost::int32_t>(buffer, EXTRA_MAPPING_SIZE_POSITION);
    extra_mapping_name = parse_string(buffer, EXTRA_MAPPING_NAME_POSITION);
    report_name = parse_string(buffer, REPORT_NAME_POSITION);

    native_byte_order = read_identifier == ARCHITECTURE_IDENTIFIER;

    // SWAP BYTE ORDER IF NECESSARY
    if (!native_byte_order)
    {
        //check if data can be converted:
        endian_swap(read_identifier);
        if(read_identifier != ARCHITECTURE_IDENTIFIER)
        {
            //! \todo Should throw an exception instead. (TT)
            std::cerr << "File is corrupt or originated from an unknown "
            "architecture." << std::endl;
                exit(1);
        }

        //if data was stored on a machine with different architecture,
        //then the data needs to be converted to be compatible with the
        //reading machine
        endian_swap(header_size);
        endian_swap(number_of_cells);
        endian_swap(number_of_compartments);
        endian_swap(number_of_frames);
        endian_swap(start_time);
        endian_swap(end_time);
        endian_swap(timestep);
        endian_swap(mapping_size);
        endian_swap(extra_mapping_size);
    }
#ifdef WIN32
    LARGE_INTEGER cfp; 
    LARGE_INTEGER zero;
    zero.QuadPart = 0;
    SetFilePointerEx(report_file, zero, &cfp, FILE_CURRENT);
    cellinfo_block_offset = (boost::uint64_t) cfp.QuadPart;
#else
    cellinfo_block_offset = report_file.tellg();
#endif
#ifdef BBP_DEBUG
    timer2.print();
    timer2.start("parse_header() mapping block");
#endif
    // PARSE MAPPING (FIRST PARSE CELLINFO HEADER BLOCK) ____________________

    std::vector<Cell_Info> cells;

    size_t t = SIZE_CELL_INFO_LENGTH * number_of_cells;
    char * cellinfo_buffer = new char[t];
#ifdef WIN32
    seek(cellinfo_block_offset);
    number_of_bytes_read = 0;
	result = ReadFile(report_file, (char*) cellinfo_buffer, SIZE_CELL_INFO_LENGTH * 
        number_of_cells, & number_of_bytes_read, 0);
#else
    // seek to beginning of cellinfo block in the file
    report_file.seekg(cellinfo_block_offset);
    // read cellinfo block
    report_file.read(cellinfo_buffer, SIZE_CELL_INFO_LENGTH * number_of_cells);
#endif
    // Parse partial cellinfo data for first cell that corresponds
    // to the beginning of the data, mapping and extramapping blocks
    // in the file.
    mapping_block_offset = parse_var<boost::uint64_t>(cellinfo_buffer,
                                                      MAPPING_INFO_POSITION);

    extra_mapping_block_offset = parse_var<boost::uint64_t>(cellinfo_buffer,
                                                EXTRA_MAPPING_INFO_POSITION);

    data_block_offset = parse_var<boost::uint64_t>(cellinfo_buffer,
                                                   DATA_INFO_POSITION);

    if (!native_byte_order)
    {
        endian_swap(data_block_offset);
        endian_swap(extra_mapping_block_offset);
        endian_swap(mapping_block_offset);
    }

    Cell_Info cell;
    cells.reserve(number_of_cells);

    for (boost::int32_t i = 0; i < number_of_cells; ++i)
    {
        cell.gid = parse_var<boost::int32_t>(
        cellinfo_buffer, NUMBER_OF_CELL_POSITION + i * SIZE_CELL_INFO_LENGTH);
        cell.number_of_compartments = parse_var<boost::int32_t>(
        cellinfo_buffer, NUMBER_OF_COMPARTMENTS_POSITION +
        i * SIZE_CELL_INFO_LENGTH);
        cell.mapping_offset = parse_var<boost::uint64_t>(
        cellinfo_buffer, MAPPING_INFO_POSITION + i * SIZE_CELL_INFO_LENGTH);
        cell.extra_mapping_offset = parse_var<boost::uint64_t>(
        cellinfo_buffer, EXTRA_MAPPING_INFO_POSITION + i * SIZE_CELL_INFO_LENGTH);
        cell.data_offset = parse_var<boost::uint64_t>(
        cellinfo_buffer, DATA_INFO_POSITION + i * SIZE_CELL_INFO_LENGTH);

        if (!native_byte_order)
        {
            endian_swap(cell.gid);
            endian_swap(cell.number_of_compartments);
            endian_swap(cell.data_offset);
            endian_swap(cell.extra_mapping_offset);
            endian_swap(cell.mapping_offset);
        }
        cells.push_back(cell);
    }

#ifdef BBP_DEBUG
    timer2.print();
    timer2.start("parse_header() section mapping");
#endif

    // PARSE SECTION MAPPING ("CELL MAPPING" OR "COMPARTMENT MAPPING" BLOCK)

    Mapping_Item * mapping_buffer =
        new Mapping_Item[mapping_size * number_of_compartments];
    Compartment_Report_Mapping::Offset_Mapping		offset_mapping;
    Compartment_Report_Mapping::Compartment_Counts	compartment_counts;

    // read mapping block.
#ifdef WIN32
    seek(mapping_block_offset);
#else
    report_file.seekg(mapping_block_offset);
#endif
    parse_array(mapping_buffer, mapping_size * number_of_compartments);

    std::sort(cells.begin(), cells.end(), sort_by_gid);

    /*!
        According to Garik Suess, all compartments of a cell in a frame are
        next to each other, and all compartments of a section are next
        to each other, however, the sections are not necessarily sorted
        by their index in the frame. so it could be that for cell with
        GID 50 while all data is contiguous, the sections are out of order
        so compartments for section 3 6 22 8 could be next to each other,
        while the compartments inside these sections are in order.
    */
    for(std::vector<Cell_Info>::iterator i = cells.begin();
        i != cells.end(); ++i)
    {
        Section_ID current_section  = UNDEFINED_SECTION_ID,
                   previous_section;
        Compartment_Count section_compartment_count = 0;

        std::map<Section_ID, Section_Mapping> sections_mapping;

        // go through all compartments of that cell and read out
        // the sections they belong to
        for(size_t j = 0; j < (size_t) i->number_of_compartments; ++j)
        {
            previous_section = current_section;
            boost::uint64_t pos = i->mapping_offset - mapping_block_offset
            + (j * sizeof(Mapping_Item) * mapping_size);
            current_section = (Section_ID)
                parse_var<Mapping_Item>((char*)mapping_buffer, (size_t) pos);

            // in case this is the start of a new section
            if (current_section != previous_section)
            {
                Section_Mapping section_mapping;
                section_mapping.frame_index = j + ((i->data_offset -
                    data_block_offset) / sizeof(Simulation_Value));
                section_mapping.number_of_compartments =
                    0;
                sections_mapping[current_section] = section_mapping;
                if (previous_section != UNDEFINED_SECTION_ID)
                {
                    sections_mapping[previous_section].number_of_compartments =
                        section_compartment_count;
                }
                section_compartment_count = 0;
            }
            ++section_compartment_count;
        }
        sections_mapping[current_section].number_of_compartments
            = section_compartment_count;


        // now convert the maps into the desired mapping format
        std::vector<Report_Frame_Index> section_offsets;
        std::vector<Compartment_Count>  section_number_of_compartments;

        // get maximum section id
        Section_ID max_id = sections_mapping.rbegin()->first;
        section_offsets.resize(max_id + 1, 0);
        section_number_of_compartments.resize(max_id + 1, 0);

        for(std::map<Section_ID, Section_Mapping>::const_iterator k =
            sections_mapping.begin(); k != sections_mapping.end(); ++k)
        {
            section_offsets[k->first] = k->second.frame_index;
            section_number_of_compartments[k->first] =
                k->second.number_of_compartments;
        }
        original_target.insert(i->gid);
        offset_mapping.push_back(section_offsets);
        compartment_counts.push_back(section_number_of_compartments);
    }

    // create compartment mapping
    original_mapping.swap_mapping(offset_mapping,
                                  compartment_counts,
                                  number_of_compartments);
#ifdef BBP_DEBUG
    timer2.print();
#endif
    initialized = true;

    // update interest target
    if (subtarget.size() == 0)
    {
        cell_target(original_target);
    }
    else
    {
        cell_target(subtarget);
    }

#ifdef WIN32
    seek(data_block_offset);
#else
    report_file.seekg(data_block_offset);
#endif
    delete [] mapping_buffer;
    delete [] cellinfo_buffer;
#ifdef BBP_DEBUG
    timer.print();
#endif
}

// ----------------------------------------------------------------------------

/*!
    \todo If the cell_target function should turn out to be a bottleneck, 
    then the Compartment_Report_Mapping class should be opened to allow
    direct copies of tables/arrays stored inside to save time. Also
    the temporary map could be cached for excessive use of cell_target(). (TT)
*/
inline void Compartment_Report_Binary_File_Parser::cell_target(
    const Cell_Target & cells)
{
#ifdef BBP_DEBUG
    Timer timer, timer2;
    timer.start("Compartment_Report_Binary_File_Parser::cell_target()");
#endif
    subtarget = cells;

    if (initialized)
    {
#ifdef BBP_DEBUG
        timer2.start("Compartment_Report_Binary_File_Parser::cell_target() "
            "- if subtarget != original_target");
#endif
        if (subtarget != original_target)
        {
            subtarget_requested = true;
            if ((subtarget & original_target) != subtarget)
            {
                throw_exception(std::runtime_error("Requested target is "
                    "not a subset of the report target"),
                    SEVERE_LEVEL, __FILE__, __LINE__);
            }
        }
        else
        {
            subtarget_requested = false;
        }
#ifdef BBP_DEBUG
        timer2.print();
#endif
        if (subtarget_requested == true)
        {
#ifdef BBP_DEBUG
            timer2.start("Compartment_Report_Binary_File_Parser::cell_target - "
                "building lookup map table");
#endif
            // build gid to mapping index lookup table
            std::map<Cell_GID, Cell_Index>	gid_to_index;
            Cell_Index c = 0;
            for(Cell_Target::const_iterator i = original_target.begin();
                i != original_target.end(); ++i)
            {
                gid_to_index[*i] = c;
                ++c;
            }
#ifdef BBP_DEBUG
            timer2.print();
            timer2.start("Compartment_Report_Binary_File_Parser::cell_target - "
                "resizing temp mapping tables");
#endif
            Compartment_Report_Mapping::Offset_Mapping		offset_mapping;
            Compartment_Report_Mapping::Compartment_Counts	compartment_counts;

            offset_mapping.resize(subtarget.size());
            compartment_counts.resize(subtarget.size());
#ifdef BBP_DEBUG
            timer2.print();
            timer2.start("Compartment_Report_Binary_File_Parser::cell_target - "
                "building conversion mapping");
#endif
            
            // then build conversion mapping from original to subtarget
            size_t cell = 0;
            Report_Frame_Index total_compartments = 0;
            for (Cell_Target::const_iterator i = subtarget.begin();
                 i != subtarget.end(); ++i)
            {
                Cell_Index index = gid_to_index[*i];

                std::vector<Report_Frame_Index> & sec_offsets 
                    = offset_mapping[cell];
                std::vector<Compartment_Count>  & comp_counts 
                    = compartment_counts[cell];

                Report_Frame_Index num_secs 
                    = original_mapping.number_of_sections(index);
                sec_offsets.resize((unsigned int) num_secs);
                comp_counts.resize((unsigned int) num_secs);

                Compartment_Count comp_count_temp;
                for (Section_ID sid = 0; sid < num_secs; ++sid)
                {
                    comp_count_temp = 
                        original_mapping.number_of_compartments(index, sid);
                    sec_offsets[sid] =
                        original_mapping.section_offset(index, sid);
                    comp_counts[sid] = comp_count_temp;
                    total_compartments += comp_count_temp;
                }
                ++cell;
            }
#ifdef BBP_DEBUG
            timer2.print();
            timer2.start("Compartment_Report_Binary_File_Parser::cell_target - "
                "copying conversion mapping offset and compartment count "
                "tables to prepare subtarget mapping");
#endif
            Compartment_Report_Mapping::Offset_Mapping
                new_offset_mapping = offset_mapping;

            Compartment_Report_Mapping::Compartment_Counts
                temp = compartment_counts;

#ifdef BBP_DEBUG
            timer2.print();
            timer2.start("Compartment_Report_Binary_File_Parser::cell_target - "
                "linking conversion mapping tables");
#endif
            conversion_mapping.swap_mapping(offset_mapping,
                                           temp,
                                           total_compartments);

#ifdef BBP_DEBUG
            timer2.print();
            timer2.start("Compartment_Report_Binary_File_Parser::cell_target - "
                "building subtarget mapping");
#endif
            Cell_Index cell_index = 0;
            Report_Frame_Index rfi = 0;
            for(Cell_Target::const_iterator i = subtarget.begin();
                i != subtarget.end(); ++i)
            {
                for(Section_ID j = 0;
                    j < conversion_mapping.number_of_sections(cell_index);
                    ++j)
                {
                    if (conversion_mapping.
                        number_of_compartments(cell_index, j) != 0)
                    {
                        new_offset_mapping[cell_index][j] = rfi;
                        rfi += conversion_mapping.
                            number_of_compartments(cell_index, j);
                    }
                }
                ++cell_index;
            }

            subtarget_mapping.swap_mapping(new_offset_mapping,
                                           compartment_counts,
                                           total_compartments);
#ifdef BBP_DEBUG
            timer2.print();
#endif
        }
    }
    else
    {
#ifndef NDEBUG
        std::cerr << "Compartment_Report_File_Parser::cell_target(target) "
            "failed: Parser not initialized" << std::endl;
#endif
    }
#ifdef BBP_DEBUG
    timer.print();
#endif
}

// ----------------------------------------------------------------------------

inline void Compartment_Report_Binary_File_Parser::close()
{
#ifdef WIN32
    CloseHandle(report_file);
    report_file = INVALID_HANDLE_VALUE;
    result = true;
#else
    report_file.close();
#endif
    initialized = false;
    subtarget_requested = false;
    subtarget.clear();
    _current_frame = 0;
}

// ----------------------------------------------------------------------------

inline void Compartment_Report_Binary_File_Parser::
jump_to_frame(Frame_Number frame_number_to_seek)
{
    //! \todo boundary check
    _current_frame = frame_number_to_seek;
    seek(data_block_offset + number_of_compartments 
        * sizeof(Simulation_Value) * frame_number_to_seek);
}

// ----------------------------------------------------------------------------

inline Frame_Number Compartment_Report_Binary_File_Parser::
current_framestamp() const
{
    return _current_frame;
}

// ----------------------------------------------------------------------------

inline size_t Compartment_Report_Binary_File_Parser::frame_size() const
{
    if (subtarget_requested == true)
        return (size_t) subtarget_mapping.number_of_compartments();
    else
        return (size_t) original_mapping.number_of_compartments();
}

// ----------------------------------------------------------------------------

inline const Compartment_Report_Mapping & 
Compartment_Report_Binary_File_Parser::mapping() const
{
    if (subtarget_requested == true)
    {
        return subtarget_mapping;
    }
    else
    {
        return original_mapping;
    }
}

// ----------------------------------------------------------------------------

inline const Cell_Target & 
Compartment_Report_Binary_File_Parser::cell_target() const
{
    if (subtarget_requested == true)
        return subtarget;
    else
        return original_target;
}

// ----------------------------------------------------------------------------

template <typename T>
inline T Compartment_Report_Binary_File_Parser::
parse_var(char * buffer, size_t position)
{
    return *((T *) (buffer + position));
}

// ----------------------------------------------------------------------------

inline std::string Compartment_Report_Binary_File_Parser::
parse_string(char * buffer, size_t position)
{
    return std::string((char *) (buffer + position));
}

// ----------------------------------------------------------------------------

template<class T>
inline void Compartment_Report_Binary_File_Parser::
parse_array(T * buffer, size_t number_of_elements)
{
    //! \todo error checks (TT)
#ifdef WIN32
	DWORD number_of_bytes_read = 0;

	result = ReadFile(report_file, (char*)buffer, sizeof(T) * number_of_elements, & number_of_bytes_read, 0);

    if (result == false)
    {
        throw_exception(std::runtime_error("Compartment_Report_Binary_File_Parser::parse_array(T*, size_t): File reading error on Windows."), 
                SEVERE_LEVEL, __FILE__, __LINE__);
    }    
#else
    report_file.read((char *) buffer, number_of_elements * sizeof(T));
    if (!report_file.good())
    {
         if (report_file.eof())
                            throw_exception(std::runtime_error(
                            "Tried to read beyond end of file"), 
                            SEVERE_LEVEL, __FILE__, __LINE__);
         else
             throw_exception(std::runtime_error("Compartment_Report_Binary"
                "_File_Parser::parse_array(T*, size_t): File reading error."),
                SEVERE_LEVEL, __FILE__, __LINE__);
    }
#endif
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif
    if (sizeof(T) != 1 && !native_byte_order)
    {
#ifdef WIN32
#pragma warning( pop )
#endif
        for(size_t i = 0; i < number_of_elements; ++i)
        {
            endian_swap(buffer[i]);
        }
    }
}

// ----------------------------------------------------------------------------

inline void Compartment_Report_Binary_File_Parser::
seek(boost::uint64_t byte_position_in_file)
{
    //! \todo exception handling (TT)
#ifdef WIN32
	LARGE_INTEGER offset;
	offset.QuadPart = (LONGLONG) byte_position_in_file;
	DWORD dwPos = SetFilePointer(report_file, 
        offset.LowPart, & offset.HighPart, FILE_BEGIN);

	if (dwPos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		offset.QuadPart = -1;
        throw_exception(std::runtime_error("Seeking error in report file."), 
                SEVERE_LEVEL, __FILE__, __LINE__);
	}
#else
    report_file.clear();
    report_file.seekg(byte_position_in_file);
    if (report_file.eof())
    {
        throw_exception(std::runtime_error("Seek beyond end of file."),
                        SEVERE_LEVEL, __FILE__, __LINE__);
    }
#endif
}

// ----------------------------------------------------------------------------

}
#endif
