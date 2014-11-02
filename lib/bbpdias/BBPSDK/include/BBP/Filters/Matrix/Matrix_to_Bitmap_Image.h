/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		Thomas Traenkler
		(c) 2006-2007. All rights reserved.

		Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MATRIX_TO_BITMAP_IMAGE_H
#define BBP_MATRIX_TO_BITMAP_IMAGE_H

#include "BBP/Common/Graphics/2D/Bitmap_Image.h"
#include "BBP/Common/Graphics/Color_Mapping.h"
#include "BBP/Common/Math/Matrix.h"
#include "BBP/Common/Filter/Filter.h"

namespace bbp {

// ----------------------------------------------------------------------------

/*!
    \ingroup Filter
*/
template <typename Matrix_Data_Type>
class Matrix_to_Bitmap_Image 
	: public Filter <Matrix <Matrix_Data_Type>, 
			         Bitmap_Image <Color_RGBA> >
{

public:

	Matrix_to_Bitmap_Image();
	~Matrix_to_Bitmap_Image();
	
	inline void process();
	inline void dynamic_range(Matrix_Data_Type minimum,
							  Matrix_Data_Type maximum);

private:
	Color_Mapping <Matrix_Data_Type> colour_mapping;
};


// ----------------------------------------------------------------------------

template <typename Matrix_Data_Type>
Matrix_to_Bitmap_Image <Matrix_Data_Type>::Matrix_to_Bitmap_Image(void)
: colour_mapping(-65, -20.0) //!< \todo This should not be hardcoded. (TT)
{
#ifndef NDEBUG
	std::clog << "Initialize matrix to image filter" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

template <typename Matrix_Data_Type>
Matrix_to_Bitmap_Image <Matrix_Data_Type>::~Matrix_to_Bitmap_Image(void)
{
}

// ----------------------------------------------------------------------------

template <typename Matrix_Data_Type>
void Matrix_to_Bitmap_Image <Matrix_Data_Type>::process()
{
	Matrix<Matrix_Data_Type> & in = this->input();
	Bitmap_Image<Color_RGBA> & out = this->output();

    if (this->initialized == false)
    {
        if (in.rows() != out.height() || 
            in.columns() != out.width())
        {
    	    out.resize(in.columns(), in.rows());
        }
        this->initialized = true;
    }


	for (Count x = 0; x < out.width(); ++x)
	for (Count y = 0; y < out.height(); ++y)
	{
		out(x,y) = colour_mapping( in(x,y) );
	}
}

// ----------------------------------------------------------------------------

template <typename Matrix_Data_Type>
void Matrix_to_Bitmap_Image <Matrix_Data_Type>::dynamic_range(
	Matrix_Data_Type minimum, Matrix_Data_Type maximum)
{
	colour_mapping.dynamic_range(minimum, maximum);
}

// ----------------------------------------------------------------------------

}
#endif
