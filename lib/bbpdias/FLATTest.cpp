#include "cpu_timer.cpp"
#include "Query.cpp"
#include "QueryGenerator.cpp"
#include "VertexStructure.hpp"
#include "FLATGenerator.hpp"


//#define ACCURACY_TEST
#define PROFILING

using namespace dias;
using namespace std;

#ifdef PROFILING
tpie::cpu_timer crawling,seeding,loading,total;
#endif

class rtreeVisitor : public SpatialIndex::IVisitor
{
public:

    Query* query;
    bool done;
    VertexStructure* vertexStructure;
    Vertex blank;

    rtreeVisitor(Query* queryPtr, string vertexFile)
		{
			query = queryPtr;
			done = false;
			vertexStructure = new VertexStructure(vertexFile,false);
		}

    virtual void visitNode(const SpatialIndex::INode& in) {
	}

    virtual bool doneVisiting() {
    	return false;
    };

    virtual void visitData(const SpatialIndex::IData& in) {

    }

    virtual void visitData(const SpatialIndex::IData& in, SpatialIndex::id_type id) {
		byte *b;
		uint32_t l;
		in.getData(l, &b);

		MetadataEntry m = MetadataEntry(b, l);

		//retrieve the page referenced & test if anything is in it
		VertexPage* vp = vertexStructure->Get(m.pageId);

		for (int i = 0; i < VERTICES_PER_PAGE; i++) {
			if (vp->vertices[i] == blank)
				continue;
			if (Box::enclose(query->box, vp->vertices[i])) {
				done = true;
				query->seedID = id;
				break;
			}
		}

		delete vp;
	}

    virtual void visitData(std::vector<const SpatialIndex::IData *>& v
                            __attribute__((__unused__))) {
	}
};

class CrawlStatistics
{
public:
	int SeedIOs;
	int CrawlIOs;
	int MetaLinks;
	int ConsideredPoints; // Used in BFS maybe Result or maybe Rejected
	int RejectedPoints;   // Used in BFS but not inside query
	int ResultPoints;     // Used in BFS and inside Query
	int UseLessPoints;    // Never used in BFS but were in page
	int UselessPage;
	int ResultPage;
	int MixedPage;

	CrawlStatistics()
	{
		UselessPage=0;
		MetaLinks=0;
		ResultPage=0;
		MixedPage=0;
		SeedIOs=0;
		CrawlIOs=0;
		ConsideredPoints=0;
		UseLessPoints=0;
		ResultPoints=0;
		RejectedPoints=0;
	}

	void print()
	{
            std::cout  << SeedIOs << "," << CrawlIOs << "," << ResultPoints << "," <<  UseLessPoints << std::endl;
	}
};

