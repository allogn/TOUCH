#ifndef CONE
#define CONE

#include <math.h>
#include "libbbpdias/Vertex.hpp"
#include "libbbpdias/Box.hpp"

#define DISCRETIZE_RESOLUTION 10

namespace dias {
/*
 * Conic Structure
 * and Basic Geometric Functions
 *
 * Author: Farhan Tauheed
 */
class Cone
{
public:
        Vertex begin;
        Vertex end;
        spaceUnit radiusBegin;
	spaceUnit radiusEnd;

	Cone()
	{
	}
	Cone(const Vertex& b,const Vertex& e,const spaceUnit& r1, const spaceUnit& r2)
	{
		begin = b;
		end   = e;
		radiusBegin = r1;
		radiusEnd   = r2;
	}

	Cone 		 & operator  =(const Cone& rhs);
	bool 		   operator ==(const Cone& rhs) const;
	friend std::ostream & operator << (std::ostream & lhs, const Cone& rhs);

	static void center(const Cone &c,Vertex &v);
	static spaceUnit length(const Cone &c);
	static bigSpaceUnit volume(const Cone &c);
	static void boundingBox(const Cone &c,Box &box);
	static bool overlap(const Cone &c, const Box &b);
	static void discretize(const Cone &c,Vertex vertices[]);
};

std::ostream& operator << (std::ostream & lhs, const Cone & rhs)
	{
	lhs << "Begin: " << rhs.begin << "[" << rhs.radiusBegin << "]   End: " << rhs.end << "[" << rhs.radiusEnd << "]";
	return lhs;
	}

Cone & Cone::operator=(const Cone &rhs)
	{
	begin  = rhs.begin;
	end    = rhs.end;
	radiusBegin = rhs.radiusBegin;
	radiusEnd   = rhs.radiusEnd;
	return *this;
	}

bool Cone::operator==(const Cone &rhs) const
	{
	return  (begin==rhs.begin && end==rhs.end && radiusBegin==rhs.radiusBegin && radiusEnd == rhs.radiusEnd);
	}

void Cone::center(const Cone &c,Vertex &center)
	{
	Vertex::midPoint (c.begin,c.end,center);
	}

spaceUnit Cone::length(const Cone &c)
	{
	return Vertex::distance(c.begin,c.end);
	}

//http://www.grc.nasa.gov/WWW/K-12/rocket/Images/volumenose.gif
bigSpaceUnit Cone::volume(const Cone &c)
	{
	bigSpaceUnit volume=0;
	volume = (M_PI/12) * length(c) * ((c.radiusBegin*c.radiusBegin) + (c.radiusBegin*c.radiusEnd) + (c.radiusEnd*c.radiusEnd) );
	return volume;
	}

//Farhan's implementation of the tightest bounding box
void Cone::boundingBox(const Cone &c, Box &box)
	{
	spaceUnit distance=0;
	Vertex difference,projectionBegin,projectionEnd;

	// Calculate Projection on End Points
	distance = length(c);
	Vertex::differenceVector(c.begin,c.end,difference);

	for (int i=0;i<DIMENSION;i++)
		if (distance>0)
		{
		projectionBegin[i]= sinf(acosf(difference[i] / distance))*c.radiusBegin;
		projectionEnd[i]  = sinf(acosf(difference[i] / distance))*c.radiusEnd;
		}

	// Make Covering Box on both end Points and Combine
	Box boxBegin,boxEnd;

	Box::coveringBox(c.begin,projectionBegin,boxBegin);
	Box::coveringBox(c.end,projectionEnd,boxEnd);

	Box::combine(boxBegin,boxEnd,box);
	}

//Discretize vertices on the Surface of Cone
void Cone::discretize(const Cone &c __attribute__((__unused__)),
                      Vertex vertices[] __attribute__((__unused__)))
	{
	//TODO
	}

// BBP method of if Cone overlaps the Box
bool Cone::overlap(const Cone&c,const Box&b)
	{
	Vertex vertices[DISCRETIZE_RESOLUTION];
	discretize(c,vertices);

	int i=0;
	for (i=0;i<DISCRETIZE_RESOLUTION;i++)
		if (Box::enclose(b,vertices[i])==true) return true;

	return false;
	}

}
#endif
