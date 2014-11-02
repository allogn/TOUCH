#ifndef METADATA_
#define METADATA_
#include "libbbpdias/Box.hpp"

using namespace dias;
using namespace std;

typedef uint32_t id;
/*
 * if the Metadata cannot fit in memory for 500M data points put on disk
 * Projected for 500Miliion points size of Metadata = 200Mb at most
 */
class MetadataEntry
{
public:
	Box pageMbr;
	Box voronoiMbr;
	set<id> pageLinks;
	id pageId;

	MetadataEntry() {};

	MetadataEntry(byte * buffer, int length) {

		voronoiMbr.low[0] = readFloat(0, buffer);
		voronoiMbr.low[1] = readFloat(sizeof(float), buffer);
		voronoiMbr.low[2] = readFloat(2*sizeof(float), buffer);

		voronoiMbr.high[0] = readFloat(3*sizeof(float), buffer);
		voronoiMbr.high[1] = readFloat(4*sizeof(float), buffer);
		voronoiMbr.high[2] = readFloat(5*sizeof(float), buffer);

		pageId = readId(6*sizeof(float), buffer);
		int count = readId(6*sizeof(float) + sizeof(id), buffer);

		for (int i=0; i<count; i++) {
			pageLinks.insert(readId(6*sizeof(float) + 2*sizeof(id) + i*sizeof(id), buffer));
		}
	}

	void MakeMetaData(VertexPage* vp)
	{
		Vertex blank;
		pageMbr.low  = vp->vertices[0];
		pageMbr.high = vp->vertices[0];

		for (int i=0;i<VERTICES_PER_PAGE;i++)
		{
			if (vp->vertices[i]==blank) continue;
			if (pageMbr.low[0]>(vp->vertices[i])[0]) pageMbr.low[0] = (vp->vertices[i])[0];
			if (pageMbr.low[1]>(vp->vertices[i])[1]) pageMbr.low[1] = (vp->vertices[i])[1];
			if (pageMbr.low[2]>(vp->vertices[i])[2]) pageMbr.low[2] = (vp->vertices[i])[2];

			if (pageMbr.high[0]<(vp->vertices[i])[0]) pageMbr.high[0] = vp->vertices[i][0];
			if (pageMbr.high[1]<(vp->vertices[i])[1]) pageMbr.high[1] = vp->vertices[i][1];
			if (pageMbr.high[2]<(vp->vertices[i])[2]) pageMbr.high[2] = vp->vertices[i][2];
		}
	}

	void serialize(byte ** buffer, int &length)
	{
		//12 float for pageMBR and voronoiMBR
		//size of unsigned int times pageLinks
		length = sizeof(float) * 6 + sizeof(int) + sizeof(id) + sizeof(id) * pageLinks.size();

		*buffer = new byte[length];

		writeFloat(voronoiMbr.low[0], 0, *buffer);
		writeFloat(voronoiMbr.low[1], sizeof(float), *buffer);
		writeFloat(voronoiMbr.low[2], 2*sizeof(float), *buffer);

		writeFloat(voronoiMbr.high[0], 3*sizeof(float), *buffer);
		writeFloat(voronoiMbr.high[1], 4*sizeof(float), *buffer);
		writeFloat(voronoiMbr.high[2], 5*sizeof(float), *buffer);

		writeId(pageId, 6*sizeof(float), *buffer);
		writeId((int)pageLinks.size(), 6*sizeof(float) + sizeof(id), *buffer);

		int i=0;

		for (set<id>::iterator it = pageLinks.begin();it!=pageLinks.end();it++) {
			writeId((id)*it, 6*sizeof(float) + 2*sizeof(id) + i*sizeof(id), *buffer);
			i++;
		}
	}

	void print()
	{
		cout << "Page ("<< pageLinks.size() << ") -> ";
		for (set<id>::iterator it = pageLinks.begin();it!=pageLinks.end();it++)
			cout << *it << " ";
		cout << "\n";
	}
private:

	void writeFloat(float f, int pos, byte* buf) {

		byte* tmp = (byte*)&f;

		for(int i=0; i<sizeof(float); i++) {
			buf[i+pos] = tmp[i];
		}
	}

