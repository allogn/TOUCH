#ifndef PAY_LOAD_HPP
#define PAY_LOAD_HPP

#include "SpatialObject.hpp"
#include <vector>
using namespace std;

#define PAYLOAD_BUFFERSIZE 2097152

namespace FLAT
{
	/*
	 * Class Responsible for Managing the Payload on our Index
	 */
	class PayLoad
	{
	public:
		BufferedFileReader* dataReader;      //Pointer to file containing data pages for writing
		BufferedFileWriter* dataWriter;      //Pointer to file containing data pages for reading
		string filename;
		int pageSize;
		int objectsPerPage;
		int objectSize;
		bool isCreated;
		SpatialObjectType objType;

		PayLoad();

		~PayLoad();

		void create(string indexFileStem,int pageSize,int objectsPerPage,int objectSize,SpatialObjectType objectType);

		void load(string indexFileStem);

		bool putPage(vector<SpatialObject*>& itemArray);

 		bool getPage(vector<SpatialObject*>& itemArray,int pageId);
	};
}

#endif
