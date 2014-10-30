#ifndef MESH_OBJECT_READER_HPP_
#define MESH_OBJECT_READER_HPP_

#include <BBP/Microcircuit/Microcircuit.h>
#include <BBP/Microcircuit/Experiment.h>
#include <BBP/Microcircuit/Neuron.h>
#include <BBP/Microcircuit/Mesh.h>
#include "SpatialObjectStream.hpp"
#include "DataFileReader.hpp"
#include "Triangle.hpp"

using namespace std;
using namespace bbp;

namespace FLAT
{
/*
 * Class Responsible for Reading Meshes from BBPSDK
 * It Reads the Header info and gives output the actual objects
 */
class MeshObjectReader :public SpatialObjectStream
{
public:
	uint64 limit;
	uint64 counter;

	Experiment experiment;
	Microcircuit* microcircuit;

	Transform_3D<Micron> transformation;

	Neurons::iterator neuronIT;
	Array<bbp::Vertex_Index>::const_iterator trianglesIT;
	Array<bbp::Vertex_Index>::const_iterator trianglesEnd;
	Array<bbp::Vector_3D<bbp::Micron> > vertices;
	Neurons::iterator  neuronEnd;
	string location;

	MeshObjectReader(string location,uint64 max);

	MeshObjectReader(string location,uint64 max,const Box& Universe);

	~MeshObjectReader();

	bool hasNext();

	SpatialObject* getNext();

	void rewind();
};
}

#endif
