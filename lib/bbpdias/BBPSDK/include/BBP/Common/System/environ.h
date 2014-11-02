/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_ENVIRON_H
#define BBP_ENVIRON_H

#include <string>

namespace bbp
{
    /*! Custom, reentrant, implementation of getenv to avoid MSVC++ complains
        about unsafety of getenv.
        @param name The name of the variable to search in the environment.
        @param value The value of the variable. Input value left untouched if
        the variable doesn't exists.
        @return true if the variable exists and false otherwise.
    */
    bool getenv(const char *name, std::string &value);
}

#endif
