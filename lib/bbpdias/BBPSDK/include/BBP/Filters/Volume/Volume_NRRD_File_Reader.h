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

#ifndef BBP_VOLUME_NRRD_FILE_READER_H
#define BBP_VOLUME_NRRD_FILE_READER_H

#include <stdexcept>
#include "BBP/Common/Types.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Filters/Volume/Volume_Filter.h"
#include "BBP/Common/System/File/File.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Reads a stream of byte volumes from NRRD binary files (Simian compatible).
/*!
    \ingroup Math Geometry Filter_Reader Filter
*/
class Volume_NRRD_File_Reader
    : public Filter <void, Volume <Byte, Micron, Millisecond> >
{
public:
    inline Volume_NRRD_File_Reader();
    inline Volume_NRRD_File_Reader(Filepath filename_prefix);
    inline ~Volume_NRRD_File_Reader();

    //! start up process of the filter
    inline void start();
    //! read the NRRD file
    inline void process();

    void filename_prefix(const Filepath & prefix)
    {
        _filename_prefix = prefix;
    }

private:
    Filepath _filename_prefix;
};

// ----------------------------------------------------------------------------

struct Volume_NRRD_File_Header
{
    std::string                 version,
                                number_of_voxels,
                                voxel_data_type,
                                encoding,
                                endian;
    Count                       dimension;
    std::vector<std::string>    labels;
    std::vector<Count>          sizes, 
                                spacings;
};

// ----------------------------------------------------------------------------

inline std::ostream & operator << 
(std::ostream & lhs, const Volume_NRRD_File_Header & rhs)
{
    lhs << "NRRD file header: " << std::endl
        << "Version: "      << rhs.version << std::endl
        << "Voxels: "       << rhs.number_of_voxels << std::endl
        << "Type: "         << rhs.voxel_data_type << std::endl
        << "Dimension: "    << rhs.dimension << std::endl
        << "encoding: "     << rhs.encoding << std::endl
        << "endian: "       << rhs.endian << std::endl
        << "sizes: ";
    for (Count i = 0; i < rhs.sizes.size(); ++i)
    {
        lhs << rhs.sizes[i] << " ";
    }
    lhs << std::endl
        << "spacings: ";
    for (Count i = 0; i < rhs.spacings.size(); ++i)
    {
        lhs << rhs.spacings[i] << " ";
    }
    lhs << std::endl
        << "labels: ";
    for (Count i = 0; i < rhs.labels.size(); ++i)
    {
        lhs << rhs.labels[i] << " ";
    }
    lhs << std::endl
        << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

Volume_NRRD_File_Reader::Volume_NRRD_File_Reader()
{
}

// ----------------------------------------------------------------------------

Volume_NRRD_File_Reader::Volume_NRRD_File_Reader(Filepath filename_prefix)
    : _filename_prefix(filename_prefix)
{
}

// ----------------------------------------------------------------------------

Volume_NRRD_File_Reader::~Volume_NRRD_File_Reader()
{
}

// ----------------------------------------------------------------------------

void Volume_NRRD_File_Reader::start()
{
    std::clog << "Reading NRRD header from " 
        << _filename_prefix.string() << std::endl;
    std::ifstream file(_filename_prefix.string().c_str(), 
        std::ios::out | std::ios::binary);
    
    // check if file could be opened
    if (file.is_open() == false)
    {
        throw std::runtime_error(
            std::string("Cannot open file for reading: ") 
            + _filename_prefix.string());
    }

    // read header
    Volume_NRRD_File_Header header;
    std::stringstream line_stream;
    std::string line, token;

    file >> header.version 
         >> token >> header.number_of_voxels
         >> token;


    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    header.voxel_data_type = line_stream.str();

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    line_stream >> token >> header.dimension;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    line_stream >> token >> header.encoding;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    line_stream >> token >> header.endian;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    
    line_stream >> token;   // "sizes:"
    while (line_stream.good())
    {
        line_stream >> token;
        header.sizes.push_back(boost::lexical_cast<Count>(token));
    }

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();

    line_stream >> token;
    while (line_stream.good())
    {
        line_stream >> token;   
        header.spacings.push_back(boost::lexical_cast<Count>(token));
    }

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    while (line_stream.good())
    {
        line_stream >> token;
        header.labels.push_back(token);
    }

    if (header.dimension != 4)
    {
        throw std::runtime_error
            ("Volume NRRD File could not be read"
             " (header data dimension not 4)");
    }

    std::cout << header;

    output().resize( header.sizes[1], header.sizes[2], header.sizes[3]);
    file.close();
    Process_Control_Interface::start();
}

// ----------------------------------------------------------------------------

void Volume_NRRD_File_Reader::process()
{
    std::clog << "Reading NRRD binary volume file" << _filename_prefix.string()
        << std::endl;
    std::ifstream file(_filename_prefix.string().c_str(), 
        std::ios::out | std::ios::binary);
    
    // check if file could be opened
    if (file.is_open() == false)
    {
        throw std::runtime_error(
            std::string("Cannot open file for reading: ") 
            + _filename_prefix.string());
    }

    // read header
    Volume_NRRD_File_Header header;
    std::stringstream line_stream;
    std::string line, token;

    file >> header.version 
         >> token >> header.number_of_voxels
         >> token;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    header.voxel_data_type = line_stream.str();

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    line_stream >> token >> header.dimension;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    line_stream >> token >> header.encoding;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    line_stream >> token >> header.endian;

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    
    line_stream >> token;   // "sizes:"
    while (line_stream.good())
    {
        line_stream >> token;
        header.sizes.push_back(boost::lexical_cast<Count>(token));
    }

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();

    line_stream >> token;
    while (line_stream.good())
    {
        line_stream >> token;   
        header.spacings.push_back(boost::lexical_cast<Count>(token));
    }

    std::getline(file, line, '\n');
    line_stream.str(line);
    line_stream.clear();    
    while (line_stream.good())
    {
        line_stream >> token;
        header.labels.push_back(token);
    }

    if (header.dimension != 4)
    {
        throw std::runtime_error(
            "Volume NRRD File could not be "
            "read (data dimension not 4)");
    }

    std::getline(file, line, '\n');

    std::clog << "Reading NRRD data" << std::endl;

    output().resize(header.sizes[1], header.sizes[2], header.sizes[3]);

    char byte;


    Volume<Byte, Micron, Millisecond> & volume = output();

    // read volume data
    for (Count z = 0; z < header.sizes[3]; z++) 
    for (Count y = 0; y < header.sizes[2]; y++)
    for (Count x = 0; x < header.sizes[1]; x++)
    {
        file.get(byte);
        volume(x,y,z) = (Byte) byte;
        //std::cout << Count (volume(x,y,z)) << " ";
    }
    file.close();
}

// ----------------------------------------------------------------------------

}
#endif
