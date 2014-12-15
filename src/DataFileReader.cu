#include "DataFileReader.hpp"

namespace FLAT
{
	DataFileReader::DataFileReader(std::string& fileName)
	{
		try
		{
			inputFile = new BufferedFile();
			inputFile->open(fileName);
			counter=0;
			loadHeader();
		}
		catch(...)
		{
			std::cout << "Cannot load InputFile" << std::endl;
			exit(0);
		}
	}

	DataFileReader::~DataFileReader()
	{
		inputFile->close();
		delete inputFile;
	}

	void DataFileReader::loadHeader()
	{
		inputFile->seekEnd(RAW_DATA_HEADER_SIZE);
		objectType      = (SpatialObjectType)inputFile->readUInt32();
		objectCount     = inputFile->readUInt64();         // Number of Objects in Dataset
		objectByteSize  = inputFile->readUInt32();         // Size in Bytes of each Object
		inputFile->read(&universe);
		rewind();
	}

	bool DataFileReader::hasNext()
	{
		data = SpatialObjectFactory::create(objectType);

		inputFile->read(data);
		if (inputFile->eof || counter>=objectCount)
		{
			delete data;
			return false;
		}
		else
		{
			counter++;
			return true;
		}
	}

	SpatialObject* DataFileReader::getNext()
	{
		return data;
	}

	void DataFileReader::rewind()
	{
		inputFile->rewindForReading();
		counter=0;
	}
}
