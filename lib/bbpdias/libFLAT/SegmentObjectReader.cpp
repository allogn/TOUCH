#include "SegmentObjectReader.hpp"


namespace FLAT
{
	SegmentObjectReader::SegmentObjectReader(string location,uint64 max)
	{
		const bbp::URI blue_config_filename(location);
		limit = max;
		objectType = CONE;
		objectByteSize = SpatialObjectFactory::getSize(CONE);
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

	SegmentObjectReader::SegmentObjectReader(string location,uint64 max,const Box& Universe)
	{
		SegmentObjectReader(location,max);
		universe = Universe;
	}

	SegmentObjectReader::~SegmentObjectReader()
	{
		microcircuit->unload();
		delete segments;
		delete sections;
	}

	bool SegmentObjectReader::hasNext()
	{
		if (counter==limit && counter>0)
		{
			objectCount = counter;
			return false;
		}

		while (segmentIT==segments->end())
		{
			sectionIT++;
			while (sectionIT==sections->end())
			{
				neuronIT++;
				if (neuronIT==neuronEnd)
				{
					objectCount = counter;
					return false;
				}
				transformation = neuronIT->global_transform();
				delete sections;
				sections = new Sections(neuronIT->morphology().all_sections());
				sectionIT    = sections->begin();
			}
			delete segments;
			segments = new Segments(sectionIT->segments());
			segmentIT    = segments->begin();
		}
		counter++;

		return true;
	}

	SpatialObject* SegmentObjectReader::getNext()
	{
		bbp::Vector_3D<bbp::Micron> gbegin = transformation * segmentIT->begin().center();
		bbp::Vector_3D<bbp::Micron> gend   = transformation * segmentIT->end().center();
		Vertex begin = Vertex(gbegin.x(),gbegin.y(),gbegin.z());
		Vertex end   = Vertex(gend.x(),gend.y(),gend.z());
		Cone* segment = new Cone(begin,end,segmentIT->begin().radius(),segmentIT->end().radius());
		Box mbr = segment->getMBR();
		if (counter==1) universe = mbr;
		else
		{
			for (int i=0;i<DIMENSION;i++)
			{
				if (mbr.high[i]>universe.high[i]) universe.high[i] = mbr.high[i];
				if (mbr.low[i]<universe.low[i])   universe.low[i] = mbr.low[i];
			}
		}
		segmentIT++;
		return segment;
	}

	void SegmentObjectReader::rewind()
	{
		Neurons& neurons = microcircuit->neurons();
		neuronIT    = neurons.begin();
		neuronEnd   = neurons.end();
		transformation = neuronIT->global_transform();
		sections = new Sections(neuronIT->morphology().all_sections());
		sectionIT    = sections->begin();
		segments = new Segments(sectionIT->segments());
		segmentIT    = segments->begin();
	}
}
