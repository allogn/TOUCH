#include "SpatialObject.hpp"
#include "Vertex.hpp"

namespace FLAT
{
    SpatialObject::SpatialObject()
    {

    }
    SpatialObject::~SpatialObject()
    {

    }
	bool SpatialObjectXAsc::operator()(SpatialObject* const r1, SpatialObject* const r2)
	{
		if (r1->getSortDimension(0) < r2->getSortDimension(0)) return true;
		else return false;
	}
	bool SpatialObjectYAsc::operator()(SpatialObject* const r1, SpatialObject* const r2)
	{
		if (r1->getSortDimension(1) < r2->getSortDimension(1)) return true;
		else return false;
	}
	bool SpatialObjectZAsc::operator()(SpatialObject* const r1, SpatialObject* const r2)
	{
		if (r1->getSortDimension(2) < r2->getSortDimension(2)) return true;
		else return false;
	}
}
