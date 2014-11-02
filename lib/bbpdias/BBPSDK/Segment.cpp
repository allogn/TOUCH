/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:	Thomas Traenkler
                                Nikolai Chapochnikov

*/

#if defined(_MSC_VER)
#pragma once
#endif

#include "BBP/Model/Microcircuit/Segment.h"

namespace bbp {

// ----------------------------------------------------------------------------

Segment_ID Segment::id() const
{
    Interval<Morphology_Point_ID> section;
    for (size_t i = 0; i < _morphology->number_of_sections(); ++i)
    {
        section.begin = _morphology->section_start_points()[i];
        section.end   = section.begin + 
                        _morphology->section_point_counts()[i];
        if (_begin_point_id >= section.begin && 
            _begin_point_id < section.end)
        {
            return _begin_point_id - section.begin;
        }
    }
    //! \todo Throw exception instead. (TT)
    return UNDEFINED_SEGMENT_ID;
}

// ----------------------------------------------------------------------------

} // end namespace bbp
