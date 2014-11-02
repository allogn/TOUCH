#include "FLATGenerator.hpp"


#define MEMORY_MAPPING // in memory mapping of vertices to page id . turn off to use Btree prohibitively slow

using namespace dias;
using namespace std;

#ifdef MEMORY_MAPPING
id idMap[250000000];
#endif


class data_stream : public SpatialIndex::IDataStream
{
public:
	int currentMD;
	int pages;
	MetadataEntry* md;

	data_stream (MetadataEntry* metadataStructure, int Pages)
	{
		currentMD=0;
		pages = Pages;
		md = metadataStructure;
	}

	virtual ~data_stream()
	{
		delete md;
	}

	virtual bool hasNext()
	{
		return (currentMD < pages);
	}

	virtual uint32_t size()
	{
		abort(); // FIXME
	}

	virtual void rewind()
	{
		abort(); // FIXME
	}

	virtual SpatialIndex::IData* getNext()
	{
		md[currentMD].pageId = currentMD;
	    byte * buffer;
	    int length;
	    md[currentMD].serialize(&buffer, length);

	    double low[3], high[3];

		low[0] = md[currentMD].pageMbr.low[0];
		low[1] = md[currentMD].pageMbr.low[1];
		low[2] = md[currentMD].pageMbr.low[2];
		high[0] = md[currentMD].pageMbr.high[0];
		high[1] = md[currentMD].pageMbr.high[1];
		high[2] = md[currentMD].pageMbr.high[2];

		SpatialIndex::Region r = SpatialIndex::Region(low, high, 3);

		SpatialIndex::RTree::Data* ret = new SpatialIndex::RTree::Data(length, buffer, r, currentMD);

		currentMD++;

		return ret;
	}
};

