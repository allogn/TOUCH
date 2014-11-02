#include "Sphere.hpp"


namespace FLAT
{
	std::ostream & operator << (std::ostream & lhs, const Sphere& rhs)
	{
		lhs << "Center: " << rhs.center << "[" << rhs.radius << "]";
		return lhs;
	}

	spaceUnit Sphere::diameter(const Sphere &s)
	{
		return s.radius*2;
	}

	bigSpaceUnit Sphere::volume(const Sphere &s)
	{
		return (s.radius*s.radius*s.radius*M_PI*4/3);
	}

	// Spatial Object virtual functions
	Box Sphere::getMBR()
	{
		Box box;
		for (int i=0;i<DIMENSION;i++)
		{
			box.high.Vector[i] = center.Vector[i]+radius;
			box.low.Vector[i]  = center.Vector[i]-radius;
		}
		return box;
	}
	Vertex Sphere::getCenter()
	{
		return center;
	}
	spaceUnit Sphere::getSortDimension(int dimension)
	{
		return center.Vector[dimension];
	}
	bool Sphere::IsResult(Box& region)
	{
		return true;
	}
	SpatialObjectType Sphere::getType()
	{
		return SPHERE;
	}

	void Sphere::serialize(uint8* buffer)
	{
		uint8* ptr = buffer;
		memcpy(ptr,&(center.Vector),DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(ptr,&radius,DIMENSION * sizeof(spaceUnit));
	}

	void Sphere::unserialize(uint8* buffer)
	{
		uint8* ptr = buffer;
		memcpy(&(center.Vector), ptr,DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(&radius, ptr,DIMENSION * sizeof(spaceUnit));
	}

	uint32 Sphere::getSize()
	{
		return (DIMENSION+1) * sizeof(spaceUnit);
	}
}
