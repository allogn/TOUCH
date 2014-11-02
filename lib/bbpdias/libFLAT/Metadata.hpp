#ifndef METADATA_
#define METADATA_

#include "Box.hpp"
#include <SpatialIndex.h>
#include <set>

using namespace std;
namespace FLAT
{

typedef uint32_t id;
/*
 * if the Metadata cannot fit in memory for 500M data points put on disk
 * Projected for 500Miliion points size of Metadata = 200Mb at most
 */
	class MetadataEntry
	{
	public:
		Box pageMbr;
		Box partitionMbr;
		set<id> pageLinks;
		id pageId;
#ifdef DEBUG
		int i,j,k;

#endif

		MetadataEntry();

		MetadataEntry(byte * buffer, int length);

		~MetadataEntry();

		void serialize(byte ** buffer, int &length);

		void print();

	private:

		void write(spaceUnit f, int pos, byte* buf);

		void writeId(int i, int pos, byte* buf);

		int readId(int pos, byte* buf);

		spaceUnit read(int pos, byte* buf);
	};


	class MetaDataStream : public SpatialIndex::IDataStream
	{
	public:
		unsigned i;
		uint32_t pages;
		uint32_t hopFactor;
#ifdef DEBUG
		uint32_t links;
		int frequency[100];
#endif
		vector<MetadataEntry*>* metadataStructure;

		MetaDataStream (vector<MetadataEntry*>* metadataStructure,int objectPerXBins,int objectPerYBins,int objectPerPage);

		virtual ~MetaDataStream();

		virtual bool hasNext();

		virtual uint32_t size();

		virtual void rewind();

		virtual SpatialIndex::IData* getNext();

		void GenerateLinks(uint32_t i);
	};

}

#endif
