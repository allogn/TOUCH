#ifndef SEGMENT_OBJECT_READER_HPP_
#define SEGMENT_OBJECT_READER_HPP_

#include <BBP/Microcircuit/Microcircuit.h>
#include <BBP/Microcircuit/Experiment.h>
#include <BBP/Microcircuit/Neuron.h>
#include "SpatialObjectStream.hpp"
#include "DataFileReader.hpp"
#include "Cone.hpp"

using namespace std;
using namespace bbp;

namespace FLAT
{
	/*
	 * Class Responsible for Reading Segments from BBPSDK
	 * It Reads the Header info and gives output the actual objects
	 */
	class SegmentObjectReader :public SpatialObjectStream
	{
	public:
		uint64 limit;
		uint64 counter;

		Experiment experiment;
		Microcircuit* microcircuit;

		Sections* sections;
		Segments* segments;

		Transform_3D<Micron> transformation;

		Neurons::iterator neuronIT;
		Sections::iterator sectionIT;
		Segments::const_iterator segmentIT;
		Neurons::iterator  neuronEnd;
		string location;

		SegmentObjectReader(string location,uint64 max);

		SegmentObjectReader(string location,uint64 max,const Box& Universe);

		~SegmentObjectReader();

		bool hasNext();

		SpatialObject* getNext();

		void rewind();
	};
}

#endif
