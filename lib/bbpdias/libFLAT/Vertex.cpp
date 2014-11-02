#include <cassert>
#include <limits>
#include "SpatialObjectFactory.hpp"
#include "Vertex.hpp"
#include "Box.hpp"
namespace FLAT
{
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

	Vertex Vertex::operator+(const Vertex &rhs) const
	{
		Vertex temp;

		for (int i=0;i<DIMENSION;i++)
			temp.Vector[i] = Vector[i] + rhs.Vector[i];

		return temp;
	}

	Vertex Vertex::operator-(const Vertex &rhs) const
	{
		Vertex temp;

		for (int i=0;i<DIMENSION;i++)
			temp.Vector[i] = Vector[i] - rhs.Vector[i];

		return temp;
	}

	Vertex Vertex::operator*(const Vertex &rhs) const
	{
		Vertex temp;

		for (int i=0;i<DIMENSION;i++)
			temp.Vector[i] = Vector[i] * rhs.Vector[i];

		return temp;
	}

	Vertex Vertex::operator/(const Vertex &rhs) const
	{
		Vertex temp;

		for (int i=0;i<DIMENSION;i++)
			temp.Vector[i] = Vector[i] / rhs.Vector[i];

		return temp;
	}

	/*
	 * Static Geometric Calculation function
	 */

	// The Absolute difference in each dimension of the vector
	void Vertex::differenceVector(const Vertex &v1,const Vertex &v2, Vertex &difference)
	{
		for (int i=0;i<DIMENSION;i++)
			difference.Vector[i] = fabsf(v1.Vector[i]-v2.Vector[i]);
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
			d += (difference.Vector[i]) * (difference.Vector[i]);

		return d;
	}

	// Manhattan Distance between 2 points
	spaceUnit Vertex::manhattanDistance (const Vertex &v1,const Vertex &v2)
	{
		spaceUnit d=0;
		Vertex difference;
		Vertex::differenceVector(v1,v2,difference);

		for (int i=0;i<DIMENSION;i++)
			d += difference.Vector[i];

		return d;
	}

	// Midpoint of 2 Vertices
	void Vertex::midPoint (const Vertex &v1,const Vertex &v2,Vertex &mid)
	{
		for (int i=0;i<DIMENSION;i++)
			mid.Vector[i] = (v1.Vector[i]+v2.Vector[i]) /2;
	}

	// for SpatialObject Base Class
	Box Vertex::getMBR()
	{
		return Box(*this,*this);
	}

	spaceUnit Vertex::getSortDimension(int dimension)
	{
		return Vector[dimension];
	}

	void Vertex::serialize(uint8* buffer)
	{
		memcpy(buffer,&Vector,DIMENSION * sizeof(spaceUnit));
	}

	void Vertex::unserialize(uint8* buffer)
	{
		memcpy(&Vector, buffer,DIMENSION * sizeof(spaceUnit));
	}

	uint32 Vertex::getSize()
	{
		return DIMENSION * sizeof(spaceUnit);
	}

	// for SpatialObject Base Class
	Vertex Vertex::getCenter()
	{
		return *this;
	}

	bool Vertex::IsResult(Box& region)
	{
		return Box::enclose(region,*this);
	}

	SpatialObjectType Vertex::getType()
	{
		return VERTEX;
	}


}
