#include <vector>
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <limits>
#include "Box.hpp"

namespace FLAT
{
	static boost::mt11213b generator (42u);

	std::ostream & operator << (std::ostream & lhs,const Box & rhs)
	{
		lhs << "Low: " << rhs.low << "  High: " << rhs.high;
		return lhs;
	}

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
			v *= distance.Vector[i];
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
		return (b.high.Vector[dimension]-b.low.Vector[dimension]);
	}

	bigSpaceUnit Box::overlapVolume(const Box&b1,const Box &b2)
	{
		if (overlap(b1,b2)==false) return 0;

		int countOverlap=0;
		Box overlap;
		combine(b1,b2,overlap);

		for (int i=0;i<DIMENSION;i++)
		{
			if (b1.low[i]  >= b2.low[i] && b1.low[i]  <= b2.high[i]) {countOverlap++;overlap.low[i]=b1.low[i];}
			if (b1.high[i] >= b2.low[i] && b1.high[i] <= b2.high[i]) {countOverlap++;overlap.high[i]=b1.high[i];}
			if (b2.low[i]  >= b1.low[i] && b2.low[i]  <= b1.high[i]) {countOverlap++;overlap.low[i]=b2.low[i];}
			if (b2.high[i] >= b1.low[i] && b2.high[i] <= b1.high[i]) {countOverlap++;overlap.high[i]=b2.high[i];}
		}

		return volume(overlap);
	}

	// Combine 2 Axis Aligned Bounding Boxes into a bigger cube without
	void Box::combine (const Box &b1,const Box &b2,Box &combined)
	{
		for (int i=0;i<DIMENSION;i++)
		{
			if (b1.low.Vector[i] <=b2.low.Vector[i])  combined.low.Vector[i]  = b1.low.Vector[i];  else combined.low.Vector[i]  = b2.low.Vector[i];
			if (b1.high.Vector[i]>=b2.high.Vector[i]) combined.high.Vector[i] = b1.high.Vector[i]; else combined.high.Vector[i] = b2.high.Vector[i];
		}
	}

	// Calculate Bounding Box over a Polyherdra
	void Box::boundingBox (Box& bb,const std::vector<Vertex> &vertices)
	{
		if (vertices.empty()==true)
		{
			std::cout << "list empty";
		}
		else
		{
			bb.low = vertices.at(0);
			bb.high = vertices.at(0);
		}
		int size= vertices.size();
		for (int i=0;i<size;i++)
		{
			for (int j=0;j<DIMENSION;j++)
			{
				if (bb.low.Vector[j]>vertices.at(i).Vector[j]) bb.low.Vector[j] = vertices.at(i).Vector[j];
				if (bb.high.Vector[j]<vertices.at(i).Vector[j]) bb.high.Vector[j] = vertices.at(i).Vector[j];
			}
		}
	}

	// Make a random box inside the World Box with the given percentage Volume
	void Box::randomBox(const Box& world, const float percentageVolume,Box& random)
	{
	bigSpaceUnit newVolume = volume(world)*percentageVolume/100;
	spaceUnit dimensionInc = pow(newVolume,1.0/3.0) /2;

	for (size_t i = 0; i < DIMENSION; i++)
		{
		boost::uniform_real<> uni_dist (world.low.Vector[i],world.high.Vector[i]);
		boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
		float center = uni();
		random.low.Vector[i]  = center-dimensionInc;
		random.high.Vector[i] = center+dimensionInc;
		}
	}

	Box Box::make_box (const std::vector<std::string> & coords)
	{
		assert (coords.size () == 6);
		Box temp;
		for (int i=0;i<DIMENSION;i++)
		{
			temp.low.Vector[i]  = boost::lexical_cast<spaceUnit> (coords[i]);
			temp.high.Vector[i] = boost::lexical_cast<spaceUnit> (coords[i+DIMENSION]);
		}
		return temp;
	}

	void Box::infiniteBox (Box& box)
	{
		for (int i=0;i<DIMENSION;i++)
		{
			box.low.Vector[i] = std::numeric_limits<spaceUnit>::min();
			box.high.Vector[i]  = std::numeric_limits<spaceUnit>::max();
		}
	}

	Box Box::operator+(const Box &rhs) const
	{
		Box temp;
		Box::combine(*this,rhs,temp);
		return temp;
	}

	Box Box::getMBR()
	{
		return *this;
	}

	Vertex Box::getCenter()
	{
		Vertex v;
		Box::center(*this,v);
		return v;
	}

	spaceUnit Box::getSortDimension(int dimension)
	{  //TODO
		//return high[dimension]+low[dimension];
		return low.Vector[dimension];
	}

	bool Box::IsResult(Box& region)
	{
		return Box::overlap(region,*this);
	}

	SpatialObjectType Box::getType()
	{
		return BOX;
	}

	void Box::serialize(uint8* buffer)
	{
		uint8* ptr = buffer;
		memcpy(ptr,&(low.Vector),DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(ptr,&(high.Vector),DIMENSION * sizeof(spaceUnit));
	}

	void Box::unserialize(uint8* buffer)
	{
		uint8* ptr = buffer;
		memcpy(&(low.Vector), ptr,DIMENSION * sizeof(spaceUnit));
		ptr += DIMENSION * sizeof(spaceUnit);
		memcpy(&(high.Vector), ptr,DIMENSION * sizeof(spaceUnit));
	}

	uint32 Box::getSize()
	{
		return 2* DIMENSION * sizeof(spaceUnit);
	}
}

