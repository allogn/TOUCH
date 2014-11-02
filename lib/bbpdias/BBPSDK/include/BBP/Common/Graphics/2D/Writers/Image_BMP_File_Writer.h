/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_IMAGE_BMP_FILE_WRITER_H
#define BBP_IMAGE_BMP_FILE_WRITER_H

#include "BBP/Common/Graphics/2D/Bitmap_Image.h"
#include "BBP/Common/Graphics/Color_RGBA.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/System/File/File.h"
 
namespace bbp {

//---------------------------------------------------------------------------

typedef boost::uint32_t DWORD;
typedef boost::uint16_t WORD;
typedef boost::int32_t  LONG;
struct Bitmap_Image_BMP_File_Header; // tagBITMAPFILEHEADER on Win32
struct Bitmap_Image_BMP_Info_Header; // tagBITMAPINFOHEADER on Win32

//---------------------------------------------------------------------------

#if ! (defined(lint) || defined(RC_INVOKED))
#if ( _MSC_VER >= 800 && !defined(_M_I86)) || defined(_PUSHPOP_SUPPORTED)
#pragma warning(disable:4103)
#if !(defined( MIDL_PASS )) || defined( __midl )
#pragma pack(push,2)
#else
#pragma pack(2)
#endif
#else
#pragma pack(2)
#endif
#endif // ! (defined(lint) || defined(RC_INVOKED))

typedef struct Bitmap_Image_BMP_File_Header
{
    WORD    bfType; 
    DWORD   bfSize; 
    WORD    bfReserved1; 
    WORD    bfReserved2;
    DWORD   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 

#if ! (defined(lint) || defined(RC_INVOKED))
#if ( _MSC_VER >= 800 && !defined(_M_I86)) || defined(_PUSHPOP_SUPPORTED)
#pragma warning(disable:4103)
#if !(defined( MIDL_PASS )) || defined( __midl )
#pragma pack(pop)
#else
#pragma pack()
#endif
#else
#pragma pack()
#endif
#endif // ! (defined(lint) || defined(RC_INVOKED))

//---------------------------------------------------------------------------

typedef struct Bitmap_Image_BMP_Info_Header
{
    DWORD  biSize; 
    LONG   biWidth; 
    LONG   biHeight; 
    WORD   biPlanes; 
    WORD   biBitCount; 
    DWORD  biCompression; 
    DWORD  biSizeImage; 
    LONG   biXPelsPerMeter; 
    LONG   biYPelsPerMeter; 
    DWORD  biClrUsed; 
    DWORD  biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

//---------------------------------------------------------------------------

//! Write 24 bit RGB image as BMP file to disk.
/*!
    \ingroup Graphics
*/
class Image_BMP_File_Writer
{
public:
    //! Construct writer.
    inline Image_BMP_File_Writer();
    //! Construct writer and open a file for writing.
    inline Image_BMP_File_Writer(Filepath filepath);
    inline ~Image_BMP_File_Writer();
    //! Open output file.
    inline void open(Filepath filepath);
    //! Close output file.
    inline void close();
    //! Write image to disk.
    /*! 
        \bug Potential bug, check if Color_RGBA is ok (was RGB before). (TT)
    */
    inline void write(const Bitmap_Image<Color_RGBA> & image);

private:
    // Image array buffer.
    std::vector<Byte>   _buffer;
    // Open file.
    std::ofstream       _file;
    Filepath            _filename;
};

//---------------------------------------------------------------------------

Image_BMP_File_Writer::Image_BMP_File_Writer()
{
}


//---------------------------------------------------------------------------

Image_BMP_File_Writer::~Image_BMP_File_Writer()
{
}

//---------------------------------------------------------------------------

Image_BMP_File_Writer::Image_BMP_File_Writer(Filepath filepath)
{
    open(filepath);
}

//---------------------------------------------------------------------------

void Image_BMP_File_Writer::open(Filepath filepath)
{
    _filename = filepath;
    if (_file.is_open() == true)
    {
        close();
    }

    _file.open(filepath.string().c_str(), std::ios::binary);
    // Test if file can be opened.
    if (_file.is_open() == false)
    {
        std::string error = std::string("Bitmap Image BMP writing failed: "
            "Could not open file ") + filepath.string();
        std::cerr << error << std::endl;
        throw std::runtime_error(error);
    }
}

//---------------------------------------------------------------------------

void Image_BMP_File_Writer::close()
{
    _file.close();
    _filename = "";
}

//---------------------------------------------------------------------------

void Image_BMP_File_Writer::write(const Bitmap_Image<Color_RGBA> & image)
{
    // Convert image to bmp file frame buffer.
    _buffer.resize(image.width() * image.height() * 3);
    Index i = 0;
    for (Index y = 0; y < image.height(); ++y)
    for (Index x = 0; x < image.width(); ++x)
    {
        _buffer[i]      = image(x,y).blue();
        _buffer[i + 1]  = image(x,y).green();
        _buffer[i + 2]  = image(x,y).red();
        i += 3;
    }

    // Write BMP header to file.
    BITMAPFILEHEADER  bmp_file_header;
    BITMAPINFOHEADER  bmp_info_header;
    bmp_file_header.bfType          =   0x4D42;  //"BM"
    bmp_file_header.bfSize          =   image.width() * image.height() * 3 * 
                                        sizeof(Byte) + 
                                        sizeof(BITMAPFILEHEADER) +  
                                        sizeof(BITMAPINFOHEADER);
    bmp_file_header.bfReserved1     =   0;
    bmp_file_header.bfReserved2     =   0;
    bmp_file_header.bfOffBits       =   sizeof(BITMAPFILEHEADER) +  
                                        sizeof(BITMAPINFOHEADER);
    bmp_info_header.biSize          =   sizeof(BITMAPINFOHEADER);
    bmp_info_header.biWidth         =   image.width();
    bmp_info_header.biHeight        =   image.height();
    bmp_info_header.biPlanes        =   1;
    bmp_info_header.biBitCount      =   24;
    bmp_info_header.biCompression   =   0L; //BI_RGB
    bmp_info_header.biSizeImage     =   0;
    bmp_info_header.biXPelsPerMeter =   0;
    bmp_info_header.biYPelsPerMeter =   0;
    bmp_info_header.biClrUsed       =   0;
    bmp_info_header.biClrImportant  =   0;

    // Write image header.
    _file.write((const char *) & bmp_file_header, sizeof(BITMAPFILEHEADER));
    _file.write((const char *) & bmp_info_header, sizeof(BITMAPINFOHEADER));

    Count padding = ( 4 - ( ( 1 * image.width() ) % 4 ) ) % 4;

    // Write image _buffer to file and close.
    for (Index y = 0; y < image.height(); ++y)
    {
        _file.write((const char *) & _buffer[image.width() * y * 3 * sizeof(Byte)], 
            image.width() * 3 * sizeof(Byte));

        for (Index i = 0; i < padding; i++ )
        {
          _file.put(0);
        }
    }
    std::clog   << "Bitmap image " << _filename.string() << " " 
                << image.width() << "x" << image.height() 
                << " written." << std::endl;
}

//---------------------------------------------------------------------------

}
#endif

