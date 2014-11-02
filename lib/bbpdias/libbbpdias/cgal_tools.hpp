#ifndef CGAL_TOOLS_HPP_
#define CGAL_TOOLS_HPP_

#include "libbbpdias/tools.hpp"

namespace dias {

    void tetcircumcenter(const dias::vect & a, const dias::vect & b,
                         const dias::vect & c, const dias::vect & d,
                         dias::vect & circumcenter);
}

// This is to enable to compile some programs without linking CGAL library in
// and with commented out abort make it runnable under Valgrind
namespace CGAL
{

    void
    assertion_fail( const char* expr,
                    const char* file,
                    int         line,
                    const char* msg);
}

#endif
