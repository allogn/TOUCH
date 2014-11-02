#ifndef MBR_HPP_
#define MBR_HPP_

#include <iostream>

#include <BBP/Model/Microcircuit/Segment.h>

#include "tools.hpp"
#include "Box.hpp"

#include <SpatialIndex.h>

namespace dias {

    class mbr
    {
    public:
        mbr (void);

        mbr (const vect & _low, const vect & _high);

        void expand (const vect & point);

        void expand (const mbr & other_mbr);

        void expand (const bbp::Vector_3D<bbp::Micron> & point);

        dias::Box as_box (void) const;

        SpatialIndex::Region as_region (void) const;

        static size_t serialized_size_in_float (void);

        void serialize_as_float (byte ** ptr) const;

        static mbr unserialize_as_float (byte ** ptr);

        void output_without_endl (std::ostream & out) const;

    private:
        dias::vect low;
        dias::vect high;

        friend std::ostream & operator << (std::ostream & out, const mbr & m);
    };

}

#endif
