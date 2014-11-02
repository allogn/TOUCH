#include "ExternalSort.hpp"
#include "Vertex.hpp"
#include <algorithm>

namespace FLAT
{

	ExternalSort::ExternalSort(uint32 footPrintMB, uint8 dim, SpatialObjectType objType)
	{
		this->objType = objType;
		uint32 objectSize = SpatialObjectFactory::getSize(objType);
		size = (uint32)floor( ((footPrintMB*1024*1024)+0.0)/(objectSize+0.0));
		buffer.reserve(size);
		buckets.clear();
		buffer.clear();
		sortDimension = dim;
		sorted=NULL;
		outOfCore=false;
	}

	ExternalSort::~ExternalSort()
	{
		clean();
	}

	void ExternalSort::rewind()
	{
		if (outOfCore==false)
			iter = buffer.begin();
		else
			sorted->rewind();
	}

	void ExternalSort::makeBucket()
	{
		if (sortDimension==0)
			std::sort(buffer.begin(), buffer.end(), SpatialObjectXAsc());
		else if (sortDimension==1)
			std::sort(buffer.begin(), buffer.end(), SpatialObjectYAsc());
		else std::sort(buffer.begin(), buffer.end(), SpatialObjectZAsc());

		BufferedFile* tf = new BufferedFile();
		tf->maketemporary();

		for (vector<SpatialObject*>::iterator it = buffer.begin(); it!= buffer.end(); ++it)
		{
			tf->write(*it);
			delete (*it);
		}
		buffer.clear();
		tf->rewind();
		buckets.push_back(tf);
		outOfCore=true;
	}

	void ExternalSort::insert(SpatialObject* object)
	{
		buffer.push_back(object);
		if (buffer.size() >= size)
			makeBucket();
	}

	int ExternalSort::sort()
	{
		if (outOfCore==false)
		{
			if (sortDimension==0)
				std::sort(buffer.begin(), buffer.end(), SpatialObjectXAsc());
			else if (sortDimension==1)
				std::sort(buffer.begin(), buffer.end(), SpatialObjectYAsc());
			else std::sort(buffer.begin(), buffer.end(), SpatialObjectZAsc());
			rewind();
			return buffer.size();
		}

		uint64 count = 0;
		if (buffer.empty()==false)
			makeBucket();
		sorted = new BufferedFile();
		sorted->maketemporary();

		uint32 Bsize = 0;
		Bsize = buckets.size();
		buffer.reserve(Bsize);
		uint32 eofCount = 0;
		uint32 outindex = 0;
		for (uint32 i = 0; i < Bsize; i++)
		{
			SpatialObject* temp = SpatialObjectFactory::create(objType);
			buckets.at(i)->read(temp);
			if (buckets.at(i)->eof)
			{
				delete buckets.at(i);
				buckets.at(i) = NULL;
				delete temp;
				temp = NULL;
				eofCount++;
			}
			buffer.push_back(temp);
		}

		while (eofCount < Bsize)
		{
			// set pointer to first item
			for (uint32 i = 0; i < Bsize; i++)
				if (buckets.at(i) != NULL)
				{
					outindex = i;
					break;
				}
			if (outindex == Bsize)
				break;

			// find min item
			for (uint32 i = 0; i < Bsize; i++)
				if (buckets.at(i) != NULL)
					if (buffer.at(i)->getCenter()[sortDimension] < buffer.at(outindex)->getCenter()[sortDimension])
						outindex = i;

			//write Sorted Content.
			sorted->write(buffer.at(outindex));
#ifdef PROGRESS
			count++;
			if (count%10000000==0)	cout << "SORTING: " << count << " OBJECTS DONE"<< endl;
#endif
			delete buffer.at(outindex);

			SpatialObject* temp = NULL; temp = SpatialObjectFactory::create(objType);
			buckets.at(outindex)->read(temp);
			if (buckets.at(outindex)->eof)
			{
				delete buckets.at(outindex);
				buckets.at(outindex) = NULL;
				delete temp;
				temp = NULL;
				eofCount++;
			}
			buffer.at(outindex) = temp;
		}

		for (vector<BufferedFile*>::iterator it = buckets.begin(); it
				!= buckets.end(); ++it)
			delete *it;
		buckets.clear();
		for (vector<SpatialObject*>::iterator it = buffer.begin(); it
				!= buffer.end(); ++it)
			delete *it;
		buffer.clear();

		rewind();
		return count;
	}

	SpatialObject* ExternalSort::getNext()
	{
		return next;
	}

	bool ExternalSort::hasNext()
	{
		if (outOfCore==false)
		{
			if (iter==buffer.end()) return false;
			next = (*iter);
			iter++;
			return true;
		}
		next = SpatialObjectFactory::create(objType);
		sorted->read(next);
		if (sorted->eof)
		{
			delete next;
			return false;
		}
		return true;
	}

	void ExternalSort::clean()
	{
		if (outOfCore==false)
			buffer.clear();

		if (sorted!=NULL)
		{
			delete sorted;
			sorted=NULL;
		}
	}

}
