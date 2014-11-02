/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.
    
        Responsible author: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VOLUME_ASCII_FILE_STREAM_WRITER_H
#define BBP_VOLUME_ASCII_FILE_STREAM_WRITER_H

#include <iomanip>
#include "BBP/Common/Types.h"
#include "BBP/Common/System/File/File.h"
#include "BBP/Common/Filter/Filter_Writer.h"
#include "BBP/Common/Math/Geometry/Volume.h"


namespace bbp {

// ----------------------------------------------------------------------------

//! Writes a stream of 3D volume states to ASCII files.
/*!
    \ingroup Math Geometry Filter_Writer Filter
*/
template <typename Volume_Data_Type, 
          typename Space_Unit, 
          typename Time_Unit>
class Volume_ASCII_File_Stream_Writer
    : public Filter_Writer <Volume <Volume_Data_Type, Space_Unit, Time_Unit> >
{
public:

    Volume_ASCII_File_Stream_Writer();
    ~Volume_ASCII_File_Stream_Writer();

    //! Write out volume to file.
    inline void process();

    //! write voltage volume to an ASCII file of specified filename
    inline void open(Filename filename_prefix);
    inline void close();

private:
    std::ofstream file;
    Filename filename_prefix;
    Count   frame_number;
};


// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
Volume_ASCII_File_Stream_Writer<Volume_Data_Type, Space_Unit, Time_Unit>::
Volume_ASCII_File_Stream_Writer()
: frame_number(0)
{
}

// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
Volume_ASCII_File_Stream_Writer<Volume_Data_Type, Space_Unit, Time_Unit>::
~Volume_ASCII_File_Stream_Writer()
{
    close();
}

// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
void Volume_ASCII_File_Stream_Writer<Volume_Data_Type, 
    Space_Unit, Time_Unit>::process()
{
    Count maximum_x = this->input().resolution().x();
    Count maximum_y = this->input().resolution().y();
    Count maximum_z = this->input().resolution().z();

    // open file if necessary
    if (file.is_open() == false) 
    {
        if (filename_prefix.empty() == true)
            open("volume_output");
        else
            open(filename_prefix);
    }

    // writer header
    file << "#nb_line " << maximum_x;
    file << "#nb_col "  << maximum_z;
    file << "#height "  << maximum_y;
    file.precision(1);

    // write volume values to file
    for (Count y = 0; y < maximum_y; y++)
    for (Count x = 0; x < maximum_x; x++)
    for (Count z = 0; z < maximum_z; z++)
    {
        file << std::fixed << (this->input()) (x, y, z);
    }
    close();
    ++frame_number;
}

// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
void Volume_ASCII_File_Stream_Writer<Volume_Data_Type, Space_Unit, Time_Unit>::
    open (Filename filename_prefix)
{
    std::ostringstream convert; 
    convert << std::setw(5)<< std::setfill('0') << frame_number; 
    std::string filename = filename_prefix.string() + "_frame_" + 
        convert.str() + ".txt";

    // open file for writing
    file.open(filename.c_str());

    if (file.is_open() == false)
    {
        std::cerr << "Volume ASCII File Writer: Could not open file for "
            "writing " << filename << std::endl;
        throw std::runtime_error("Volume ASCII File Writer: Could not open "
            "file for writing ");
    }
}

// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
void Volume_ASCII_File_Stream_Writer<Volume_Data_Type, 
    Space_Unit, Time_Unit>::close()
{
    file.close();
}

// ----------------------------------------------------------------------------

}
#endif
