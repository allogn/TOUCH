#include "libbbpdias/cgal_tools.hpp"

#include <iostream>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Tetrahedron_3.h>
#include <CGAL/Point_3.h>

namespace dias {

    void tetcircumcenter(const dias::vect & a, const dias::vect & b,
                         const dias::vect & c, const dias::vect & d,
                         dias::vect & circumcenter)
    {
        typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

        CGAL::Point_3<K> p1(a[0],a[1],a[2]);
        CGAL::Point_3<K> p2(b[0],b[1],b[2]);
        CGAL::Point_3<K> p3(c[0],c[1],c[2]);
        CGAL::Point_3<K> p4(d[0],d[1],d[2]);

        CGAL::Tetrahedron_3<K> t(p1,p2,p3,p4);
        CGAL::Point_3<K> center = CGAL::circumcenter(t);
        circumcenter[0] = center.x ();
        circumcenter[1] = center.y ();
        circumcenter[2] = center.z ();
    }
}

// This is to enable to compile some programs without linking CGAL library in
// and with commented out abort make it runnable under Valgrind
namespace CGAL
{

    void
    assertion_fail( const char* expr,
                    const char* file,
                    int         line,
                    const char* msg)
    {
        std::cerr << "expr = " << expr << '\n';
        std::cerr << "file = " << file << '\n';
        std::cerr << "line = " << line << '\n';
        std::cerr << "msg = " << msg << '\n';
        abort ();
    }
}

