#include "MeshObjectReader.hpp"

namespace FLAT
{
MeshObjectReader::MeshObjectReader(string location,uint64 max)
{
	const bbp::URI blue_config_filename(location);
	limit = max;
	objectType = TRIANGLE;
	objectByteSize = SpatialObjectFactory::getSize(TRIANGLE);
	objectCount = max;
	counter=0;
	try
	{
	experiment.open(blue_config_filename);
	microcircuit = &experiment.microcircuit();
	const bbp::Targets & targets = experiment.targets();
	const bbp::Cell_Target target = targets.cell_target("Column");
	microcircuit->load(target, bbp::NEURONS | bbp::MORPHOLOGIES | bbp::MESHES);
	}
	catch(...)
	{
#ifdef FATAL
		cout << "Microcircuit Failed to load";
		exit(1);
#endif
	}
	rewind();
#ifdef INFORMATION
		cout << "Microcircuit Loaded\n";
#endif
}

MeshObjectReader::MeshObjectReader(string location,uint64 max,const Box& Universe)
{
	MeshObjectReader(location,max);
	universe = Universe;
}

MeshObjectReader::~MeshObjectReader()
{
	microcircuit->unload();
}

bool MeshObjectReader::hasNext()
{
	if (counter==limit && counter>0)
	{
		objectCount = counter;
		return false;
	}
	while (trianglesIT==trianglesEnd)
	{
		neuronIT++;
		if (neuronIT==neuronEnd)
		{
			objectCount = counter;
			return false;
		}
		transformation = neuronIT->global_transform();
		const Mesh& meshes = neuronIT->morphology().mesh();
		vertices = meshes.vertices();
		trianglesIT = meshes.triangles().begin();
		trianglesEnd= meshes.triangles().end();
	}

	counter++;
	return true;
}

SpatialObject* MeshObjectReader::getNext()
{
	if (trianglesIT==trianglesEnd) cout << "Cannot read meshes\n";
	bbp::Vector_3D<bbp::Micron> v1 = transformation * vertices[*trianglesIT++];
	if (trianglesIT==trianglesEnd) cout << "Cannot read meshes\n";
	bbp::Vector_3D<bbp::Micron> v2 = transformation * vertices[*trianglesIT++];
	if (trianglesIT==trianglesEnd) cout << "Cannot read meshes\n";
	bbp::Vector_3D<bbp::Micron> v3 = transformation * vertices[*trianglesIT++];

	Triangle* mesh = new Triangle(  Vertex(v1.x(),v1.y(),v1.z()) ,
			                        Vertex(v2.x(),v2.y(),v2.z()) ,
			                        Vertex(v3.x(),v3.y(),v3.z()) );
	Box mbr = mesh->getMBR();
	if (counter==1) universe = mbr;
	else
	{
		for (int i=0;i<DIMENSION;i++)
		{
			if (mbr.high[i]>universe.high[i]) universe.high[i] = mbr.high[i];
			if (mbr.low[i]<universe.low[i])   universe.low[i] = mbr.low[i];
		}
	}

	return mesh;
}

void MeshObjectReader::rewind()
{
	Neurons& neurons = microcircuit->neurons();
	neuronIT    = neurons.begin();
	neuronEnd   = neurons.end();
	transformation = neuronIT->global_transform();
	const Mesh& meshes = neuronIT->morphology().mesh();
	vertices = meshes.vertices();
	trianglesIT = meshes.triangles().begin();
	trianglesEnd= meshes.triangles().end();
}




}
