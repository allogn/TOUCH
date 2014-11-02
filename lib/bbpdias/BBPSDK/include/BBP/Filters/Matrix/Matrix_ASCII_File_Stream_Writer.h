/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.
    
        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MATRIX_ASCII_FILE_STREAM_WRITER_H
#define BBP_MATRIX_ASCII_FILE_STREAM_WRITER_H

#include "BBP/Common/Types.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/Math/Matrix.h"
#include "BBP/Common/System/File/File.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Writes a stream of matrices to ASCII files.
/*!
    \ingroup Filter Writer Math
*/
template <typename T>
class Matrix_ASCII_File_Stream_Writer
    : public Filter <Matrix <T>, void>
{
public:
    Filename        filename_prefix;

    Matrix_ASCII_File_Stream_Writer();
    ~Matrix_ASCII_File_Stream_Writer();

    //! Open _file for writing.
    inline void open(Filename filename);
    //! Close _file.
    inline void close();

    //! Start filter stream processing (opening _file).
    inline void start();
    //! Process step writing the current matrix input to a _file.
    inline void process();

private:
    std::ofstream   _file;
    Count           _series_file_counter;
};

// ----------------------------------------------------------------------------

template <typename T>
Matrix_ASCII_File_Stream_Writer<T>::Matrix_ASCII_File_Stream_Writer()
    : _series_file_counter(0)
{
}

// ----------------------------------------------------------------------------

template <typename T>
Matrix_ASCII_File_Stream_Writer<T>::~Matrix_ASCII_File_Stream_Writer()
{
    if (_file.is_open() == true)
    {
        _file.close();
    }
}

// ----------------------------------------------------------------------------

template <typename T>
void Matrix_ASCII_File_Stream_Writer<T>::process()
{
    Matrix <T> & in = this->input();

    std::clog << "Writing Matrix ASCII _file" << std::endl;
    Filename filename (filename_prefix.string() + "_sequence_" + 
        boost::lexical_cast<std::string>(_series_file_counter) + ".txt");

    open(filename);

    for (Index y = 0; y < in.rows(); ++y)
    {
        for (Index x = 0; x < in.columns(); ++x)
        {
            _file << in(x, y) << " ";
        }
        _file << std::endl;
    }
    close();
    ++_series_file_counter;
}

// ----------------------------------------------------------------------------

template <typename T>
void Matrix_ASCII_File_Stream_Writer<T>::start()
{
    if (filename_prefix.empty() == true)
    {
        filename_prefix = "matrix";
    }
    Filter <Matrix <T>, void>::start();
}

// ----------------------------------------------------------------------------

template <typename T>
void Matrix_ASCII_File_Stream_Writer<T>::open(Filename filename)
{
    _file.open(filename.string().c_str(), 
        std::ios::out | std::ios::binary);

    if (_file.is_open() == false)
    {
        throw std::runtime_error(
            std::string("Could not open _file for writing matrix "
            "ASCII _file ") + filename.string());
    }
}

// ----------------------------------------------------------------------------

template <typename T>
void Matrix_ASCII_File_Stream_Writer<T>::close()
{
    _file.close();
}

// ----------------------------------------------------------------------------

}
#endif
