#include "DataFileReader.hpp"
#include "constants.hpp"
#include "SpatialObject.hpp"
#include <SpatialIndex.h>

using namespace std;
using namespace FLAT;

	class data_stream : public SpatialIndex::IDataStream
	{
	public:

		DataFileReader* i;

		data_stream (DataFileReader* input)
		{
			i = input;
		}

		~data_stream()
		{
		}

		bool hasNext()
		{
			return i->hasNext();
		}

		uint32_t size()
		{
			abort();
		}

		void rewind()
		{
			abort();
		}

		SpatialIndex::IData* getNext()
		{
			SpatialObject * so = i->getNext();

		    double low[3], high[3];

			low[0] = so->getMBR().low[0];
			low[1] = so->getMBR().low[1];
			low[2] = so->getMBR().low[2];
			high[0] = so->getMBR().high[0];
			high[1] = so->getMBR().high[1];
			high[2] = so->getMBR().high[2];

			SpatialIndex::Region r = SpatialIndex::Region(low, high, 3);

			SpatialIndex::RTree::Data* ret = new SpatialIndex::RTree::Data(0, 0, r, 0);

			delete so;

			return ret;
		};
	};


int main(int argc, const char* argv[])
{
	///////////////////////// TEST STR INDEX AND TESSELLATION ////////////////////////////

	string inputFile  = argv[1];
	string outPutStem = argv[2];

	DataFileReader* input = new DataFileReader(inputFile);
	input->loadHeader();

	SpatialIndex::IStorageManager* rtreeStorageManager = SpatialIndex::StorageManager::createNewDiskStorageManager(outPutStem, 4096);

	SpatialIndex::id_type indexIdentifier=1;
	data_stream* ds = new data_stream(input);
	try {
		SpatialIndex::RTree::createAndBulkLoadNewRTree (
				SpatialIndex::RTree::BLM_STR,
				*ds,
				*rtreeStorageManager,
				FLAT::fill_factor, FLAT::index_node_capacity_float,
				FLAT::leaf_node_capacity_float, DIMENSION,
				SpatialIndex::RTree::RV_RSTAR,
				indexIdentifier);
	} catch (Tools::IllegalArgumentException e) {
		cout << e.what() << endl;
	}

	delete input;
	delete rtreeStorageManager;
}
