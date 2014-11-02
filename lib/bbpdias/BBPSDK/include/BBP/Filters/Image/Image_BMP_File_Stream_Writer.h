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

#ifndef BBP_IMAGE_BMP_FILE_STREAM_WRITER_H
#define BBP_IMAGE_BMP_FILE_STREAM_WRITER_H

#include "BBP/Common/Graphics/2D/Bitmap_Image.h"
#include "BBP/Common/Graphics/Color_RGBA.h"
#include "BBP/Common/Graphics/2D/Writers/Image_BMP_File_Writer.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/System/File/File.h"
 
namespace bbp {

//---------------------------------------------------------------------------

//! Stream writer that writes a stream of images as BMP files to disk.
/*!
    \ingroup Filter
    \bug Potential bug, check if Color_RGBA is ok (was RGB before). (TT)
*/
class Image_BMP_File_Stream_Writer
	: public Filter <Bitmap_Image <Color_RGBA>, void>
{
public:
	Filename	filename_prefix;
	Count		series_counter;

	inline Image_BMP_File_Stream_Writer();

	//! start BMP image writer stream filter
	inline void start();
	//! write BMP image to disk
	inline void process();

private:
	Image_BMP_File_Writer writer;
};


//---------------------------------------------------------------------------

Image_BMP_File_Stream_Writer::Image_BMP_File_Stream_Writer()
: series_counter(0)
{
}

//---------------------------------------------------------------------------

void Image_BMP_File_Stream_Writer::start()
{
	if (filename_prefix.empty() == true)
	{
		filename_prefix = "image";
	}
    Filter<Bitmap_Image <Color_RGBA>, void>::start();
}

//---------------------------------------------------------------------------

void Image_BMP_File_Stream_Writer::process()
{
	// Filter input image.
	Bitmap_Image <Color_RGBA> & image = input();
	// Filter output filename.
	Filename filename(filename_prefix.string() 
        + boost::lexical_cast<std::string>(series_counter) + ".bmp");

	writer.open(filename);
	writer.write(image);
	writer.close();
	++series_counter;
	std::clog	<< "Bitmap_Image " << filename << " " 
				<< image.width() << "x" << image.height() << " written." 
				<< std::endl;
}

//---------------------------------------------------------------------------

}
#endif

