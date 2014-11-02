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

#ifndef BBP_IMAGE_TIFF_FILE_STREAM_WRITER_H
#define BBP_IMAGE_TIFF_FILE_STREAM_WRITER_H

#ifdef BBP_USE_TIFF

#include <tiffio.h>  // libtiff library (Sam Leffler)

#include "BBP/Common/Types.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/Graphics/2D/Bitmap_Image.h"
#include "BBP/Common/Graphics/Color_RGBA.h"

namespace bbp {

//--------------------------------------------------------------------------

//! Stream writer that writes a stream of images as TIFF files to disk.
/*! 
    \bug Potential bug, check if Color_RGBA is ok. (TT)
*/
class Image_TIFF_File_Stream_Writer
	: public Filter <Bitmap_Image <Color_RGBA>, void>
{
public:
	Filename	filename_prefix;
	Count		series_counter;

	inline Image_TIFF_File_Stream_Writer();
	inline ~Image_TIFF_File_Stream_Writer();

	//! start TIFF image writer stream filter
	inline void start();
	//! write TIFF image to disk
	inline void process();
};


//---------------------------------------------------------------------------

Image_TIFF_File_Stream_Writer::Image_TIFF_File_Stream_Writer()
: series_counter(0)
{
}

//---------------------------------------------------------------------------

Image_TIFF_File_Stream_Writer::~Image_TIFF_File_Stream_Writer()
{
}

//---------------------------------------------------------------------------

void Image_TIFF_File_Stream_Writer::start()
{
	if (filename_prefix.empty() == true)
	{
		filename_prefix = "image";
	}
	this->stream_state = STREAM_STARTED;
}

//---------------------------------------------------------------------------

void Image_TIFF_File_Stream_Writer::process()
{
	Bitmap_Image <Color_RGBA> & image = * input;
	Filename filename(filename_prefix + boost::lexical_cast<std::string>
						(series_counter) + ".tiff");
	TIFF * file;
	unsigned char * cursor;
	std::vector<unsigned char> buffer;
	buffer.resize(image.width() * image.height() * 3);

	Index i = 0;

	// convert image to tiff file frame buffer
	for (Index y = 0; y < image.height(); ++y)
	for (Index x = 0; x < image.width(); ++x)
	{
		buffer[i] = image(x,y).red;
		buffer[i + 1] = image(x,y).green;
		buffer[i + 2] = image(x,y).blue;
		i += 3;
	}

	file = TIFFOpen(filename.string().c_str(), "w");
	
	if (file) 
	{
		buffer.resize(image.width() * image.height() * 3);
		TIFFSetField(file, TIFFTAG_IMAGEWIDTH, (uint32) image.width());
		TIFFSetField(file, TIFFTAG_IMAGELENGTH, (uint32) image.height());
		TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
		TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
		TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
		TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, "");

		cursor = & buffer[0];
		for (int y = image.height() - 1; y >= 0; y--) 
		{
			if (TIFFWriteScanline(file, cursor, y, 0) < 0) 
			{
				TIFFClose(file);
				return;
			}
			cursor += image.width() * sizeof(unsigned char) * 3;
		}
		TIFFClose(file);
		std::clog	<< "TIFF Bitmap_Image " << filename << " " 
					<< image.width() << "x" << image.height() 
					<< " written." << std::endl;
	}
	else
	{
		std::cerr << "Could not write TIFF image: " << filename << std::endl;
	}
	++series_counter;
}

//---------------------------------------------------------------------------

}
#endif // BBP_USE_TIFF
#endif

