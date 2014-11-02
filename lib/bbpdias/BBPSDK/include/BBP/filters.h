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

#ifndef BBP_SDK_FILTERS_INCLUDES_H
#define BBP_SDK_FILTERS_INCLUDES_H

// model ______________________________________________________________________


#include "model.h" // includes common also


// filters _____________________________________________________________

//#ifdef BBP_THREADS_SUPPORTED
#include "BBP/Common/Filter/Filter_Reader.h"
#include "BBP/Filters/Volume/Volume_Temporal_Average.h"
#include "BBP/Filters/Volume/Volume_Voltage_to_Byte.h"
#include "BBP/Filters/Volume/Volume_Voltage_to_Colour.h"
#include "BBP/Filters/Volume/Volume_Maximum_Intensity_Projection.h"
#include "BBP/Filters/Matrix/Matrix_to_Bitmap_Image.h"
#include "BBP/Filters/Matrix/Matrix_ASCII_File_Stream_Writer.h"
#include "BBP/Filters/Volume/Volume_Printer.h"
#include "BBP/Filters/Volume/Volume_NRRD_File_Reader.h"
#include "BBP/Filters/Volume/Volume_NRRD_File_Stream_Writer.h"
#include "BBP/Filters/Volume/Volume_ASCII_File_Stream_Writer.h"

#include "BBP/Filters/Microcircuit/Segment_Voxel_Mapper.h"
#include "BBP/Filters/Microcircuit/Segment_Voxel_Density.h"

#include "BBP/Filters/Microcircuit/Compartment_Voltage_Voxel_Average.h"
#include "BBP/Filters/Microcircuit/Compartment_Voltages_to_Microcircuit.h"
#include "BBP/Filters/Microcircuit/Compartment_Report_Printer.h"
#include "BBP/Filters/Microcircuit/Compartment_Report_Comparison.h"
#include "BBP/Filters/Image/Image_BMP_File_Stream_Writer.h"
#include "BBP/Filters/Image/Image_TIFF_File_Stream_Writer.h"
//#endif // threads

#endif // include guard
