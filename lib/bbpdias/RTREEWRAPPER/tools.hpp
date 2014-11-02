// Useful definitions that do not depend on SpatialIndex nor BBPSDK
#ifndef TOOLS_HPP_
#define TOOLS_HPP_

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdint.h>

#include <boost/array.hpp>
#include <boost/format.hpp>

typedef uint8_t byte;

namespace dias {

    //typedef uint8_t byte;

    typedef boost::array<double, 3> vect;
    enum { vect_size_double = sizeof(double) * 3 };
    enum { vect_size_float = sizeof(float) * 3 };
    enum { vect_size = vect_size_double };

    template <class T>
    void serialize (const T & t, byte ** serialized_ptr)
    {
        memcpy (*serialized_ptr, &t, sizeof (t));
        *serialized_ptr += sizeof (t);
    }

    template <class T>
    void unserialize (T & t, byte ** serialized_ptr)
    {
        memcpy (&t, *serialized_ptr, sizeof (t));
        *serialized_ptr += sizeof (t);
    }

    void include_mbr (vect * plow, vect * phigh,
                      const vect & ilow, const vect & ihigh);

    void serialize_vect_as_float (const vect & v, byte ** serialized_ptr);

    inline dias::vect unserialize_vect_as_float (byte ** ptr)
    {
        dias::vect result;
        float tmp;
        dias::unserialize (tmp, ptr);
        result[0] = tmp;
        dias::unserialize (tmp, ptr);
        result[1] = tmp;
        dias::unserialize (tmp, ptr);
        result[2] = tmp;
        return result;
    }

    inline std::ostream & operator << (std::ostream & out,
                                       const dias::vect & v)
    {
        out << v[0] << " " << v[1] << " " << v[2];
        return out;
    }
}

#endif
