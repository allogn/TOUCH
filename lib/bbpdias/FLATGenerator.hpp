#include <cstring>
#include <vector>
#include <iomanip>
#include <SpatialIndex.h>
#include <Region.h>

#include "VertexStructure.hpp"
#include "Metadata.hpp"
#include "TileSort.cpp"
#include "SVAReader.hpp"
#include "libbtree/btree.hpp"

typedef Tools::PoolPointer<SpatialIndex::Region> RegionPtr;

#define DIM 3

class nodeSkeleton
{
public:

	SpatialIndex::id_type *m_pIdentifier;
	SpatialIndex::Region **m_ptrMBR;
	uint32_t level;
	uint32_t children;
	uint32_t nodeType;
	uint32_t *m_pDataLength;
	byte **m_pData;
	uint32_t dataLength;
	SpatialIndex::Region m_nodeMBR;

    ~nodeSkeleton() {
   		delete m_ptrMBR;
    	delete m_pDataLength;
    	delete m_pData;
    	delete m_pIdentifier;
    }
};

static void writeNode(SpatialIndex::id_type page,
		SpatialIndex::IStorageManager* m_pStorageManager,
		nodeSkeleton * n)
{
	byte* buffer = new byte[n->dataLength];;

	byte* ptr;

	ptr = buffer;

	memcpy(ptr, &n->nodeType, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(ptr, &n->level, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(ptr, &n->children, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (uint32_t u32Child = 0; u32Child < n->children; ++u32Child)
	{
		memcpy(ptr, n->m_ptrMBR[u32Child]->m_pLow, DIM * sizeof(double));
		ptr += DIM * sizeof(double);

		memcpy(ptr, n->m_ptrMBR[u32Child]->m_pHigh, DIM * sizeof(double));
		ptr += DIM * sizeof(double);

		memcpy(ptr, &(n->m_pIdentifier[u32Child]), sizeof(SpatialIndex::id_type));
		ptr += sizeof(SpatialIndex::id_type);

		memcpy(ptr, &(n->m_pDataLength[u32Child]), sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		if (n->m_pDataLength[u32Child] > 0)
		{
			memcpy(ptr, n->m_pData[u32Child], n->m_pDataLength[u32Child]);
			ptr += n->m_pDataLength[u32Child];
		}
	}

	// store the node MBR for efficiency. This increases the node size a little bit.
	memcpy(ptr, n->m_nodeMBR.m_pLow, DIM * sizeof(double));
	ptr += DIM * sizeof(double);

	//finally store that thing
	try
	{
		m_pStorageManager->storeByteArray(page, n->dataLength, buffer);
		delete[] buffer;
	}
	catch (SpatialIndex::InvalidPageException& e)
	{
		delete[] buffer;
		std::cerr << e.what() << std::endl;
		throw;
	}
}

static nodeSkeleton * readNode(SpatialIndex::id_type page,
		SpatialIndex::IStorageManager* m_pStorageManager)
{
	byte* buffer;
	byte* ptr = 0;

	nodeSkeleton * ns = new nodeSkeleton();

	SpatialIndex::Region m_infiniteRegion;
	m_infiniteRegion.makeInfinite(DIM);

	ns->m_nodeMBR = m_infiniteRegion;

	try
	{
		m_pStorageManager->loadByteArray(page, ns->dataLength, &buffer);
	}
	catch (SpatialIndex::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		throw;
	}

	try
	{
		memcpy(&ns->nodeType, buffer, sizeof(uint32_t));

		if (ns->nodeType != SpatialIndex::RTree::PersistentLeaf && ns->nodeType != SpatialIndex::RTree::PersistentIndex) return ns;

		Tools::PointerPool<SpatialIndex::Region> m_regionPool(1000);

		// skip the node type information, it is not needed.
		buffer += sizeof(uint32_t);

		memcpy(&ns->level, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);

		memcpy(&ns->children, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);

		ns->m_ptrMBR = new SpatialIndex::Region*[ns->children + 1];
		ns->m_pIdentifier = new SpatialIndex::id_type[ns->children + 1];
		ns->m_pDataLength = new uint32_t[ns->children + 1];
		ns->m_pData = new byte*[ns->children + 1];

		for (uint32_t u32Child = 0; u32Child < ns->children; ++u32Child)
		{
			ns->m_ptrMBR[u32Child] = new SpatialIndex::Region();//m_regionPool.acquire();
			*(ns->m_ptrMBR[u32Child]) = m_infiniteRegion;

//			ns->m_ptrMBR[u32Child] = m_infiniteRegion;

			memcpy(ns->m_ptrMBR[u32Child]->m_pLow, buffer, DIM * sizeof(double));
			buffer += DIM * sizeof(double);
			memcpy(ns->m_ptrMBR[u32Child]->m_pHigh, buffer, DIM * sizeof(double));
			buffer += DIM * sizeof(double);
			memcpy(&(ns->m_pIdentifier[u32Child]), buffer, sizeof(SpatialIndex::id_type));
			buffer += sizeof(SpatialIndex::id_type);

			memcpy(&(ns->m_pDataLength[u32Child]), buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			if (ns->m_pDataLength[u32Child] > 0)
			{
				ns->m_pData[u32Child] = new byte[ns->m_pDataLength[u32Child]];
				memcpy(ns->m_pData[u32Child], buffer, ns->m_pDataLength[u32Child]);
				buffer += ns->m_pDataLength[u32Child];
			}
			else
			{
				ns->m_pData[u32Child] = 0;
			}
		}

		memcpy(ns->m_nodeMBR.m_pLow, buffer, DIM * sizeof(double));
		buffer += DIM * sizeof(double);
		memcpy(ns->m_nodeMBR.m_pHigh, buffer, DIM * sizeof(double));

		buffer = ptr;

		delete[] buffer;
	}
	catch (...)
	{
		delete[] buffer;
		throw;
	}

	return ns;
}
