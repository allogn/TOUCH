#include "cpu_timer.cpp"
#include "Rtree.cpp"
#include "Query.cpp"
#include "QueryGenerator.cpp"
#include "VertexStructure.hpp"
#include "Metadata.hpp"


//#define ACCURACY_TEST
#define PROFILING

//using namespace dias;
using namespace std;

#ifdef PROFILING
tpie::cpu_timer crawling,seeding,loading,total;
#endif


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
	string metaFile   = inputStem + "_meta.dat";

	/******************** LOADING INDEX *********************/
#ifdef PROFILING
	total.start();
	loading.start();
#endif
	MetaData* metadata = new MetaData(metaFile,false);
	int Pages = metadata->Pages;
	MetadataEntry* metadataStructure [Pages];
	metadata->Load(metadataStructure);
	VertexStructure* vertexStructure = new VertexStructure(vertexFile,false);
	Rtree* seedTree = new Rtree(seedFile,false);
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

		/*=================== SEEDING ======================*/
#ifdef PROFILING
	seeding.start();
#endif
		seedTree->Seed(*query);
		cs.SeedIOs += query->nodeAccesses;
		if (query->points>0)
			{
			pageQueue.push(query->seed.id);
			visitedPages.insert( query->seed.id );
			}
#ifdef PROFILING
	seeding.stop();
	crawling.start();
#endif
		/*=================== CRAWLING ======================*/
		while (!pageQueue.empty())
		{
			/*-------------- VISIT ---------------*/
			int visitPage = pageQueue.front();
			pageQueue.pop();
			if (Box::overlap(metadataStructure[visitPage]->pageMbr,query->box))
			{
				VertexPage* vp = vertexStructure->Get(visitPage);
				cs.CrawlIOs++;
				for (int i=0;i<VERTICES_PER_PAGE;i++)
				{
					if (vp->vertices[i]==blank) continue;
					if (Box::enclose(query->box,vp->vertices[i]))
						cs.ResultPoints++;
					else
						cs.UseLessPoints++;
				}
				delete vp;
			}

			/*---------- BFS CANDIDATES -----------*/
			for (set<id>::iterator links = metadataStructure[visitPage]->pageLinks.begin(); links != metadataStructure[visitPage]->pageLinks.end(); links++)
			{
				if (visitedPages.find(*links)!=visitedPages.end()) continue;
				visitedPages.insert( *links );

				if (Box::overlap(metadataStructure[*links]->voronoiMbr,query->box))
					pageQueue.push(*links);
			}
		}
#ifdef PROFILING
	crawling.stop();
#endif
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
	cout << cs.SeedIOs << "," << seeding << "," << cs.CrawlIOs <<"," << crawling << "," << loading << "," << total << "," << cs.ResultPoints << "\n";
#endif

}
