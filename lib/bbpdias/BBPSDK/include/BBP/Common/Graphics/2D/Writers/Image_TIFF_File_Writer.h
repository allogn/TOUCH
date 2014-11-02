/*

      Ecole Polytechnique Federale de Lausanne
      Brain Mind Institute,
      Blue Brain Project & Thomas Traenkler
      (c) 2006-2008. All rights reserved.

      Responsible authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_IMAGE_TIFF_FILE_WRITER_H
#define BBP_IMAGE_TIFF_FILE_WRITER_H

#ifdef BBP_SUPPORT_TIFF
#include <tiffio.h> // libtiff library (Sam Leffler)
#include "BBP/Common/Graphics/2D/Bitmap_Image.h"
#include "BBP/Common/Graphics/Color_RGBA.h"
#include "BBP/Common/Filter/Filter.h"
 
namespace bbp {


//---------------------------------------------------------------------------

//! Write 24 bit RGB image as BMP file to disk.
class Image_TIFF_File_Writer
{
public:
  //! Construct writer.
  inline Image_TIFF_File_Writer();
  //! Construct writer and open a file for writing.
  inline Image_TIFF_File_Writer(Filepath filepath);
  inline ~Image_TIFF_File_Writer();
  //! Open output file.
  inline void open(Filepath filepath);
  //! Close output file.
  inline void close();
  //! Write image to disk.
  /*! 
    \bug Potential bug, check if Color_RGBA is ok (was RGB before). (TT)
  */
  inline void write(const Image<Color_RGBA> & image);

private:
  // Image array buffer.
  std::vector<Byte>   _buffer;
  // Open file.
  TIFF *              _file;

};


//---------------------------------------------------------------------------

Image_TIFF_File_Writer::Image_TIFF_File_Writer()
: _file(0)
{
}

//---------------------------------------------------------------------------

Image_TIFF_File_Writer::Image_TIFF_File_Writer(Filepath filepath)
: _file(0)
{
  open(filepath);
}


//---------------------------------------------------------------------------

Image_TIFF_File_Writer::~Image_TIFF_File_Writer()
{
  if (_file)
      TIFFclose(_file);
}

//---------------------------------------------------------------------------

void Image_TIFF_File_Writer::open(Filepath filepath)
{
  // If TIFF file open close it first.
  if (_file)
  {
      TIFFclose(file);
      _file = 0;
  }

  // Open file.
  _file = TIFFOpen(filepath, "w");
  if (_file == false)
  {
      std::string error = std::string("Bitmap Image TIFF writing failed: "
          "Could not open file ") + filename;
      std::cerr << error << std::endl;
      throw std::runtime_error(error);
  }
}

//---------------------------------------------------------------------------

void Image_TIFF_File_Writer::close()
{
  TIFFclose(_file);
}

//---------------------------------------------------------------------------

void Image_TIFF_File_Writer::write(const Bitmap_Image<Color_RGB> & image)
{
  if (_file)
  {
      bool ret = false;
      GLubyte * image, 
              * p;
      int i;

      // Set TIFF file specification for format and header.
      TIFFSetField(_file, TIFFTAG_IMAGEWIDTH, (uint32) width);
      TIFFSetField(_file, TIFFTAG_IMAGELENGTH, (uint32) height);
      TIFFSetField(_file, TIFFTAG_BITSPERSAMPLE, 8);
      TIFFSetField(_file, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
      TIFFSetField(_file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
      TIFFSetField(_file, TIFFTAG_SAMPLESPERPIXEL, 3);
      TIFFSetField(_file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
      TIFFSetField(_file, TIFFTAG_ROWSPERSTRIP, 1);
      TIFFSetField(_file, TIFFTAG_IMAGEDESCRIPTION, "");
      p = image;

      // Write image to TIFF file.
      for (i = height - 1; i >= 0; i--)
      {
          if (TIFFWriteScanline(file, p, i, 0) < 0)
          {
              free(image);
              TIFFClose(file);
              return false;
          }
          p += width * sizeof(GLubyte) * 3;
      }
  }

}

//---------------------------------------------------------------------------

}
#endif //BBP_SUPPORT_TIFF
#endif //include guard


