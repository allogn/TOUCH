#include "SpatialObjectFactory.hpp"
#include "Box.hpp"
#include "Vertex.hpp"
#include "Cone.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"

namespace FLAT
{
	SpatialObject* SpatialObjectFactory::create (SpatialObjectType objType)
	{
		 switch (objType)
		 {
		 case VERTEX:
			 return new Vertex();
		 case BOX:
			 return new Box();
		 case CONE:
			 return new Cone();
		 case TRIANGLE:
			 return new Triangle();
		 case SPHERE:
			 return new Sphere();
		 case NONE:
			 exit(1);
		 }
		 exit(1);
		 return NULL;
	}

	uint32 SpatialObjectFactory::getSize (SpatialObjectType objType)
	{
		 switch (objType)
		 {
		 case VERTEX:
			 return DIMENSION*sizeof(spaceUnit);
		 case BOX:
			 return DIMENSION*2*sizeof(spaceUnit);
		 case CONE:
			 return ((DIMENSION*2)+2)*sizeof(spaceUnit);
		 case TRIANGLE:
			 return DIMENSION*3*sizeof(spaceUnit);
		 case SPHERE:
			 return (DIMENSION+1)*sizeof(spaceUnit);
		 case NONE:
			 exit(1);
		 }
		 exit(1);
		 return 0;
	}

	std::string SpatialObjectFactory::getTitle (SpatialObjectType objType)
	{
		 switch (objType)
		 {
		 case VERTEX:
			 return "Vertex";
		 case BOX:
			 return "Axis-Aligned-Box";
		 case CONE:
			 return "Segment-Cone";
		 case TRIANGLE:
			 return "Mesh-Triangle";
		 case SPHERE:
			 return "Sphere";
		 case NONE:
			 exit(1);
		 }
		 exit(1);
		 return "";
	}

}