int main(int argc, const char* argv[])
{
	/********************** ARGUMENTS ***********************/

	string inputStem = argv[1];
	string queryFile  = argv[2];
	string seedFile   = inputStem + "_seed";
	string vertexFile = inputStem + "_vertex.dat";

	/******************** LOADING INDEX *********************/
#ifdef PROFILING
	total.start();
	loading.start();
#endif
	VertexStructure* vertexStructure = new VertexStructure(vertexFile,false);
	SpatialIndex::IStorageManager* rtreeStorageManager = SpatialIndex::StorageManager::loadDiskStorageManager(seedFile);
	SpatialIndex::id_type indexIdentifier = 1;
	SpatialIndex::ISpatialIndex* seedtree = SpatialIndex::RTree::loadRTree(*rtreeStorageManager, indexIdentifier);

	cout << "Index Loaded\n";
#ifdef PROFILING
	loading.stop();
#endif
	/********************** DO QUERIES **********************/



	QueryGenerator qg;
	qg.Load (queryFile);
	Vertex blank; // last page of the index is partially filled with blank vertices. not to be considered in result.
	CrawlStatistics cs;
	for (vector<Query>::iterator query = qg.queries.begin();query != qg.queries.end();query++)
	{
		int nodereads = 0;
		int mdreads = 0;
		int crawls = 0;
		int results = 0;

#ifdef ACCURACY_TEST
		/*===== BRUTE FORCE RIVAL FOR ACCURACY TEST==========*/
		int results=0;
		for (int j=0;j<Pages;j++)
		{
			VertexPage* vp = vertexStructure->Get(j);
			for (int i=0;i<VERTICES_PER_PAGE;i++)
				if (Box::enclose(query->box,vp->vertices[i]))
					results++;
		}
#endif

		queue<int> pageQueue;
		set<int> visitedPages;
		set<int> resultPages;

		/*=================== SEEDING ======================*/
#ifdef PROFILING
	seeding.start();
#endif
		double lo[DIMENSION],hi[DIMENSION];
		for (int i=0;i<DIMENSION;i++)
		{
			lo[i] = (double)(*query).box.low[i];
			hi[i] = (double)(*query).box.high[i];
		}

		SpatialIndex::Region query_region = SpatialIndex::Region(lo,hi,DIMENSION);

		rtreeVisitor visitor = rtreeVisitor(&(*query), vertexFile);

		seedtree->seedQuery(query_region, visitor);

		if (query->seedID != -1)
			{
			pageQueue.push(query->seedID);
			}

#ifdef PROFILING
	seeding.stop();
	crawling.start();
#endif
		/*=================== CRAWLING ======================*/
		while (!pageQueue.empty())
		{

			/*--------------  VISIT ---------------*/
			int visitPage = pageQueue.front();
			pageQueue.pop();

			if(visitedPages.find(visitPage) != visitedPages.end()) continue;

			nodeSkeleton * nss = readNode(visitPage, rtreeStorageManager);
			nodereads++;
			visitedPages.insert(visitPage);

			if (nss->nodeType == SpatialIndex::RTree::PersistentLeaf) {

				for (int i = 0; i < nss->children; i++) {
					MetadataEntry m = MetadataEntry(nss->m_pData[i], nss->m_pDataLength[i]);
					mdreads++;
					if (Box::overlap(m.voronoiMbr,query->box)) {

						Box pageMBR(Vertex(nss->m_ptrMBR[i]->m_pLow[0],
								nss->m_ptrMBR[i]->m_pLow[1],
								nss->m_ptrMBR[i]->m_pLow[2]), Vertex(
								nss->m_ptrMBR[i]->m_pHigh[0],
								nss->m_ptrMBR[i]->m_pHigh[1],
								nss->m_ptrMBR[i]->m_pHigh[2]));

						for (set<id>::iterator links = m.pageLinks.begin(); links != m.pageLinks.end(); links++)
						{
							pageQueue.push(*links);
						}

						if (Box::overlap(pageMBR,query->box)) {
							VertexPage* vp = vertexStructure->Get(m.pageId);
							cs.CrawlIOs++;
							crawls++;
							for (int i=0;i<VERTICES_PER_PAGE;i++) {
								if (vp->vertices[i]==blank) continue;
								if (Box::enclose(query->box,vp->vertices[i])) {
									resultPages.insert(m.pageId);
									cs.ResultPoints++;
									results++;
								} else
									cs.UseLessPoints++;
							}
							delete vp;
						}
					}
				}
			}
			delete nss;
		}
#ifdef PROFILING
	crawling.stop();
#endif

	cout << resultPages.size() << endl;

	vector<SpatialIndex::id_type> keys;

	set<SpatialIndex::id_type> metanodes;

	rtreeStorageManager->getKeys(&keys);

	for (vector<SpatialIndex::id_type>::iterator iter = keys.begin(); iter != keys.end(); ++iter) {

		SpatialIndex::Region m_nodeMBR;

		SpatialIndex::id_type id = *iter;
		nodeSkeleton * nss = readNode(id, rtreeStorageManager);

		if (nss->nodeType == SpatialIndex::RTree::PersistentLeaf) {
			for (int i = 0; i < nss->children; i++) {
				MetadataEntry m = MetadataEntry(nss->m_pData[i], nss->m_pDataLength[i]);

				if(resultPages.find(m.pageId) != resultPages.end()) {
					metanodes.insert(id);
				}
			}
		}
	}

	cout << metanodes.size() << endl;
	cout << nodereads << " " << mdreads << " " << results << " " << crawls << endl;

		cs.MetaLinks=visitedPages.size();
#ifdef ACCURACY_TEST
		if (results==cs.ResultPoints)
		cout << "ACCURATE! Brute Force Results: " << results << "\t\tHALT Results:" << cs.ResultPoints << "\n";
		else
		cout << "ERROR! Brute Force Results: " << results << "\t\tHALT Results:" << cs.ResultPoints << "\n";
#endif
	}
#ifdef PROFILING
	total.stop();
	cout << "Stats: Seed I/Os" << cs.SeedIOs << "," << seeding << "," << cs.CrawlIOs <<"," << crawling << "," << loading << "," << total << "," << cs.ResultPoints << "\n";
#endif

}
