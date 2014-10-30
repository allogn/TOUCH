#ifndef DENDRITE_READER_HPP_
#define DENDRITE_READER_HPP_

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
	class DendriteReader :public SpatialObjectStream
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

		DendriteReader(string location,uint64 max);

		DendriteReader(string location,uint64 max,const Box& Universe);

		~DendriteReader();

		bool hasNext();

		SpatialObject* getNext();

		void rewind();
	};
}

#endif
