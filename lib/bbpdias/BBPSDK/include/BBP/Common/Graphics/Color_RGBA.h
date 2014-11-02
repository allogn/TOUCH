/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2008. All rights reserved.

		Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COLOR_RGBA_H
#define BBP_COLOR_RGBA_H

#include <iostream>
#include "BBP/Common/Types.h"

namespace bbp {

//---------------------------------------------------------------------------

//! Color labels.
enum Color_Name
{
	COLOR_RED           =   0xFF000000,
	COLOR_GREEN         =   0x00FF0000,
	COLOR_BLUE          =   0x0000FF00,
	COLOR_BLACK         =   0x000000FF,
	COLOR_WHITE         =   0xFFFFFFFF,
	COLOR_TRANSPARENT   =   0x00000000
};

//---------------------------------------------------------------------------

//! Color RGBA 32 bit (red, green, blue, alpha; 8 bit per component)
/*!
    \ingroup Graphics
*/
class Color_RGBA
{
public:
    inline Color_RGBA();
	inline Color_RGBA(Color_Name name);
	inline Color_RGBA(Byte red, Byte green, Byte blue);
	inline Color_RGBA(Byte red, Byte green, Byte blue, Byte alpha);
	inline ~Color_RGBA();

    //! Get red color component.
    const Byte & red() const 
    { 
        return _red; 
    }
    //! Get green color component.
    const Byte & green() const 
    { 
        return _green; 
    }
    //! Get blue color component.
    const Byte & blue() const 
    { 
        return _blue; 
    }
    //! Get alpha transparency color component.
    const Byte & alpha() const 
    {
        return _alpha; 
    }

    //! Set red color component.
    void red(const Byte & value)
    { 
        _red = value; 
    }
    
    //! Set green color component.
    void green(const Byte & value)
    { 
        _green = value; 
    }
    
    //! Set blue color component.
    void blue(const Byte & value)
    { 
        _blue = value; 
    }
    
    //! Set alpha transparency color component.
    void alpha(const Byte & value)
    {
        _alpha = value; 
    }


	inline void print();

private:
    Byte    _red,    //!< Red intensity component, range [0..255].
            _green,  //!< Green intensity component, range [0..255].
            _blue,   //!< Blue intensity component, range [0..255].
            _alpha;  //!< Alpha opacity component , range [0..255].
};

inline std::ostream & operator << (std::ostream & lhs, Color_RGBA & rhs);


// ----------------------------------------------------------------------------

Color_RGBA::Color_RGBA()
{
}

// ----------------------------------------------------------------------------

Color_RGBA::Color_RGBA(Byte red, Byte green, Byte blue)
: _red(red), _green(green), _blue(blue), _alpha(255)
{
}

// ----------------------------------------------------------------------------

Color_RGBA::Color_RGBA(Byte red, Byte green, Byte blue, Byte alpha)
: _red(red), _green(green), _blue(blue), _alpha(alpha)
{
}

// ----------------------------------------------------------------------------

Color_RGBA::Color_RGBA(Color_Name name)
{
	switch (name)
	{
		case COLOR_RED:
			red(255);
			green(0);
			blue(0);
            alpha(255);
			break;

		case COLOR_GREEN:
			red(0);
			green(255);
			blue(0);
            alpha(255);
			break;

		case COLOR_BLUE:
			red(0);
			green(0);
			blue(255);
            alpha(255);
			break;

		case COLOR_WHITE:
			red(255);
			green(255);
			blue(255);
            alpha(255);
			break;

		case COLOR_BLACK:
			red(0);
			green(0);
			blue(0);
            alpha(255);
			break;

		case COLOR_TRANSPARENT:
			red(0);
			green(0);
			blue(0);
			alpha(0);
			break;
	}
}

// ----------------------------------------------------------------------------

Color_RGBA::~Color_RGBA()
{
}

// ----------------------------------------------------------------------------

void Color_RGBA::print()
{
    std::cout << * this << std::flush;
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, Color_RGBA & rhs)
{
    lhs << "Color = { "
        << "red "   << static_cast<unsigned long>(rhs.red())   << ", "
        << "green " << static_cast<unsigned long>(rhs.green()) << ", "
        << "blue "  << static_cast<unsigned long>(rhs.blue())  << ", "
        << "alpha " << static_cast<unsigned long>(rhs.alpha()) << " }"
        << std::endl;
	return lhs;
}

// ----------------------------------------------------------------------------

}
#endif /*BBP_COLOR_RGBA_H*/
