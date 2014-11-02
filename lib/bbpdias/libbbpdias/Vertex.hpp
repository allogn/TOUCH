#ifndef VERTEX_HPP_
#define VERTEX_HPP_

#include "math.h"
#include "TemporaryFile.hpp"
#include <cassert>
#include <iostream>

namespace dias {

#define DIMENSION 3 // should be atleast 1
typedef float  spaceUnit;
typedef double bigSpaceUnit;

/*
* Vertex or Point Structure
* And Basic Geometric Functions
* Supports N-Dimensions
*
* Author: Farhan Tauheed
*/

class Vertex
{
public:
    spaceUnit Vector[DIMENSION];

    Vertex()
        {
            for (int i=0;i<DIMENSION;i++)
                Vector[i]=0;
        }

    Vertex(spaceUnit x, spaceUnit y ,spaceUnit z) //For just 3D Vertex
        {
            Vector[0]=x;
            Vector[1]=y;
            Vector[2]=z;
        }

    friend std::ostream  & operator << (std::ostream & lhs,const Vertex & rhs);

    spaceUnit       & operator[](int subscript);

    const spaceUnit & operator[](int subscript) const;

    Vertex          & operator=(const Vertex &rhs);

    bool 		 operator==(const Vertex &rhs) const;

    Vertex 		 operator+(const Vertex &rhs) const;

    Vertex 		 operator-(const Vertex &rhs) const;

    Vertex 		 operator*(const Vertex &rhs) const;

    Vertex 		 operator/(const Vertex &rhs) const;

    static void differenceVector(const Vertex &v1,const Vertex &v2, Vertex &difference);

    static spaceUnit distance (const Vertex &v1,const Vertex &v2);

    static bigSpaceUnit squaredDistance (const Vertex &v1,const Vertex &v2);

    static spaceUnit manhattanDistance (const Vertex &v1,const Vertex &v2);

    static void midPoint (const Vertex &v1,const Vertex &v2, Vertex &mid);

    bool store(Farhan::TemporaryFile &tf);

    bool load(Farhan::TemporaryFile &tf);


	struct xAsc : public std::binary_function<Vertex* const, Vertex* const, bool>
	{
		bool operator()(Vertex* const r1, Vertex* const r2)
		{
			if ((*r1)[0] < (*r2)[0]) return true;
			else return false;
		}
	};

	struct yAsc : public std::binary_function<Vertex* const, Vertex* const, bool>
	{
		bool operator()(Vertex* const r1, Vertex* const r2)
		{
			if ((*r1)[1] < (*r2)[1]) return true;
			else return false;
		}
	};

	struct zAsc : public std::binary_function<Vertex* const, Vertex* const, bool>
	{
		bool operator()(Vertex* const r1, Vertex* const r2)
		{
			if ((*r1)[2] < (*r2)[2]) return true;
			else return false;
		}
	};
};

inline const spaceUnit & Vertex::operator [](int subscript) const
{
    assert (subscript >= 0 && subscript < DIMENSION);
    return Vector[subscript];
}

inline Vertex& Vertex::operator = (const Vertex & rhs)
{
    for (int i = 0; i < DIMENSION; i++)
        Vector[i] = rhs[i];
    return *this;
}

}

#endif