int main(int argc, const char* argv[])
{
	/********************** ARGUMENTS ***********************/
	string inputFile  = argv[1];
	string outputStem = argv[2];
	string seedFile   = outputStem + "_seed";
	string vertexFile = outputStem + "_vertex.dat";
	string metaFile   = outputStem + "_meta.dat";
	string tempFile	  = outputStem + "_temp";
	cout << fixed << showpoint << setprecision(1);

	/********************* TILE SORT *************************/
	cout << "Starting First Pass: Tile Sorting\n";

	SVAReader* reader = new SVAReader(inputFile);
	TileSort* tiler = new TileSort(500*7000,VERTICES_PER_PAGE);
        while (reader->hasNext())
	{
		tiler->Insert(reader->getNext());
	}
	int Objects = tiler->sortTile();
	delete reader;

	cout << "First Pass Complete: " << Objects << "\n\n";

	/******************* CREATE STRUCTURES ******************/
	int Pages = ceil((Objects+0.0) / (VERTICES_PER_PAGE+0.0));
	MetadataEntry*   metadataStructure;
	metadataStructure = new MetadataEntry[Pages];
	VertexStructure* vertexStructure = new VertexStructure(vertexFile,true);
#ifndef MEMORY_MAPPING
	 boost::shared_ptr<SpatialIndex::IStorageManager> tmp_btree_file (SpatialIndex::StorageManager::createNewDiskStorageManager (tempFile, PAGE_SIZE));
	 vertex_to_page_id_map* btree = new vertex_to_page_id_map (tmp_btree_file,PAGE_SIZE);
#endif

	/******************** MAKE VERTEX PAGES ********************/
	cout << "Starting Second Pass: Making Vertices Pages\n";
	for (int pageId=0;pageId<Pages;pageId++)
	{
		VertexPage* vp = new VertexPage();

		for (int i=0;i<VERTICES_PER_PAGE;i++)
			if (tiler->hasNext())
			{
				Vertex_info* vinfo = tiler->getNext();
				vp->vertices[i].Vector[0] = vinfo->vi.coords[0];
				vp->vertices[i].Vector[1] = vinfo->vi.coords[1];
				vp->vertices[i].Vector[2] = vinfo->vi.coords[2];
#ifdef MEMORY_MAPPING
				idMap[vinfo->vid] = pageId;
#else
				btree->insert(vinfo->vid,pageId);
#endif

				delete vinfo;
			}
		vertexStructure->Put(vp);
		metadataStructure[pageId].MakeMetaData(vp);

		if (pageId%100000==0 && pageId!=0) cout << "Page Processed : " << pageId << " Out of " << Pages <<"\n";
		delete vp;
	}

        delete vertexStructure;
	cout << "Second Pass Complete\n\n";

	/******************** MAKE META DATA STRUCTURE ********************/
	cout << "Starting Third Pass: Replacing Vertex Ids with Page Ids\n";
	tiler->Rewind();

	for (id pageId=0;pageId<Pages;pageId++)
	{
		vector<Vertex> pageVoronoiMBR;
		for (int i=0;i<VERTICES_PER_PAGE;i++)
			if (tiler->hasNext())
			{
				Vertex_info* vinfo = tiler->getNext();
				Box voronoiBoundingBox = vinfo->vi.voronoi_mbr.as_box();
				pageVoronoiMBR.push_back(voronoiBoundingBox.low);
				pageVoronoiMBR.push_back(voronoiBoundingBox.high);
				for (vector<vertex_id>::iterator it = vinfo->vi.neighbours.begin();it != vinfo->vi.neighbours.end();it++)
				{
#ifdef MEMORY_MAPPING
					id linkPageId = idMap[*it];
#else
			        dias::map_query_result queryResult = btree->range_query (std::make_pair (*it, *it));
					id linkPageId = queryResult[0].second;
#endif
					if (linkPageId!= pageId) {
						metadataStructure[pageId].pageLinks.insert(linkPageId);
					}
				}
				delete vinfo;
			}
		Box::boundingBox(metadataStructure[pageId].voronoiMbr,pageVoronoiMBR);

		if (pageId%100000==0 && pageId!=0) cout << "Page Processed : " << pageId << " Out of " << Pages <<"\n";
	}

	cout << "Third Pass Complete: " << Pages << "\n\n";
	delete tiler;









//	for (int pageId=0;pageId<Pages;pageId++) {
////		links += metadataStructure[pageId]->pageLinks.size();
//		metadataStructure[pageId].pageLinks.clear();
//	}
//
////	long overlapping = 0;
//	long emptybb = 0;
//
//	for(int i=0; i<Pages; i++) {
//		for(int j=i+1; j<Pages; j++) {
//
//			if(Box::overlap(metadataStructure[i].pageMbr, metadataStructure[j].pageMbr)) {
//				metadataStructure[i].pageLinks.insert(j);
//				metadataStructure[j].pageLinks.insert(i);
////				overlapping++;
//			}
////			} else {
////				Box combined;
////				Box::combine(metadataStructure[i]->pageMbr, metadataStructure[j]->pageMbr, combined);
////				long ov = 0;
////				for(int k=0; k<Pages; k++) {
////					if(Box::overlap(metadataStructure[k]->pageMbr, combined)) {
////						ov++;
////					}
////				}
////
////				if(ov == 2) {
////					metadataStructure[i]->pageLinks.insert(j);
////					metadataStructure[js]->pageLinks.insert(i);
////					emptybb++;
////				}
////			}
//		}
//	}
//
//	ofstream out("test2");
//	for (int pageId=0;pageId<Pages;pageId++) {
//		out << metadataStructure[pageId].pageLinks.size() << endl;
//	}
//	out.close();








	SpatialIndex::IStorageManager* rtreeStorageManager = SpatialIndex::StorageManager::createNewDiskStorageManager(seedFile, PAGE_SIZE);

	SpatialIndex::id_type indexIdentifier=1;
	data_stream* ds = new data_stream(metadataStructure,Pages);
	SpatialIndex::ISpatialIndex *seedTree = SpatialIndex::RTree::createAndBulkLoadNewRTree (
	        SpatialIndex::RTree::BLM_STR,
	        *ds,
	        *rtreeStorageManager,
	        dias::fill_factor, dias::index_node_capacity,
	        37, 3,
	        SpatialIndex::RTree::RV_RSTAR,
	        indexIdentifier);

	cout << "Forth Pass Complete\n\n";

	/******************* DELETE STRUCTURES ******************/
	//delete ds;

	vector<SpatialIndex::id_type> keys;

	rtreeStorageManager->getKeys(&keys);

	map<uint32_t,SpatialIndex::id_type> mapping;

	for (vector<SpatialIndex::id_type>::iterator iter = keys.begin(); iter != keys.end(); ++iter) {

		SpatialIndex::Region m_nodeMBR;

		SpatialIndex::id_type id = *iter;
		nodeSkeleton * nss = readNode(id, rtreeStorageManager);

		if (nss->nodeType == SpatialIndex::RTree::PersistentLeaf) {
			for (int i = 0; i < nss->children; i++) {
				MetadataEntry m = MetadataEntry(nss->m_pData[i], nss->m_pDataLength[i]);
				mapping.insert (pair<uint32_t,SpatialIndex::id_type>(m.pageId, *iter));
			}
		}
	}

	cout << "Mapping done\n\n";

	for (vector<SpatialIndex::id_type>::iterator iter = keys.begin(); iter != keys.end(); ++iter) {

		SpatialIndex::Region m_nodeMBR;

		SpatialIndex::id_type id = *iter;
		nodeSkeleton * nss = readNode(id, rtreeStorageManager);

		if (nss->nodeType == SpatialIndex::RTree::PersistentLeaf) {
			for(int i=0; i<nss->children; i++) {
				MetadataEntry m = MetadataEntry(nss->m_pData[i], nss->m_pDataLength[i]);
				set<uint32_t> updatedLinks;

				for(set<uint32_t>::iterator it = m.pageLinks.begin(); it != m.pageLinks.end(); ++it) {
					map<uint32_t,SpatialIndex::id_type>::iterator res = mapping.find(*it);
					if(res != mapping.end()) {
						updatedLinks.insert(res->second);
					} else {
						cout << "Unable to find mapping for " << *it << endl;
					}
				}

//				if(m.pageLinks.size() != updatedLinks.size()) {
//					cout << "Unequal sizes: " << m.pageLinks.size() << " :: " << updatedLinks.size() <<  endl;
//				}

				m.pageLinks.swap(updatedLinks);
				int length = nss->m_pDataLength[i];
				m.serialize(&nss->m_pData[i], length);
			}

			writeNode(id, rtreeStorageManager, nss);
		}
	}

	cout << "Done: " << Pages << "\n\n";

	delete rtreeStorageManager;
}
