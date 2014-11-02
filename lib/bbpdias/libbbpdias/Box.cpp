#include <vector>

#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/lexical_cast.hpp>

#include "Box.hpp"

static boost::mt11213b generator (42u);

namespace dias {

std::ostream & operator << (std::ostream & lhs,const Box & rhs)
{
	lhs << "Low: " << rhs.low << "  High: " << rhs.high;
	return lhs;
}

Box&  Box::operator=(const Box &rhs)
{
	low  = rhs.low;
	high = rhs.high;
	return *this;
}

bool Box::operator==(const Box &rhs) const
	{
	return (low==rhs.low && high==rhs.high);
	}

/*
 * Static Functions
 */

// Make a bounding Box over a sphere or eliptical surface
void Box::coveringBox (const Vertex &center, const Vertex &radialVector, Box &box)
	{
	box.low  = center - radialVector;
	box.high = center + radialVector;
	}

// Give the volume of the Box
bigSpaceUnit Box::volume(const Box &b)
	{
	bigSpaceUnit v=1;
	Vertex distance;
	Vertex::differenceVector(b.low,b.high,distance);

	for (int i=0;i<DIMENSION;i++)
		v *= distance[i];
	return v;
	}

// Give the Center Coordinate of the Box
void Box::center(const Box &b, Vertex &center)
	{
	Vertex::midPoint (b.low,b.high,center);
	}

// Side Length of a particular dimension
spaceUnit Box::length(const Box &b,const int dimension)
	{
	return (b.high[dimension]-b.low[dimension]);
	}

// Do 2 Boxes overlap Any Volume?
bool Box::overlap (const Box &b1,const Box &b2)
	{
	for (int i=0;i<DIMENSION;i++)
	{
		int countOverlap=0;
		if (b1.low[i]  >= b2.low[i] && b1.low[i]  <= b2.high[i]) countOverlap++;
		if (b1.high[i] >= b2.low[i] && b1.high[i] <= b2.high[i]) countOverlap++;
		if (b2.low[i]  >= b1.low[i] && b2.low[i]  <= b1.high[i]) countOverlap++;
		if (b2.high[i] >= b1.low[i] && b2.high[i] <= b1.high[i]) countOverlap++;
		if (countOverlap==0) return false;
	}
	return true;
	}

bigSpaceUnit Box::overlapVolume(const Box&b1,const Box &b2)
	{
	if (overlap(b1,b2)==false) return 0;

	Box overlap;
	combine(b1,b2,overlap);

	for (int i=0;i<DIMENSION;i++)
	{
		if (b1.low[i]  >= b2.low[i] && b1.low[i]  <= b2.high[i]) overlap.low[i]=b1.low[i];
		if (b1.high[i] >= b2.low[i] && b1.high[i] <= b2.high[i]) overlap.high[i]=b1.high[i];
		if (b2.low[i]  >= b1.low[i] && b2.low[i]  <= b1.high[i]) overlap.low[i]=b2.low[i];
		if (b2.high[i] >= b1.low[i] && b2.high[i] <= b1.high[i]) overlap.high[i]=b2.high[i];
	}

	return volume(overlap);
	}

// Does bigger Box encloses smaller Box completely?
bool Box::enclose (const Box &bigger,const Box &smaller)
	{
	if (enclose(bigger,smaller.low) && enclose(bigger,smaller.high)) return true;
	return false;
	}

// Is Vertex inside the Box??
bool Box::enclose (const Box &b,const Vertex &v)
{
	int i=0;
	for (i=0;i<DIMENSION;i++)
		if (b.low[i]>v[i] || b.high[i]<=v[i]) break;
	if (i==DIMENSION) return true;
	else return false;
}

// Combine 2 Axis Aligned Bounding Boxes into a bigger cube
void Box::combine (const Box &b1,const Box &b2,Box &combined)
{
	for (int i=0;i<DIMENSION;i++)
	{
		if (b1.low[i] <=b2.low[i])  combined.low[i]  = b1.low[i];  else combined.low[i]  = b2.low[i];
		if (b1.high[i]>=b2.high[i]) combined.high[i] = b1.high[i]; else combined.high[i] = b2.high[i];
	}
}

// Calculate Bounding Box over a Polyherdra
void Box::boundingBox (Box& bb,const std::vector<Vertex> &vertices)
{
	if (vertices.empty()==true)
	{
		std::cout << "Voronoi list empty";
	}
	else
	{
		bb.low = vertices.at(0);
		bb.high = vertices.at(0);
	}
	for (std::vector<Vertex>::const_iterator a = vertices.begin(); a != vertices.end(); ++a)
	{
		if (bb.low[0]>(*a)[0]) bb.low[0] = (*a)[0];
		if (bb.low[1]>(*a)[1]) bb.low[1] = (*a)[1];
		if (bb.low[2]>(*a)[2]) bb.low[2] = (*a)[2];

		if (bb.high[0]<(*a)[0]) bb.high[0] = (*a)[0];
		if (bb.high[1]<(*a)[1]) bb.high[1] = (*a)[1];
		if (bb.high[2]<(*a)[2]) bb.high[2] = (*a)[2];
	}
}

// Make a random box inside the World Box with the given percentage Volume
void Box::randomBox(const Box& world, const float percentageVolume,Box& random)
	{

	bigSpaceUnit newVolume = volume(world)*percentageVolume/100;
	spaceUnit dimensionInc = pow(newVolume,1.0/3.0) /2;

	for (size_t i = 0; i < DIMENSION; i++)
		{
		boost::uniform_real<> uni_dist (world.low[i],world.high[i]);
		boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
		float center = uni();
		random.low[i]  = center-dimensionInc;
		random.high[i] = center+dimensionInc;
		}
	}

Box Box::make_box (const std::vector<std::string> & coords)
{
    assert (coords.size () == 6);

    const float x1 = boost::lexical_cast<float> (coords[0]);
    const float y1 = boost::lexical_cast<float> (coords[1]);
    const float z1 = boost::lexical_cast<float> (coords[2]);
    const float x2 = boost::lexical_cast<float> (coords[3]);
    const float y2 = boost::lexical_cast<float> (coords[4]);
    const float z2 = boost::lexical_cast<float> (coords[5]);

    return dias::Box (dias::Vertex (x1, y1, z1), dias::Vertex (x2, y2, z2));
}

SpatialIndex::Region Box::as_region (void) const
{
    double p1[3];
    p1[0] = static_cast<float> (low[0]);
    p1[1] = static_cast<float> (low[1]);
    p1[2] = static_cast<float> (low[2]);
    double p2[3];
    p2[0] = static_cast<float> (high[0]);
    p2[1] = static_cast<float> (high[1]);
    p2[2] = static_cast<float> (high[2]);
    return SpatialIndex::Region (p1, p2, 3);
}

}

