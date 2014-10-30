#ifndef SYNPASE_READER_HPP_
#define SYNAPSE_READER_HPP_

#include <BBP/Microcircuit/Microcircuit.h>
#include <BBP/Microcircuit/Experiment.h>
#include <BBP/Microcircuit/Neuron.h>
#include <BBP/Microcircuit/Synapse.h>
#include "SpatialObjectStream.hpp"
#include "DataFileReader.hpp"
#include "Synapse.hpp"

using namespace std;
using namespace bbp;

namespace FLAT
{
	/*
	 * Class Responsible for Reading Segments from BBPSDK
	 * It Reads the Header info and gives output the actual objects
	 */
	class SynapseReader :public SpatialObjectStream
	{
	public:
		uint64 limit;
		uint64 counter;

		Experiment experiment;
		Microcircuit microcircuit;

		Synapses::const_iterator synapseIT;
		Synapses::const_iterator synapseITend;

		Neurons::const_iterator neuronIT;
		Neurons::const_iterator neuronEnd;
		string location;

		SynapseReader(string location,uint64 max);

		SynapseReader(string location,uint64 max,const Box& Universe);

		~SynapseReader();

		bool hasNext();

		SpatialObject* getNext();

		void rewind();
	};
}

#endif
