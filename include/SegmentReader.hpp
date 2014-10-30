#ifndef SEGMENT_READER_HPP_
#define SEGMENT_READER_HPP_

#include <BBP/Microcircuit/Microcircuit.h>
#include <BBP/Microcircuit/Experiment.h>
#include <BBP/Microcircuit/Neuron.h>
#include "SpatialObjectStream.hpp"
#include "DataFileReader.hpp"
#include "Segment.hpp"

using namespace std;
using namespace bbp;

namespace FLAT
{
	/*
	 * Class Responsible for Reading Segments from BBPSDK
	 * It Reads the Header info and gives output the actual objects
	 */
	class SegmentReader :public SpatialObjectStream
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

		SegmentReader(string location,uint64 max);

		SegmentReader(string location,uint64 max,const Box& Universe);

		~SegmentReader();

		bool hasNext();

		SpatialObject* getNext();

		void rewind();
	};
}

#endif
