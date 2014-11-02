#include "Triangle.hpp"

namespace FLAT
{
	std::ostream& operator << (std::ostream & lhs, const Triangle & rhs)
	{
		lhs << rhs.vertex1 << " " << rhs.vertex2 << " " << rhs.vertex3;
		return lhs;
	}

	Box Triangle::getMBR()
		{
		Box box;
		boundingBox(*this,box);
		return box;
		}

	Vertex Triangle::getCenter()
		{
		Vertex center;
		for (int i=0;i<DIMENSION;i++)
			center.Vector[i] = (vertex1.Vector[i]+vertex2.Vector[i]+vertex3.Vector[i])/3;
		return center;
		}

	spaceUnit Triangle::getSortDimension(int dim)
		{
		return vertex1.Vector[dim]+vertex2.Vector[dim]+vertex3.Vector[dim];
		}

	SpatialObjectType Triangle::getType()
	{
		return TRIANGLE;
	}

	bool Triangle::IsResult(Box& region)
	{
		return true;
	}

	void Triangle::serialize(uint8* buffer)
	{
		uint8* ptr = buffer;
		memcpy(ptr,&(vertex1.Vector),DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(ptr,&(vertex2.Vector),DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(ptr,&(vertex3.Vector),DIMENSION * sizeof(spaceUnit));
	}

	void Triangle::unserialize(uint8* buffer)
	{
		uint8* ptr = buffer;
		memcpy(&(vertex1.Vector), ptr,DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(&(vertex2.Vector), ptr,DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(&(vertex3.Vector), ptr,DIMENSION * sizeof(spaceUnit));
	}

	uint32 Triangle::getSize()
	{
		return DIMENSION * 3 * sizeof(spaceUnit);
	}

}
