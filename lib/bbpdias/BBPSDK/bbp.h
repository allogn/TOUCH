/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#pragma message("Compiling bbp.h\n")
#endif

#ifndef BBP_SDK_HEADERS_H
#define BBP_SDK_HEADERS_H

/*
    This main header will include everything!
    For faster parsing, other headers are 
    included implicitly.

	It may include a precompiled header instead
	if the define BBP_PRECOMPILED_HEADERS is enabled
	via the cmake option PRECOMPILED_HEADERS "ON".
*/

#ifdef WIN32

#ifdef BBP_PRECOMPILED_HEADERS
#include "../src/Model/stdafx.h"           // PRECOMPILED HEADERS
#else
#include "BBP/common.h"
#include "BBP/model.h"                  // includes common also
#include "BBP/filters.h"           // NON-PRECOMPILED HEADERS
#endif

#else
#include "BBP/common.h"
#include "BBP/model.h"                  // includes common also
#include "BBP/filters.h"      // NON-PRECOMPILED HEADERS
#endif // WIN32

#endif // INCLUDE GUARD

#if defined(_MSC_VER)
#pragma once
#pragma message("Compiling bbp.h finished\n")
#endif
