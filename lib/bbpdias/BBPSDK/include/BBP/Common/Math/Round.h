/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors:    Thomas Traenkler
                    Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMMON_ROUND_H
#define BBP_COMMON_ROUND_H

#ifdef WIN32
#include <cmath>

#undef max
#undef min

// ----------------------------------------------------------------------------

inline float round(float value)
{
    return value >= 0.0f ? std::floor(value + 0.5f): std::ceil(value - 0.5f);
}

// ----------------------------------------------------------------------------

inline double round(double value) 
{
    return value >= 0.0 ? std::floor(value + 0.5) : std::ceil(value - 0.5); 
}

// ----------------------------------------------------------------------------

inline long double round(long double value) 
{
    return value >= 0.0 ? std::floor(value + 0.5) : std::ceil(value - 0.5); 
}

// ----------------------------------------------------------------------------

#endif
#endif
