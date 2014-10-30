#ifndef EXTERNAL_SORT_HPP_
#define EXTERNAL_SORT_HPP_

#include "SpatialObject.hpp"
#include "BufferedFile.hpp"
#include "SpatialObjectStream.hpp"
#include <vector>
#define SORTFILE_BUFFERSIZE 2097152

using namespace std;
namespace FLAT
{
/*
 * External Sort on 3D objects based on Center
 * Don't use in multi thread implementation..
 *
 * Usage: make ExternalSort object
 *        call insert() to fill the sorter
 *        when done call sort() to do the dirty work
 *        use hasNext() and getNext() to iterate over the sorted data
 *        use clean() to reuse the Sorter by freeing all memory
 */
	class ExternalSort :public SpatialObjectStream
	{
	public:
		uint32 maxObjectsInMemory;
		vector<BufferedFile*> buckets;
		vector<SpatialObject*> buffer;
		BufferedFile* sorted;
		SpatialObject* next;
		uint8 sortDimension;
		vector<SpatialObject*>::iterator iter;
		bool outOfCore;

		ExternalSort(uint32 footPrintMB, uint8 dim, SpatialObjectType objType);
		~ExternalSort();

		void rewind();

		void makeBucket();

		void insert(SpatialObject* object);

		int sort();

		SpatialObject* getNext();

		bool hasNext();

		void clean();

		SpatialObject* at(int i);

	};
}
#endif
