/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible author:     Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COLOR_MAPPING_H
#define BBP_COLOR_MAPPING_H

#include <cmath>
#include "BBP/Common/Types.h"
#include "Color_RGBA.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Mapping between a type with linear order and 32 bit RGBA color.
/*!
    \ingroup Graphics
*/
template <typename T>
class Color_Mapping
{
    typedef Byte Color_Component;
public:
    //! Constructs colour mapping with initialized range and resolution
    inline Color_Mapping(const T &          minimum, 
                         const T &          maximum, 
                         const Resolution   resolution  = 256);
    inline ~Color_Mapping();

    //! Map a specific value to 32 bit RGBA color.
    /*!
        \param value input value to the mapping
        \return 32 bit RGBA colour
    */
    inline Color_RGBA operator() (const T & value) const;
    //! Set boundaries of the dynamic range of the mapping.
    /*!
        \param minimum Minimum of the dynamic range of the mapping.
        \param maximum Maximum of the dynamic range of the mapping.
    */
    inline void dynamic_range(T     minimum, 
                              T     maximum);

    //! Set transfer function table.
    inline void transfer_function(const std::vector<Color_RGBA> & table);

private:
    std::vector<Color_RGBA>     _transfer_function;
    T                           _minimum, 
                                _maximum, 
                                _dynamic_range;
};


// ----------------------------------------------------------------------------

template <typename T>
Color_Mapping<T>::Color_Mapping(const T & minimum, 
                                const T & maximum, 
                                const Resolution resolution)
: _minimum(minimum), _maximum(maximum)
{
#ifndef NDEBUG
    std::clog << "Initialize color mapping" << std::endl;
#endif
    _dynamic_range = _maximum - _minimum;
    if (_dynamic_range < 0)
    {
        throw std::runtime_error("invalid color mapping clamp boundaries");
    }

    _transfer_function.resize(resolution);

    /*
        Initialize the mapping table with a mapping from transparent black to
        opaque white mapping by default.
		\bug Potential bug: Juan had changed BYTE_SIZE (which is 8)
		to sizeof(bbp::Byte) (which I think is 1). Check if this is true,
		if so this calculation is wrong now. Also the brackets may now
		be wrong, but check this. UPDATE: Solved this preliminary, wait
        for Juan's feedback to remove this bug flag. (TT)
    */
    for (Index i = 0; i < _transfer_function.size(); ++i)
    {
        _transfer_function[i].red((Color_Component) 
            ((i / (float) resolution) * 
             (pow(2.0f, (float) sizeof(Color_Component) * 8) - 1)));
        _transfer_function[i].green((Color_Component)
            ((i / (float) resolution) * 
             (pow(2.0f, (float) sizeof(Color_Component) * 8) - 1)));
        _transfer_function[i].blue((Color_Component) 
            ((i / (float) resolution) * 
             (pow(2.0f, (float) sizeof(Color_Component) * 8) - 1)));
        _transfer_function[i].alpha((Color_Component) 
            ((i / (float) resolution) * 
             (pow(2.0f, (float) sizeof(Color_Component) * 8) - 1)));
    }
}

// ----------------------------------------------------------------------------

template <typename T>
Color_Mapping<T>::~Color_Mapping()
{
}

// ----------------------------------------------------------------------------

template <typename T>
Color_RGBA Color_Mapping<T>::operator() (const T & value) const
{
    if (value <= _minimum)
    {
        return _transfer_function[0];
    }
    else if (value >= _maximum)
    {
        return _transfer_function[_transfer_function.size() - 1];
    }

    //! \bug cast to Byte will kill tf tables larger than a byte (TT)
    return _transfer_function[(Byte) ((value - _minimum) / 
                              _dynamic_range * _transfer_function.size() - 1)];
}

// ----------------------------------------------------------------------------

template <typename T>
void Color_Mapping<T>::dynamic_range (T minimum, T maximum)
{
    _minimum = minimum;
    _maximum = maximum;
    _dynamic_range = _maximum - _minimum;
}

// ----------------------------------------------------------------------------

template <typename T>
void Color_Mapping<T>::transfer_function(
    const std::vector<Color_RGBA> & table)
{
    _transfer_function = table;
}

// ----------------------------------------------------------------------------

}
#endif
