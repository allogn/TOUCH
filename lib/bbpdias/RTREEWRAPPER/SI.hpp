#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Containers/Neurons.h>
#include <BBP/Model/Microcircuit/Containers/Sections.h>
#include <BBP/Model/Microcircuit/Containers/Segments.h>

#include <SpatialIndex.h>

#include "bbp_tools.hpp"

using namespace SpatialIndex;
using namespace bbp;
using namespace std;

#ifndef SPATIALINDEXWRAPPER_HPP_
#define SPATIALINDEXWRAPPER_HPP_

class SI {
public:
   static void indexData(std::string source, std::string destination);
   static SI* loadIndex(std::string index);
   static std::vector<string> rangeQuery(SI* si,float xlo,float ylo,float zlo,float xhi,float yhi,float zhi);

	static void writeInteger(int i, int pos, byte* buf) {

		byte* tmp = (byte*)&i;

		for(int j=0; j<sizeof(int); j++) {
			buf[j+pos] = tmp[j];
		}
	}

	static int readInteger(int pos, byte* buf) {

		byte tmp[sizeof(int)];

		for(int i=0; i<sizeof(int); i++) {
			tmp[i] = buf[i+pos];
		}

		int val = *reinterpret_cast<int *>(tmp);

		return val;
	}

private:
   SI(ISpatialIndex * tree);
   ISpatialIndex *_tree;
};

#endif /* SPATIALINDEXWRAPPER_HPP_ */
