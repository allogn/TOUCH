#include "RtreeIndex.hpp"

namespace FLAT
{
	rtree_stream::rtree_stream (SpatialObjectStream* input)
	{
		i = input;
	}

	rtree_stream::~rtree_stream()
	{
	}

	bool rtree_stream::hasNext()
	{
		return i->hasNext();
	}

	uint32_t rtree_stream::size()
	{
		abort();
	}

	void rtree_stream::rewind()
	{
		i->rewind();
	}

	SpatialIndex::IData* rtree_stream::getNext()
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


	RtreeIndex::RtreeIndex()
	{
		rtreeStorageManager = NULL;
		tree =  NULL;
	}

	RtreeIndex::~RtreeIndex()
	{
		delete rtreeStorageManager;
	}

	void RtreeIndex::buildIndex(SpatialObjectStream* input,string indexFileStem)
	{
		rtree_stream* ds = new rtree_stream(input);
		SpatialIndex::id_type indexIdentifier=1;
		rtreeStorageManager = SpatialIndex::StorageManager::createNewDiskStorageManager(indexFileStem, default_page_size);
		try
		{
			cout << "NODE FANOUT=" << index_node_capacity_double << " LEAF FANOUT="<< leaf_node_capacity_double<<endl;
			tree = SpatialIndex::RTree::createAndBulkLoadNewRTree
					(
					SpatialIndex::RTree::BLM_STR,
					*ds,
					*rtreeStorageManager,
					FLAT::fill_factor, FLAT::index_node_capacity_double,
					FLAT::leaf_node_capacity_double, DIMENSION,
					SpatialIndex::RTree::RV_RSTAR,
					indexIdentifier
					);
		}
		catch (Tools::IllegalArgumentException e)
		{
			cout << e.what() << endl;
		}
		delete ds;
	}

	void RtreeIndex::loadIndex(string indexFileStem)
	{

	}
}

