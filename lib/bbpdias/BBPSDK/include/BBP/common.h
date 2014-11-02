/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Sebastien Lasserre
                                Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SDK_COMMON_INCLUDES_H
#define BBP_SDK_COMMON_INCLUDES_H

// common _____________________________________________________________________

#include "BBP/Common/Version.h"
#include "BBP/Common/Types.h"
#include "BBP/Common/Math/Matrix.h"
#include "BBP/Common/System/Time/Timer.h"
#include "BBP/Common/Dataset/Containers/Array.h"
#include "BBP/Common/Graphics/2D/Writers/Image_BMP_File_Writer.h"
#include "BBP/Common/Graphics/2D/Writers/Image_TIFF_File_Writer.h"

//#ifdef BBP_THREADS_SUPPORTED
#include "BBP/Common/System/parallel.h"
//#endif

#endif // include guard
