#include "PayLoad.hpp"
#include "Box.hpp"
#include <iostream>
namespace FLAT
{
	PayLoad::PayLoad()
	{

	}

	PayLoad::~PayLoad()
	{
		if (isCreated)
		{
			dataWriter->close();
			delete dataWriter;
		}
		else
		{
			dataReader->close();
			delete dataReader;
		}
	}

	void PayLoad::create(string indexFileStem,int pageSize,int objectsPerPage,int objectSize,SpatialObjectType objectType)
	{
		this->filename 		 = indexFileStem + "_payload.dat";
		this->pageSize       = pageSize;
		this->objType  		 = objectType;
		this->objectsPerPage = objectsPerPage;
		this->objectSize     = objectSize;
		this->isCreated      = true;

		dataWriter  = new BufferedFileWriter(this->filename,CREATE,PAYLOAD_BUFFERSIZE);
		dataWriter->write((uint32_t)pageSize);
		dataWriter->write((uint32_t)objType);
		dataWriter->write((uint32_t)objectsPerPage);
		dataWriter->write((uint32_t)objectSize);
#ifdef NDEBUG
		cout << "\n == PAYLOAD FILE HEADER == \n\n"
		     << "OBJECT TYPE: " << SpatialObjectFactory::getTitle(objectType) << endl
			 << "PAGE SIZE: " << pageSize << endl
			 << "OBJECT BYTE SIZE: " << objectSize <<endl
			 << "OBJECTS PER PAGE: " << objectsPerPage << endl;
#endif
		int emptyBytes = pageSize - 16; // Header size = 16 bytes

		// Write Blank Bytes for Aligning header page
		for (int i=0;i<emptyBytes;i++)
			dataWriter->write((uint8_t)0);
	}

	void PayLoad::load(string indexFileStem) // TODO Exception if problem with loading file
	{
		this->filename = indexFileStem + "_payload.dat";
		dataReader  = new BufferedFileReader(this->filename,PAYLOAD_BUFFERSIZE);

		this->pageSize = dataReader->readUInt32();
		this->objType  = (SpatialObjectType)dataReader->readUInt32();
		this->objectsPerPage = dataReader->readUInt32();
		this->objectSize     = dataReader->readUInt32();
		this->isCreated= false;
#ifdef DEBUG
//		cout << "\n == PAYLOAD FILE HEADER == \n\n"
//		     << "OBJECT TYPE: " << SpatialObjectFactory::getTitle(objType) << endl
//			 << "PAGE SIZE: " << pageSize << endl
//			 << "OBJECT BYTE SIZE: " << objectSize <<endl
//			 << "OBJECTS PER PAGE: " << objectsPerPage << endl;
#endif
	}

	bool PayLoad::putPage(vector<SpatialObject*>& itemArray)
	{
		try
		{
			if (!isCreated) return false;

			// Write counter how many vertices in page to differentiate between a partially filled page

			dataWriter->write((uint32_t)itemArray.size());
			for (vector<SpatialObject*>::iterator it = itemArray.begin();it !=itemArray.end();++it)
			{
				(*it)->write(dataWriter);
				delete (*it);
			}
			int emptyBytes = pageSize-4; // 4 bytes for counter
			emptyBytes -= (itemArray.size()*objectSize);

			// Write Blank Bytes for Partially filled Pages
			for (int i=0;i<emptyBytes;i++)
				dataWriter->write((uint8_t)0);
		}
		catch(...)
		{
			cout << "problem writing page: ";
			return false;
		}
		return true;
	}

	bool PayLoad::getPage(vector<SpatialObject*>& itemArray,int pageId)
	{
		try
		{
			if (isCreated) return false;
			uint64_t offset = (uint64_t)(pageId+1)*(uint64_t)pageSize; // +1 cause first page is Header
			dataReader->seek(offset);
			uint32_t counter = dataReader->readUInt32();

			for (uint32_t i=0;i<counter;i++)
			{
				SpatialObject* temp = SpatialObjectFactory::create(objType);
				temp->read(dataReader);
				itemArray.push_back(temp);
			}

		}
		catch(...)
		{
			cout << "problem reading page: " <<pageId << "\n";
			return false;
		}
		return true;
	}

}
