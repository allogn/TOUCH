#include <cstring>
#include <vector>
#include <iomanip>
#include <SpatialIndex.h>

#include "VertexStructure.hpp"
#include "Metadata.hpp"
#include "TileSort.cpp"
#include "SVAReader.hpp"
#include "libbtree/btree.hpp"

#define MEMORY_MAPPING // in memory mapping of vertices to page id . turn off to use Btree prohibitively slow

using namespace dias;
using namespace std;

#ifdef MEMORY_MAPPING
id idMap[250000000];
#endif

class data_stream : public SpatialIndex::IDataStream
{
public:
	VertexStructure* vertexStructure;
	int pagePointer;
	VertexPage* vp;
	int currentPage;
	int pages;

	data_stream (string vertexFile,int Pages)
	{
		currentPage=0;
		pagePointer=0;
		pages = Pages;
		vertexStructure = new VertexStructure(vertexFile,false);
		vp = vertexStructure->Get(currentPage);
	}

	virtual ~data_stream()
	{
		delete vp;
		delete vertexStructure;
	}

	virtual bool hasNext()
	{
		if (pagePointer>=VERTICES_PER_PAGE)
		{
			currentPage++;
			delete vp;
			pagePointer=0;
			if (currentPage>=pages) return false;
			vp = vertexStructure->Get(currentPage);
		}
		Vertex blank;
		if (vp->vertices[pagePointer]==blank) return false;
		else return true;
	}

	virtual size_t size()
	{
		cout << "Called SIZE ABORT!!";
		abort(); // FIXME
	}

	virtual void rewind()
	{
		cout << "Called REWIND ABORT!!";
		abort(); // FIXME
	}

	virtual SpatialIndex::IData* getNext()
	{
		double lo[3];
		Vertex blank;
		if (currentPage>=pages) cout << "Current Page: " << currentPage << " of Total: " << pages << " Vertex number: " << pagePointer << "\n";
		if ( vp->vertices[pagePointer]==blank) cout << "Blank Vertex Page: " << currentPage << " of Total: " << pages << " Vertex number: " << pagePointer << "\n";

		lo[0] = vp->vertices[pagePointer].Vector[0];
		lo[1] = vp->vertices[pagePointer].Vector[1];
		lo[2] = vp->vertices[pagePointer].Vector[2];
		pagePointer++;
		SpatialIndex::Region mbr = SpatialIndex::Region(lo,lo,3);
		SpatialIndex::RTree::Data* ret = new SpatialIndex::RTree::Data(0, 0, mbr, currentPage);
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
	MetadataEntry*   metadataStructure [Pages];
	VertexStructure* vertexStructure = new VertexStructure(vertexFile,true);
#ifndef MEMORY_MAPPING
	 boost::shared_ptr<SpatialIndex::IStorageManager> tmp_btree_file (SpatialIndex::StorageManager::createNewDiskStorageManager (tempFile, PAGE_SIZE));
	 vertex_to_page_id_map* btree = new vertex_to_page_id_map (tmp_btree_file,PAGE_SIZE);
#endif
   SpatialIndex::IStorageManager* rtreeStorageManager = SpatialIndex::StorageManager::createNewDiskStorageManager(seedFile, PAGE_SIZE);

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
		metadataStructure[pageId] = new MetadataEntry();
		metadataStructure[pageId]->MakeMetaData(vp);

		if (pageId%100000==0 && pageId!=0) cout << "Page Processed : " << pageId << " Out of " << Pages <<"\n";
		delete vp;
	}

        delete vertexStructure;
	cout << "Second Pass Complete\n\n";

	/******************** MAKE META DATA STRUCTURE ********************/
	cout << "Starting Third Pass: Replacing Vertex Ids with Page Ids\n";
	tiler->Rewind();

	for (int pageId=0;pageId<Pages;pageId++)
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
					if (linkPageId!= pageId)
						metadataStructure[pageId]->pageLinks.insert(linkPageId);
				}
				delete vinfo;
			}
		Box::boundingBox(metadataStructure[pageId]->voronoiMbr,pageVoronoiMBR);

		if (pageId%100000==0 && pageId!=0) cout << "Page Processed : " << pageId << " Out of " << Pages <<"\n";
	}

	MetaData* metadata = new MetaData(metaFile,true);
	metadata->Store(metadataStructure,Pages);
	delete metadata;

	cout << "Third Pass Complete:\n\n";
	delete tiler;

	/*********************** MAKE SEEDING INDEX ***********************/
	cout << "Starting Forth Pass: Making Seed RTREE based on STR\n";
	SpatialIndex::id_type indexIdentifier=0;
	data_stream* ds = new data_stream(vertexFile,Pages);
	SpatialIndex::ISpatialIndex *seedTree = SpatialIndex::RTree::createAndBulkLoadNewRTree (
	        SpatialIndex::RTree::BLM_STR,
	        *ds,
	        *rtreeStorageManager,
	        dias::fill_factor, dias::index_node_capacity,
	        dias::leaf_node_capacity, 3,
	        SpatialIndex::RTree::RV_RSTAR,
	        indexIdentifier);

	cout << "Forth Pass Complete\n\n";
	cout << "Indexing Done!!";
	delete rtreeStorageManager;

	/******************* DELETE STRUCTURES ******************/
	for (int i=0;i<Pages;i++) delete metadataStructure[i];
	delete ds;
#ifndef MEMORY_MAPPING
	delete btree;
#endif
}
