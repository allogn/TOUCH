#ifndef DATA
#define DATA

#include "Cone.cpp"
#include "libbbpdias/TemporaryFile.hpp"
#include "libbbpdias/hilbert.hpp"

namespace dias {

/*
 * Payload Data can be used as a Leaf node
 * for spatial Indexing algorithm
 *
 * Author: Farhan Tauheed
 */
class Data
{
public:
	//Cone segment;
	Vertex midPoint;
	int id;
	//int  neuronId;
	//int  sectionId;
	//int  segmentId;

	Data()
		{
		this->id=0;
		//this->neuronId=0;
	//	this->sectionId=0;
		//this->segmentId=0;
		}
/*
	Data(const Cone &segment,int globalId)
		{
		this->segment   = segment;
		this->id 		= globalId;
		this->neuronId  = 0;
		this->sectionId = 0;
		this->segmentId = 0;
		Cone::center(segment,midPoint);
		}
*/
	Data(const Vertex &point,int globalId)
		{
		this->midPoint = point;
		this->id= globalId;
		//this->neuronId=0;
		//this->sectionId=0;
		//this->segmentId=0;
		}
/*
	Data(const Cone &segment,int globalId,int neuronId,int sectionId,int segmentId)
		{
		this->segment   = segment;
		this->id        = globalId;
		this->neuronId  = neuronId;
		this->sectionId = sectionId;
		this->segmentId = segmentId;
		Cone::center(segment,midPoint);
		}
*/
	Data&  operator=(const Data &rhs);

	bool  operator==(const Data &rhs) const;

	friend std::ostream & operator << (std::ostream & lhs,const Data & rhs);

	bool store(Farhan::TemporaryFile &tf);
	bool load(Farhan::TemporaryFile &tf);

/*	struct hilbertSort : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			double d1[3],d2[3];

			d1[0] = r1->midPoint[0];
			d1[1] = r1->midPoint[1];
			d1[2] = r1->midPoint[2];

			d2[0] = r2->midPoint[0];
			d2[1] = r2->midPoint[1];
			d2[2] = r2->midPoint[2];

			if (hilbert_ieee_cmp(3,d1,d2)>=0) return true;
			else return false;
		}
	};
*/
	struct xSortAsc : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			if (r1->midPoint[0] < r2->midPoint[0]) return true;
			else return false;
		}
	};

	struct xSortDec : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			if (r1->midPoint[0] > r2->midPoint[0]) return true;
			else return false;
		}
	};

	struct ySortAsc : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			if (r1->midPoint[1] < r2->midPoint[1]) return true;
			else return false;
		}
	};

	struct ySortDec : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			if (r1->midPoint[1] > r2->midPoint[1]) return true;
			else return false;
		}
	};

	struct zSortAsc : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			if (r1->midPoint[2] < r2->midPoint[2]) return true;
			else return false;
		}
	};

	struct zSortDec : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			if (r1->midPoint[2] > r2->midPoint[2]) return true;
			else return false;
		}
	};

	struct hilbertAsc : public std::binary_function<Data* const, Data* const, bool>
	{
		bool operator()(Data* const r1, Data* const r2)
		{
			double d1[3],d2[3];

			d1[0] = r1->midPoint[0];
			d1[1] = r1->midPoint[1];
			d1[2] = r1->midPoint[2];

			d2[0] = r2->midPoint[0];
			d2[1] = r2->midPoint[1];
			d2[2] = r2->midPoint[2];

			if (hilbert_ieee_cmp(3,d1,d2)>=0)  return true;
			else return false;
		}
	};


};

bool Data::store(Farhan::TemporaryFile &tf)
{
	try
	{
	tf.write(midPoint[0]);
	tf.write(midPoint[1]);
	tf.write(midPoint[2]);
	}
	catch(Farhan::EndOfStreamException e)
	{
		return false;
	}
	return true;
}

bool Data::load(Farhan::TemporaryFile &tf)
{
	try
	{
	midPoint[0] = tf.readFloat();
	midPoint[1] = tf.readFloat();
	midPoint[2] = tf.readFloat();
	}
	catch(Farhan::EndOfStreamException e)
	{
		return false;
	}
	return true;
}


std::ostream & operator << (std::ostream & lhs,const Data & rhs)
	{
	lhs << " Data: id=" << rhs.id << " Coordinate= " << rhs.midPoint;
	return lhs;
	}

Data&  Data::operator=(const Data &rhs)
{
	//segment = rhs.segment;
	midPoint = rhs.midPoint;
	id = rhs.id;
	//neuronId = rhs.neuronId;
	//sectionId = rhs.sectionId;
	//segmentId = rhs.segmentId;
	return *this;
}

bool  Data::operator==(const Data &rhs) const
	{
	return (id==rhs.id);
	}

}
#endif
