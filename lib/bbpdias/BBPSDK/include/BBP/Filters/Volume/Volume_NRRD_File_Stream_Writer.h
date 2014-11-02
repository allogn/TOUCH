/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Nikolai Chapochnikov
                                Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VOLUME_NRRD_FILE_STREAM_WRITER_H
#define BBP_VOLUME_NRRD_FILE_STREAM_WRITER_H

#include "BBP/Common/Types.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Filters/Volume/Volume_Filter.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"


namespace bbp {

//---------------------------------------------------------------------------

//! Writes stream of Byte volumes to NRRD volume binary files 
//! (e.g. for Simian).
/*!
    \ingroup Math Geometry Filter_Writer 
*/
class Volume_NRRD_File_Stream_Writer
    : public Filter <Volume <Byte, Micron, Millisecond>, void>
{

public:

    Filename filename_prefix;

    inline Volume_NRRD_File_Stream_Writer(Filename filename_prefix);
    
    // Added because problem to wrapp the boost::filesystem::basic_path - SL - 2009-08-12
    explicit inline Volume_NRRD_File_Stream_Writer(std::string filename_prefix);

    inline ~Volume_NRRD_File_Stream_Writer();

    //! Compute the volume voltage average and write it to output.
    inline void process();

private:
    Count series_file_counter;
};


//---------------------------------------------------------------------------

Volume_NRRD_File_Stream_Writer::Volume_NRRD_File_Stream_Writer
    (Filename filename_prefix)
    : filename_prefix(filename_prefix), 
      series_file_counter(0)
{
}

//---------------------------------------------------------------------------

Volume_NRRD_File_Stream_Writer::Volume_NRRD_File_Stream_Writer
    (std::string filename_prefix)
    : filename_prefix(filename_prefix), 
      series_file_counter(0)
{
}

//---------------------------------------------------------------------------

Volume_NRRD_File_Stream_Writer::~Volume_NRRD_File_Stream_Writer()
{
#ifdef BBP_DEBUG
    //! \todo remove this debug output
    std::cout << "Volume_NRRD_File_Stream_Writer::"
        "~Volume_NRRD_File_Stream_Writer()" << std::endl;
#endif
}

//---------------------------------------------------------------------------

void Volume_NRRD_File_Stream_Writer::process()
{
    log_message("Writing NRRD binary volume file...", 
        INFORMATIVE_LEVEL, __FILE__,__LINE__);
    Filename filename (filename_prefix.string() + "_sequence_" + 
        boost::lexical_cast<std::string>(series_file_counter) + ".nrrd");
    std::ofstream file(filename.string().c_str(), 
        std::ios::out | std::ios::binary);
    
    // check if file could be opened
    if (file.is_open() == false)
    {
        std::string error = "Cannot open file for writing: " 
            + filename.string();
        std::cerr << error << std::endl;
        throw std::runtime_error(error);
    }
    
    // write NRRD header
    Count resolution_x = input().resolution().x();  
    Count resolution_y = input().resolution().y();  
    Count resolution_z = input().resolution().z();  
    
    file << "NRRD0001" << std::endl;
    file << "number: " << (resolution_x * resolution_y* resolution_z) 
        << std::endl;
    file << "type: unsigned char" << std::endl;
    file << "dimension: 4" << std::endl;
    file << "encoding: raw" << std::endl;
    file << "endian: little" << std::endl;
    file << "sizes: 1 " << resolution_x << " " 
                        << resolution_y << " " 
                        << resolution_z << std::endl;
    file << "spacings: 1 1 1 1" << std::endl;
    file << "labels: \"d\" \"x\" \"y\" \"z\"" << std::endl 
         << std::endl;
    
    // write volume data
    for (Count z = 0; z < resolution_z; z++)    
    for (Count y = 0; y < resolution_y; y++)
    for (Count x = 0; x < resolution_x; x++)
    {
        file << (input().operator()(x,y,z));
    }

    file.close();
    ++series_file_counter;
    log_message("NRRD binary volume file written.", 
        INFORMATIVE_LEVEL, __FILE__,__LINE__);
}

//---------------------------------------------------------------------------

}
#endif
