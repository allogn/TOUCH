/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2008. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_BITMAP_IMAGE_H
#define BBP_BITMAP_IMAGE_H

#include <vector>
#include "BBP/Common/Types.h"

namespace bbp {

//-----------------------------------------------------------------------------

//! Two dimensional bitmap raster image made of pixels.
/*!
    Specify the pixel colour type as a class template parameter.
    (e.g. Bitmap_Image<Color_RGBA).
    \ingroup Graphics
*/
template <typename Pixel_Colour_Type>
class Bitmap_Image
{
public:
    //! Create an empty image.
    inline Bitmap_Image();
    //! Create an image with specified size.
    inline Bitmap_Image(Resolution x, Resolution y);
    inline ~Bitmap_Image();

    //! Access a pixel by x and y coordinates (origin upper left).
    inline const Pixel_Colour_Type & operator() (Resolution x, Resolution y) const;
    //! Access a pixel by x and y coordinates (origin upper left).
    inline Pixel_Colour_Type & operator() (Resolution x, Resolution y);
    //! Resize image dimensions (content will be lost).
    inline void resize(Resolution width, Resolution height);
    //! Retrieve width of image.
    inline Resolution width() const;
    //! Retrieve height of image.
    inline Resolution height() const;
    //! Fill whole image with the same color.
    void fill(const Pixel_Colour_Type & value);

    //! Get number of pixels in the image.
    size_t size() const;
    //! Get linear vector with pixels.
    std::vector<Pixel_Colour_Type> & vector() const;

private:

    std::vector<Pixel_Colour_Type>  _pixels;
    Resolution                      _width, 
                                    _height;
};


//-----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
std::ostream & operator << 
    (std::ostream & lhs, Bitmap_Image<Pixel_Colour_Type> & rhs)
{
    lhs << "Bitmap_Image (" << rhs.width() << "x" 
        << rhs.height() << ")" << std::endl;

    std::cout << "{ ";
    for (Resolution y = 0; y < rhs.height(); ++y)
    {
        lhs << "( ";
        for (Resolution x = 0; x < rhs.width(); ++x)
        {
            lhs << rhs(x,y) << " ";
        }
        lhs << ")" << std::endl;
    }
    lhs << "}" << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
Bitmap_Image<Pixel_Colour_Type>::Bitmap_Image()
: _width(0), _height(0)
{
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
Bitmap_Image<Pixel_Colour_Type>::Bitmap_Image
    (Resolution width, Resolution height)
: _width(width), _height(height)
{
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
Bitmap_Image<Pixel_Colour_Type>::~Bitmap_Image()
{
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
std::vector<Pixel_Colour_Type> & 
    Bitmap_Image<Pixel_Colour_Type>::vector() const
{
    return _pixels;
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
size_t Bitmap_Image<Pixel_Colour_Type>::size() const
{
    return _pixels.size();
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
const Pixel_Colour_Type & Bitmap_Image<Pixel_Colour_Type>::operator()
    (Resolution x, Resolution y) const
{
    bbp_assert (x < _width && y < _height);
    return _pixels[y * _width + x];
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
Pixel_Colour_Type & Bitmap_Image<Pixel_Colour_Type>::operator()
    (Resolution x, Resolution y)
{
    bbp_assert (x < _width && y < _height);
    return _pixels[y * _width + x];
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
void Bitmap_Image<Pixel_Colour_Type>::resize
    (Resolution width, Resolution height)
{
    _pixels.resize(width * height);
    _width = width;
    _height = height;
    std::clog << "Bitmap image resized to " 
              << width << "x" << height << std::endl;
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
Resolution Bitmap_Image<Pixel_Colour_Type>::width() const
{
    return _width;
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
Resolution Bitmap_Image<Pixel_Colour_Type>::height() const
{
    return _height; 
}

// ----------------------------------------------------------------------------

template <typename Pixel_Colour_Type>
void Bitmap_Image<Pixel_Colour_Type>::fill(const Pixel_Colour_Type & color)
{
    std::fill(_pixels.begin(), _pixels.end(), color);
}

// ----------------------------------------------------------------------------

}
#endif
