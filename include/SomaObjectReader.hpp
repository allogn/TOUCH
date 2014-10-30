#ifndef SOMA_OBJECT_READER_HPP_
#define SOMA_OBJECT_READER_HPP_

#include <BBP/Microcircuit/Microcircuit.h>
#include <BBP/Microcircuit/Experiment.h>
#include <BBP/Microcircuit/Neuron.h>
#include "SpatialObjectStream.hpp"
#include "DataFileReader.hpp"
#include "Sphere.hpp"

using namespace std;
using namespace bbp;

namespace FLAT
{
	/*
	 * Class Responsible for Reading Segments from BBPSDK
	 * It Reads the Header info and gives output the actual objects
	 */
	class SomaObjectReader :public SpatialObjectStream
	{
	public:
		uint64 limit;
		uint64 counter;

		Experiment experiment;
		Microcircuit* microcircuit;

		Transform_3D<Micron> transformation;

		Neurons::iterator neuronIT;
		Neurons::iterator  neuronEnd;
		string location;

		SomaObjectReader(string location,uint64 max);

		SomaObjectReader(string location,uint64 max,const Box& Universe);

		~SomaObjectReader();

		bool hasNext();

		SpatialObject* getNext();

		void rewind();
	};
}

#endif
