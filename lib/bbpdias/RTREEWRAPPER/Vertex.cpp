#include "Vertex.hpp"

#include <cassert>
#include <limits>

namespace dias {

std::ostream& operator << (std::ostream & lhs, const Vertex & rhs)
{
	lhs << "(";
	for(int i=0;i<DIMENSION;i++)
		if (i==DIMENSION-1)
			lhs << rhs[i] << ")";
		else
			lhs << rhs[i] << ",";
	return lhs;
}

spaceUnit & Vertex::operator [](int subscript)
{
    assert (subscript >= 0 && subscript < DIMENSION);
    return Vector[subscript];
}

Vertex Vertex::operator+(const Vertex &rhs) const
{
	Vertex temp;

	for (int i=0;i<DIMENSION;i++)
		temp[i] = Vector[i] + rhs[i];

	return temp;
}

Vertex Vertex::operator-(const Vertex &rhs) const
{
	Vertex temp;

	for (int i=0;i<DIMENSION;i++)
		temp[i] = Vector[i] - rhs[i];

	return temp;
}

Vertex Vertex::operator*(const Vertex &rhs) const
{
	Vertex temp;

	for (int i=0;i<DIMENSION;i++)
		temp[i] = Vector[i] * rhs[i];

	return temp;
}

Vertex Vertex::operator/(const Vertex &rhs) const
{
	Vertex temp;

	for (int i=0;i<DIMENSION;i++)
		temp[i] = Vector[i] / rhs[i];

	return temp;
}

bool   Vertex::operator==(const Vertex &rhs) const
	{
	int i=0;
	for (i=0;i<DIMENSION;i++)
            if (fabs (Vector[i] - rhs[i])
                > std::numeric_limits<spaceUnit>::epsilon ()) return false;
	return true;
	}

/*
 * Static Geometric Calculation function
 */

// The Absolute difference in each dimension of the vector
void Vertex::differenceVector(const Vertex &v1,const Vertex &v2, Vertex &difference)
	{
		for (int i=0;i<DIMENSION;i++)
			difference[i] = fabsf(v1[i]-v2[i]);
	}

// Euclidean Distance between 2 points
spaceUnit Vertex::distance (const Vertex &v1,const Vertex &v2)
	{
	spaceUnit d = (spaceUnit)sqrt(Vertex::squaredDistance(v1,v2));
	return d;
	}

// Squared Euclidean Distances between 2 points FASter
bigSpaceUnit Vertex::squaredDistance (const Vertex &v1,const Vertex &v2)
	{
	bigSpaceUnit d=0;
	Vertex difference;
	Vertex::differenceVector(v1,v2,difference);

	for (int i=0;i<DIMENSION;i++)
		d += (difference[i]) * (difference[i]);

	return d;
	}

// Manhattan Distance between 2 points
spaceUnit Vertex::manhattanDistance (const Vertex &v1,const Vertex &v2)
	{
	spaceUnit d=0;
	Vertex difference;
	Vertex::differenceVector(v1,v2,difference);

	for (int i=0;i<DIMENSION;i++)
		d += difference[i];

	return d;
	}

// Midpoint of 2 Vertices
void Vertex::midPoint (const Vertex &v1,const Vertex &v2,Vertex &mid)
	{
	for (int i=0;i<DIMENSION;i++)
		mid[i] = (v1[i]+v2[i]) /2;
        }

}
