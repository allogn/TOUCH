#include "mbr.hpp"

#include <iostream>
#include <limits>

#include <boost/format.hpp>

namespace dias {

    mbr::mbr (void)
    {
        for (size_t i = 0; i < low.size(); i++)
        {
            low[i] = std::numeric_limits<double>::max();
            high[i] = std::numeric_limits<double>::min();
        }
    }

    mbr::mbr (const vect & _low, const vect & _high) :
        low (_low),
        high (_high)
    {
    }

    void mbr::expand (const vect & point)
    {
        for (size_t i = 0; i < low.size(); i++)
        {
            if (point[i] < low[i])  low[i] = point[i];
            if (point[i] > high[i]) high[i] = point[i];
        }
    }

    void mbr::expand (const mbr & other_mbr)
    {
        expand (other_mbr.low);
        expand (other_mbr.high);
    }

    void mbr::expand (const bbp::Vector_3D<bbp::Micron> & point)
    {
        if (point.x () < low[0])  low[0]  = point.x ();
        if (point.x () > high[0]) high[0] = point.x ();
        if (point.y () < low[1])  low[1]  = point.y ();
        if (point.y () > high[1]) high[1] = point.y ();
        if (point.z () < low[2])  low[2]  = point.z ();
        if (point.z () > high[2]) high[2] = point.z ();
    }

    dias::Box mbr::as_box (void) const
    {
        dias::Vertex l (low[0], low[1], low[2]);
        dias::Vertex h (high[0], high[1], high[2]);
        return dias::Box (l, h);
    }

    SpatialIndex::Region mbr::as_region (void) const
    {
        return SpatialIndex::Region (low.data (), high.data (), low.size ());
    }

    size_t mbr::serialized_size_in_float (void)
    {
        return 2 * dias::vect_size_float;
    }

    void mbr::serialize_as_float (byte ** ptr) const
    {
        dias::serialize_vect_as_float (low, ptr);
        dias::serialize_vect_as_float (high, ptr);
    }

    dias::mbr mbr::unserialize_as_float (byte ** ptr)
    {
        dias::vect l = dias::unserialize_vect_as_float (ptr);
        dias::vect h = dias::unserialize_vect_as_float (ptr);
        return dias::mbr (l, h);
    }

    void mbr::output_without_endl (std::ostream & out) const
    {
        out << boost::format("%1% %2% %3% %4% %5% %6%") %
            low[0] % low[1] % low[2] % high[0] % high[1] % high[2];
    }

    std::ostream & operator << (std::ostream & out, const mbr & m)
    {
        m.output_without_endl (out);
        out << '\n';
        return out;
    }

}