	void writeId(int i, int pos, byte* buf) {

		byte* tmp = (byte*)&i;

		for(int j=0; j<sizeof(id); j++) {
			buf[j+pos] = tmp[j];
		}
	}

	int readId(int pos, byte* buf) {

		byte tmp[sizeof(id)];

		for(int i=0; i<sizeof(id); i++) {
			tmp[i] = buf[i+pos];
		}

		int val = *reinterpret_cast<int *>(tmp);

		return val;
	}

	float readFloat(int pos, byte* buf) {

		byte tmp[sizeof(float)];

		for(int i=0; i<sizeof(float); i++) {
			tmp[i] = buf[i+pos];
		}

		float val = *reinterpret_cast<float *>(tmp);

		return val;
	}

};

class MetaData
{
public:
	Farhan::BufferedFileReader* readFile;
	Farhan::BufferedFileWriter* writeFile;
	bool writeMode;
	string filename;
	int Pages;

	MetaData(string fileName,bool write)
	{
		filename = fileName;
		writeMode = write;
		if (writeMode)
		{
			writeFile = new Farhan::BufferedFileWriter(filename,Farhan::CREATE,2097152);
		}
		else
		{
			readFile  = new Farhan::BufferedFileReader(filename,2097152);
			Pages = readFile->readUInt32();
		}
	}

	~MetaData()
	{
		if (writeMode)
		{
			writeFile->close();
			delete writeFile;
		}
		else
		{
			readFile->close();
			delete readFile;
		}
	}

	void Load(MetadataEntry* metadataStructure[])
	{
		for (int i=0;i<Pages;i++)
		{
			metadataStructure[i] = new MetadataEntry();

			metadataStructure[i]->pageMbr.low[0] = readFile->readFloat();
			metadataStructure[i]->pageMbr.low[1] = readFile->readFloat();
			metadataStructure[i]->pageMbr.low[2] = readFile->readFloat();

			metadataStructure[i]->pageMbr.high[0] = readFile->readFloat();
			metadataStructure[i]->pageMbr.high[1] = readFile->readFloat();
			metadataStructure[i]->pageMbr.high[2] = readFile->readFloat();

			metadataStructure[i]->voronoiMbr.low[0] = readFile->readFloat();
			metadataStructure[i]->voronoiMbr.low[1] = readFile->readFloat();
			metadataStructure[i]->voronoiMbr.low[2] = readFile->readFloat();

			metadataStructure[i]->voronoiMbr.high[0] = readFile->readFloat();
			metadataStructure[i]->voronoiMbr.high[1] = readFile->readFloat();
			metadataStructure[i]->voronoiMbr.high[2] = readFile->readFloat();

			int links = readFile->readUInt32();
			for (int j=0;j<links;j++)
				metadataStructure[i]->pageLinks.insert(readFile->readUInt32());
		}
	}

	void Store(MetadataEntry* metadataStructure[],int pages)
	{
		writeFile->write((uint32_t)pages);
		for (int i=0;i<pages;i++)
		{
			writeFile->write(metadataStructure[i]->pageMbr.low[0]);
			writeFile->write(metadataStructure[i]->pageMbr.low[1]);
			writeFile->write(metadataStructure[i]->pageMbr.low[2]);

			writeFile->write(metadataStructure[i]->pageMbr.high[0]);
			writeFile->write(metadataStructure[i]->pageMbr.high[1]);
			writeFile->write(metadataStructure[i]->pageMbr.high[2]);

			writeFile->write(metadataStructure[i]->voronoiMbr.low[0]);
			writeFile->write(metadataStructure[i]->voronoiMbr.low[1]);
			writeFile->write(metadataStructure[i]->voronoiMbr.low[2]);

			writeFile->write(metadataStructure[i]->voronoiMbr.high[0]);
			writeFile->write(metadataStructure[i]->voronoiMbr.high[1]);
			writeFile->write(metadataStructure[i]->voronoiMbr.high[2]);

			writeFile->write((uint32_t)metadataStructure[i]->pageLinks.size());

			for (set<id>::iterator links = metadataStructure[i]->pageLinks.begin(); links != metadataStructure[i]->pageLinks.end(); links++)
				writeFile->write((uint32_t)(*links));
		}
	}
};


#endif
