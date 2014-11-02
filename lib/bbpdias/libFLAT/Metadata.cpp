#include "Metadata.hpp"
#include "Box.hpp"
#include <set>

using namespace std;
namespace FLAT {

	MetadataEntry::MetadataEntry()
	{
	}

	MetadataEntry::MetadataEntry(byte * buffer, int length)
	{
		partitionMbr.low[0]  = read(0*sizeof(spaceUnit), buffer);
		partitionMbr.low[1]  = read(1*sizeof(spaceUnit), buffer);
		partitionMbr.low[2]  = read(2*sizeof(spaceUnit), buffer);
		partitionMbr.high[0] = read(3*sizeof(spaceUnit), buffer);
		partitionMbr.high[1] = read(4*sizeof(spaceUnit), buffer);
		partitionMbr.high[2] = read(5*sizeof(spaceUnit), buffer);

		pageId = readId(6*sizeof(spaceUnit), buffer);
		int count = readId(6*sizeof(spaceUnit) + sizeof(id), buffer);

		for (int i=0; i<count; i++)
		{
			pageLinks.insert(readId(6*sizeof(spaceUnit) + 2*sizeof(id) + i*sizeof(id), buffer));
		}
	}

	MetadataEntry::~MetadataEntry()
	{
		pageLinks.clear();
	}

	void MetadataEntry::serialize(byte ** buffer, int &length)
	{
		//12 float for pageMBR and partitionMbr
		//size of unsigned int times pageLinks
		length = sizeof(spaceUnit) * 6 + sizeof(spaceUnit) + sizeof(id) + sizeof(id) * pageLinks.size();

		*buffer = new byte[length];

		write(partitionMbr.low[0],  0*sizeof(spaceUnit), *buffer);
		write(partitionMbr.low[1],  1*sizeof(spaceUnit), *buffer);
		write(partitionMbr.low[2],  2*sizeof(spaceUnit), *buffer);
		write(partitionMbr.high[0], 3*sizeof(spaceUnit), *buffer);
		write(partitionMbr.high[1], 4*sizeof(spaceUnit), *buffer);
		write(partitionMbr.high[2], 5*sizeof(spaceUnit), *buffer);

		writeId(pageId, 6*sizeof(spaceUnit), *buffer);
		writeId((int)pageLinks.size(), 6*sizeof(spaceUnit) + sizeof(id), *buffer);

		int i=0;

		for (set<id>::iterator it = pageLinks.begin();it!=pageLinks.end();it++)
		{
			writeId((id)*it, 6*sizeof(spaceUnit) + 2*sizeof(id) + i*sizeof(id), *buffer);
			i++;
		}
	}

	void MetadataEntry::print()
	{
		cout << "Page ("<< pageLinks.size() << ") -> ";
		for (set<id>::iterator it = pageLinks.begin();it!=pageLinks.end();it++)
			cout << *it << " ";
		cout << "\n";
	}

	void MetadataEntry::write(spaceUnit f, int pos, byte* buf)
	{
		byte* tmp = (byte*)&f;

		for(unsigned i=0; i<sizeof(spaceUnit); i++)
		{
			buf[i+pos] = tmp[i];
		}
	}

	void MetadataEntry::writeId(int i, int pos, byte* buf)
	{
		byte* tmp = (byte*)&i;

		for(unsigned j=0; j<sizeof(id); j++)
		{
			buf[j+pos] = tmp[j];
		}
	}

	int MetadataEntry::readId(int pos, byte* buf)
	{
		byte tmp[sizeof(id)];

		for(unsigned i=0; i<sizeof(id); i++) {
			tmp[i] = buf[i+pos];
		}

		int val = *reinterpret_cast<int *>(tmp);

		return val;
	}

	spaceUnit MetadataEntry::read(int pos, byte* buf)
	{
		byte tmp[sizeof(spaceUnit)];

		for(unsigned i=0; i<sizeof(spaceUnit); i++)
		{
			tmp[i] = buf[i+pos];
		}

		spaceUnit val = *reinterpret_cast<spaceUnit *>(tmp);

		return val;
	}

	MetaDataStream::MetaDataStream (vector<MetadataEntry*>* metadataStructure,int objectPerXBins,int objectPerYBins,int objectPerPage)
	{
		i=0;
		this->metadataStructure = metadataStructure;
		pages = metadataStructure->size();
		hopFactor = (uint32_t)((floor( (objectPerXBins+0.0) / (objectPerYBins+0.0)) *
				         ceil ( (objectPerYBins+0.0) / (objectPerPage+0.0))) +
				         ceil ( ((objectPerXBins%objectPerYBins)+0.0) / (objectPerPage+0.0)));
#ifdef DEBUG
		links=0;
		for (int i=0;i<100;i++)
			frequency[i]=0;
#endif

	}

	MetaDataStream::~MetaDataStream()
	{
	}

	bool MetaDataStream::hasNext()
	{
		return (i < pages);
	}

	uint32_t MetaDataStream::size()
	{
		return pages;
	}

	void MetaDataStream::rewind()
	{
		i=0;
	}

	void MetaDataStream::GenerateLinks(uint32_t i)
	{
		for (uint32_t j=i+1;j<pages;++j)
		{
			if (metadataStructure->at(i)->partitionMbr.high[0] < metadataStructure->at(j)->partitionMbr.low[0])
				break;

			if ((metadataStructure->at(i)->partitionMbr.high[1] < metadataStructure->at(j)->partitionMbr.low[1]))
			{
				uint32_t nextHop = ((j/hopFactor)+1)*hopFactor;
				if (nextHop < pages)
					j = nextHop;
			}

			if (Box::overlap( metadataStructure->at(i)->partitionMbr , metadataStructure->at(j)->partitionMbr ))
			{
				metadataStructure->at(i)->pageLinks.insert(j);
				metadataStructure->at(j)->pageLinks.insert(i);
#ifdef DEBUG
				links+=2;
#endif
			}
		}
#ifdef DEBUG
		if (metadataStructure->at(i)->pageLinks.size()<100)
			frequency[metadataStructure->at(i)->pageLinks.size()]++;
#endif
	}

	SpatialIndex::IData* MetaDataStream::getNext()
	{
		MetadataEntry* me = metadataStructure->at(i);

		GenerateLinks(i);

		if (me->pageId != i) cout << "Something wrong";

	    byte * buffer;
	    int length;
	    me->serialize(&buffer, length);

	    double low[3], high[3];

		low[0] = me->pageMbr.low[0];
		low[1] = me->pageMbr.low[1];
		low[2] = me->pageMbr.low[2];
		high[0] = me->pageMbr.high[0];
		high[1] = me->pageMbr.high[1];
		high[2] = me->pageMbr.high[2];

		SpatialIndex::Region r = SpatialIndex::Region(low, high, 3);
		SpatialIndex::RTree::Data* ret = new SpatialIndex::RTree::Data(length, buffer, r, i);
		i++;
#ifdef PROGRESS
			if (i%100000==0) cout << "INDUCING LINKS: "<< i << " PAGES DONE" << endl;
#endif
		delete[] buffer;
		delete me;

		return ret;
	}



}

