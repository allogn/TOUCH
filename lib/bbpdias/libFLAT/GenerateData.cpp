#include "DataFileReader.hpp"
#include "SegmentObjectReader.hpp"
#include "MeshObjectReader.hpp"
#include "SomaObjectReader.hpp"


using namespace std;
using namespace FLAT;

int main(int argc, const char* argv[])
{
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	if (argc<5) {cout << "Error incorrect arguments";exit(1);}

	string location = argv[1];  // location including BlueConfig file of BBP Circuit
	string limit    = argv[2];  // MAX objects to include in Data file .. 0 means all objects
	string dataset  = argv[3];  // SEGMENT / SOMA / MESH
	string dataType = argv[4];  // OBJECT / MBR / CENTER
	uint64 max = atol(limit.c_str());

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	SpatialObjectStream* stream;
	if 		(stricmp(dataset.c_str(),"segment")==0)
		stream = new SegmentObjectReader(location,max);
	else if (stricmp(dataset.c_str(),"mesh")==0)
		stream = new MeshObjectReader(location,max);
	else if (stricmp(dataset.c_str(),"soma")==0)
		stream = new SomaObjectReader(location,max);
	else
	{
		cout << "Unrecognized dataset\n";
		exit(1);
	}

	stringstream filename;
	if (max>0)
		filename << "RawData-" << dataset << "-" << dataType << "-"<< limit << ".bin";
	else
		filename << "RawData-" << dataset << "-" << dataType << "-All" << ".bin";

	BufferedFile* file = new BufferedFile();
	file->create(filename.str());

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	while (stream->hasNext())
	{
		SpatialObject * sobj = stream->getNext();

		if (stricmp(dataType.c_str(),"object")==0)
			file->write( sobj );
		else if (stricmp(dataType.c_str(),"mbr")==0)
			file->write( &sobj->getMBR() );
		else if (stricmp(dataType.c_str(),"center")==0)
			file->write( &sobj->getCenter() );
		else
		{
			cout << "Unrecognized dataType\n";
			exit(1);
		}

		if (stricmp(dataType.c_str(),"object")==0)
		{
			if 		(stricmp(dataset.c_str(),"segment")==0)
				cout << *((Cone*)sobj) << endl;
			else if (stricmp(dataset.c_str(),"mesh")==0)
				cout << *((FLAT::Triangle*)sobj) << endl;
			else
				cout << *((FLAT::Sphere*)sobj) << endl;
		}
		else if (stricmp(dataType.c_str(),"mbr")==0)
			cout << *((FLAT::Box*)&sobj->getMBR()) << endl;
		else
			cout << *((Vertex*)&sobj->getCenter()) << endl;

		delete sobj;
	}

	file->writeUInt32((uint32)stream->objectType);
	file->writeUInt64(stream->objectCount);
	file->writeUInt32(stream->objectByteSize);
	file->write(&(stream->universe));

	cout << "\nFile Saved: " << file->filename << endl
		 << "Object Type: " << SpatialObjectFactory::getTitle(stream->objectType) << endl
		 << "Total Objects: " << stream->objectCount << endl
		 << "Object Byte Size: " << stream->objectByteSize <<endl
		 << "Universe Bounds: " << stream->universe << endl;

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	delete file;
	delete stream;
}

