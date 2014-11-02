#ifndef VERTEX_STRUCTURE
#define VERTEX_STRUCTURE

#include <iostream>
#include "libbbpdias/Vertex.hpp"
#include "libbbpdias/Filing.hpp"

using namespace std;
using namespace dias;

#define PAGE_SIZE 4096
#define BYTES_PER_VERTEX 12
#define VERTICES_PER_PAGE PAGE_SIZE/BYTES_PER_VERTEX
#define USED_BYTES_PER_PAGE 4092
#define UNUSED_BYTES_PER_PAGE 4

class VertexPage
{
public:
	Vertex vertices[VERTICES_PER_PAGE];
	void print()
	{
		cout << "\n\n---- Vertex Page ----\n";
		for (int i=0;i<VERTICES_PER_PAGE;i++)
			cout << vertices[i].Vector[0] << " " << vertices[i].Vector[1] << " " << vertices[i].Vector[2] << "\n";
	}
};

class VertexStructure
{
public:
	Farhan::BufferedFileReader* readFile;
	Farhan::BufferedFileWriter* writeFile;
	bool writeMode;
	string filename;

	VertexStructure(string fileName,bool write)
	{
		filename = fileName;
		writeMode = write;
		if (writeMode)
			writeFile = new Farhan::BufferedFileWriter(filename,Farhan::CREATE,2097152);
		else
			readFile  = new Farhan::BufferedFileReader(filename,2097152);
	}

	~VertexStructure()
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

	void Put(VertexPage* vp)
	{
		for (int i=0;i<VERTICES_PER_PAGE;i++)
		{
			writeFile->write((vp->vertices[i])[0]);
			writeFile->write((vp->vertices[i])[1]);
			writeFile->write((vp->vertices[i])[2]);
		}
		for (int i=0;i<UNUSED_BYTES_PER_PAGE/2;i++)
		{
			uint16_t whitespace=0;
			writeFile->write(whitespace);
		}
	}

	VertexPage* Get(int pageId)
	{
		int offset = pageId*PAGE_SIZE;
		readFile->seek(offset);
		VertexPage* vp = new VertexPage();
		for (int i=0;i<VERTICES_PER_PAGE;i++)
		{
			if (readFile->eof()) {cout << "eOF" << "\n";break;}
			(vp->vertices[i])[0] = readFile->readFloat();
			if (readFile->eof()) {cout << "eOF" << "\n";break;}
			(vp->vertices[i])[1] = readFile->readFloat();
			if (readFile->eof()) {cout << "eOF" << "\n";break;}
			(vp->vertices[i])[2] = readFile->readFloat();
			if (readFile->eof()) {cout << "eOF" << "\n";break;}
		}
		return vp;
	}

};
#endif
