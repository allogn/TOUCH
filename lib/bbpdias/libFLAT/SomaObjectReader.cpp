#include "SomaObjectReader.hpp"


namespace FLAT
{
	SomaObjectReader::SomaObjectReader(string location,uint64 max)
	{
		const bbp::URI blue_config_filename(location);
		limit = max;
		objectType = SPHERE;
		objectByteSize = SpatialObjectFactory::getSize(SPHERE);
		objectCount = max;
		counter=0;
		experiment.open(blue_config_filename);
		microcircuit = &experiment.microcircuit();
		const bbp::Targets & targets = experiment.targets();
		const bbp::Cell_Target target = targets.cell_target("Column");
		microcircuit->load(target, bbp::NEURONS | bbp::MORPHOLOGIES);
		rewind();
#ifdef INFORMATION
		cout << "Microcircuit Loaded\n";
#endif
	}

	SomaObjectReader::SomaObjectReader(string location,uint64 max,const Box& Universe)
	{
	SomaObjectReader(location,max);
		universe = Universe;
	}

	SomaObjectReader::~SomaObjectReader()
	{
		microcircuit->unload();
	}

	bool SomaObjectReader::hasNext()
	{
		if (counter==limit && counter>0)
		{
			objectCount = counter;
			return false;
		}
		if (neuronIT==neuronEnd)
		{
			objectCount = counter;
			return false;
		}
		counter++;

		return true;
	}

	SpatialObject* SomaObjectReader::getNext()
	{
		bbp::Vector_3D<bbp::Micron> center = transformation * neuronIT->soma().position();
		bbp::Micron radius = neuronIT->soma().mean_radius();
		Sphere* soma  =new Sphere (Vertex(center.x(),center.y(),center.z()),radius);
		Box mbr = soma->getMBR();
		if (counter==1) universe = mbr;
		else
		{
			for (int i=0;i<DIMENSION;i++)
			{
				if (mbr.high[i]>universe.high[i]) universe.high[i] = mbr.high[i];
				if (mbr.low[i]<universe.low[i])   universe.low[i] = mbr.low[i];
			}
		}
		neuronIT++;
		transformation = neuronIT->global_transform();

		return soma;
	}

	void SomaObjectReader::rewind()
	{
		Neurons& neurons = microcircuit->neurons();
		neuronIT    = neurons.begin();
		neuronEnd   = neurons.end();
		transformation = neuronIT->global_transform();
	}
}
