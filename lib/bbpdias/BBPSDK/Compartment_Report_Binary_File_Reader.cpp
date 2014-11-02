/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	  Juan Hernando Vieites
*/

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/System/File/File.h"
#include "File/Compartment_Report_Binary_File_Reader.h"
#include "report_specialization_selection.h"

#include <BBP/Common/Exception/Exception.h>

#include <cassert>
#include <boost/cstdint.hpp>
#include <boost/progress.hpp>
#ifdef WIN32
#include "BBP/Common/Math/Round.h"
#endif

//-----------------------------------------------------------------------------

namespace bbp
{
    
//-----------------------------------------------------------------------------

const static class Display_Progress
{
public:
    Display_Progress()
    {
        std::string _;
#ifndef WIN32
        _display = bbp::getenv("BBP_SHOW_PROGRESS", _);
#else
        _display = true;
#endif
    }
    operator bool() const
    {
        return _display;
    }
private:
    bool _display;
} s_display_progress;

// ----------------------------------------------------------------------------

Compartment_Report_Binary_File_Reader::Compartment_Report_Binary_File_Reader
(const Report_Specification & specs)
    : _sequence_start_frame(0),
      _frame_counter(0),
      _frame_skip(1.0f),
      _current_frame(UNDEFINED_FRAME_NUMBER)
{
    static Report_Specialization_Register<
               Compartment_Report_Binary_File_Reader,
               float
           > register_float_buffer;

    
    namespace fs = boost::filesystem;
    
    Filepath path = uri_to_filename(specs.data_source());
    Filepath filename;
    
    enum Binary_Type
    { 
        Multi_Split, 
        Unknown 
    };
    
    Binary_Type binary_type = Unknown;
    
    // Checking which type of file
    if (fs::is_directory(path))
    {
        if (fs::exists((filename = path / (specs.label() + ".rep"))))
        {
            binary_type = Multi_Split;
        } 
        else if (fs::exists((filename = path / (specs.label() + ".bbp"))))
        {
            binary_type = Multi_Split;
        }
    } 
    else if (fs::exists(path))
    {
        filename = path;
        if (fs::extension(path) == ".rep")
        {
            binary_type = Multi_Split;
        }
        else if (fs::extension(path) == ".bbp")
        {
            binary_type = Multi_Split;
        }
    }
        
    if (binary_type == Multi_Split)
    {
        _parser.open(filename.string());
    }
    else
    {
        throw_exception(Bad_Data("Compartment_Binary_File_Report_Reader: "
                                 "no proper binary file for " + 
                                 specs.data_source() + " found"),
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
}

//-----------------------------------------------------------------------------

Compartment_Report_Binary_File_Reader::~Compartment_Report_Binary_File_Reader()
{}

//-----------------------------------------------------------------------------

void Compartment_Report_Binary_File_Reader::jump_to_frame(Frame_Number position)
    /* throw (IO_Error, Unsupported) */
{
	//! \todo Check if this is safe with regard to interest window. (TT)
    if (position == UNDEFINED_FRAME_NUMBER)
        position = 0; // Go to start
    _sequence_start_frame = position;
    _frame_counter = 0;
	_parser.jump_to_frame(position);
    _current_frame = UNDEFINED_FRAME_NUMBER;
}

//-----------------------------------------------------------------------------

void Compartment_Report_Binary_File_Reader::frame_skip(double frame_skip)
    /* throw (IO_Error, Unsupported) */
{
    _frame_skip = frame_skip;
}

//-----------------------------------------------------------------------------

double Compartment_Report_Binary_File_Reader::frame_skip() const
{
    return _frame_skip;
}

//-----------------------------------------------------------------------------

Frame_Number Compartment_Report_Binary_File_Reader::current_framestamp() const
    /* throw (IO_Error, Unsupported) */
{
    return _parser.current_framestamp();
}

//-----------------------------------------------------------------------------

void Compartment_Report_Binary_File_Reader::reset()
    /* throw (IO_Error, Unsupported) */
{
    jump_to_frame(UNDEFINED_FRAME_NUMBER);
}

//-----------------------------------------------------------------------------

Compartment_Report_Mapping_Ptr 
Compartment_Report_Binary_File_Reader::update_mapping
(const Cell_Target & new_cell_target)
{
    Compartment_Report_Mapping_Ptr new_mapping(new Compartment_Report_Mapping);
    _parser.cell_target(new_cell_target); 
    (*new_mapping) = _parser.mapping();
    return new_mapping;
}
    
//-----------------------------------------------------------------------------
        
void Compartment_Report_Binary_File_Reader::update_mapping_and_framesize
(const Cell_Target & target)
{
    _mapping = update_mapping(target);
    _frame_size = _parser.frame_size();
    _current_cell_target = target;
}

//-----------------------------------------------------------------------------

template <>
void Compartment_Report_Binary_File_Reader::load_frames_impl
(float * buffer, Frame_Number first, Frame_Number last)
        /* throw (IO_Error, Unsupported) */
{
	// memorize initial position
    Frame_Number old_stamp = _parser.current_framestamp();
    
	// loading progress bar for loading a bunch of frames
    std::auto_ptr<boost::progress_display> progress;
    if (first != last && s_display_progress)
        progress.reset(new boost::progress_display(last - first + 1));
    
	// read frame by frame until end is reached
	for (_parser.jump_to_frame(first); _parser.current_framestamp()
         <= last;
		 buffer += _parser.frame_size())
	{
		_parser.load_next_frame(buffer);
		// advance progress loading bar
        if (progress.get())
        {
            ++(*progress);
        }
    }
	// return frame parser to old position
	_parser.jump_to_frame(old_stamp);
}

//-----------------------------------------------------------------------------

template <>
bool Compartment_Report_Binary_File_Reader::load_next_frame_impl
(Simulation_Value * buffer) /* throw (IO_Error) */
{
	// calculate next frame
    Frame_Number next_frame =  (Frame_Number)
    round(_sequence_start_frame + _frame_counter * _frame_skip);
    //! \todo Check properly if next frame goes out of bounds.
    try
	{
		if (_frame_skip == 1.0)
		{
			_parser.load_next_frame(buffer);
		}
		else
		{
			_parser.jump_to_frame(next_frame);
			_parser.load_next_frame(buffer);
		}
		_current_frame = next_frame;
		++_frame_counter;
	}
	catch(std::out_of_range &)
	{
		// signal error in return value of function that not all frames
		// could be read.
		return false;
	}
	
	// frame could be read with no errors, signal green light
    return true;
}

// ----------------------------------------------------------------------------
    
} // namespace bbp end
