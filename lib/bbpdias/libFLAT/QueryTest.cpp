#include "Timer.hpp"
#include "PayLoad.hpp"
#include "SeedBuilder.hpp"
#include "SpatialObjectFactory.hpp"
#include "Metadata.hpp"
#include "Cone.hpp"



using namespace std;
using namespace FLAT;

#ifdef PROFILING
Timer crawling,seeding,loading,total;
#endif

class CrawlStatistics
{
public:
	int SeedIOs;
	int MetaDataIOs;
	int PayLoadIOs;
	int MetaDataEntries;
	int ConsideredPoints; // Used in BFS maybe Result or maybe Rejected
	int RejectedPoints;   // Used in BFS but not inside query
	int ResultPoints;     // Used in BFS and inside Query
	int UseLessPoints;    // Never used in BFS but were in page
	int UselessPage;
	int ResultPage;
	int MixedPage;

	CrawlStatistics()
	{
		SeedIOs=0;
		PayLoadIOs=0;
		MetaDataIOs=0;
		MetaDataEntries=0;

		UselessPage=0;
		ResultPage=0;
		MixedPage=0;

		ConsideredPoints=0;
		UseLessPoints=0;
		ResultPoints=0;
		RejectedPoints=0;
	}

	void print()
	{
            std::cout  << SeedIOs << "\t" << MetaDataIOs << "\t"<< PayLoadIOs << "\t" <<MetaDataEntries << "\t" << ResultPoints << "\t" <<  UseLessPoints << std::endl;
	}

	void add(CrawlStatistics* cs)
	{
		this->SeedIOs+=cs->SeedIOs;
		this->MetaDataIOs+=cs->MetaDataIOs;
		this->PayLoadIOs+=cs->PayLoadIOs;
		this->MetaDataEntries+=cs->MetaDataEntries;
		this->ConsideredPoints+=cs->ConsideredPoints;
		this->RejectedPoints+=cs->RejectedPoints;
		this->ResultPoints+=cs->ResultPoints;
		this->UseLessPoints+=cs->UseLessPoints;
		this->UselessPage+=cs->UselessPage;
		this->ResultPage+=cs->ResultPage;
		this->MixedPage+=cs->MixedPage;
	}
};

class Query
{
public:
	Box box;
	int seedID;

	Query()
	{
		seedID = -1;
	}
};

class rtreeVisitor : public SpatialIndex::IVisitor
{
public:
    Query* query;
    bool done;
    PayLoad* payload;
    CrawlStatistics* cs;

    rtreeVisitor(Query* queryPtr, string indexFileStem,CrawlStatistics* cs)
	{
		query = queryPtr;
		done = false;
		payload   = new PayLoad();
		payload->load(indexFileStem);
		this->cs = cs;
	}
    ~rtreeVisitor()
    {
    	delete payload;
    }

    virtual void visitNode(const SpatialIndex::INode& in)
    {
//		SpatialIndex::IShape* r;
//		SpatialIndex::Region re;
//		in.getShape(&r);
//		r->getMBR(re);
//		Box mbr;
//		SpatialIndex::id_type id = in.getIdentifier();
//		for (int i=0;i<DIMENSION;i++)
//		{
//		 mbr.high[i] = re.m_pHigh[i];
//		 mbr.low[i]  = re.m_pLow[i];
//		}
//		cout << in.getLevel() << "] Checking Metadata: " << id << " [" << mbr << "] ";
//		delete r;
    	if (in.isLeaf())
    	{
    		cs->MetaDataIOs++;
//    		cout << "=leaf\n";
    	}
    	else
    	{
    		cs->SeedIOs++;
//		cout << "=node\n";
    	}
	}

    virtual void visitData(const SpatialIndex::IData& in)
    {
    }

    virtual bool doneVisiting()
    {
    	return done;
    }

    virtual void visitData(const SpatialIndex::IData& in, SpatialIndex::id_type id)
    {
    	cs->MetaDataEntries++;
		FLAT::byte *b;
		uint32_t l;
		in.getData(l, &b);

		MetadataEntry m = MetadataEntry(b, l);
		delete[] b;
//
//		SpatialIndex::IShape* r;
//		SpatialIndex::Region re;
//		in.getShape(&r);
//		r->getMBR(re);
//		Box mbr;
//		for (int i=0;i<DIMENSION;i++)
//		{
//		 mbr.high[i] = re.m_pHigh[i];
//		 mbr.low[i]  = re.m_pLow[i];
//		}
//		cout << "Checking Metadata Leaf: " << id << " [" << mbr << "] \n";
//		cout << "\tMetadata Entry: ID=" << m.pageId << " Links="<<m.pageLinks.size() << " pageMBR=" << m.pageMbr << " PartitionMBR=" << m.partitionMbr << "\n";
//		delete r;
		//retrieve the page referenced & test if anything is in it
		vector<SpatialObject*> so;
		payload->getPage(so, m.pageId);
		cs->PayLoadIOs++;

		for (vector<SpatialObject*>::iterator it = so.begin(); it != so.end(); ++it)
			if (Box::overlap(query->box, (*it)->getMBR()))
			{
				done = true;
				query->seedID = id;
//				cout << "Found: " << so.size() <<" Seed:" <<id<< endl;
				break;
			}

		for (vector<SpatialObject*>::iterator it = so.begin(); it != so.end(); ++it)
				delete (*it);
	}

    virtual void visitData(std::vector<const SpatialIndex::IData *>& v
                            __attribute__((__unused__)))
    {
	}
};


void tokenize(const std::string& str,std::vector<std::string>& tokens,const std::string& delimiters = " ")
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

vector<Query> loadQueries(const std::string& filename)
{
	vector<Query> queries;
	std::ifstream readFile;
	readFile.open(filename.c_str(),std::ios::in);

	if (readFile.is_open())
	{
		while (!readFile.eof())
		{
			std::vector<std::string> tokens;
			std::string line;
			getline (readFile,line);
			if (readFile.eof()) break;
			tokenize(line, tokens);
			Query temp;
			temp.box.low[0]   = atof(tokens.at(0).c_str());
			temp.box.low[1]   = atof(tokens.at(1).c_str());
			temp.box.low[2]   = atof(tokens.at(2).c_str());
			temp.box.high[0]  = atof(tokens.at(3).c_str());
			temp.box.high[1]  = atof(tokens.at(4).c_str());
			temp.box.high[2]  = atof(tokens.at(5).c_str());
			queries.push_back(temp);
		}
		readFile.close();
	}

	return queries;
}

int main(int argc, const char* argv[])
{
	/********************** ARGUMENTS ***********************/

	string inputStem = argv[1];
	string queryFile  = argv[2];
    //std::cout  << "S-IOs" << "\t" << "MD-IOs" << "\t"<< "PLD-IOs" << "\t" << "MD-Ent" << "\t" << "Results" << "\t" <<  "UseLess" << std::endl;

	/******************** LOADING INDEX *********************/
#ifdef PROFILING
	total.start();
	loading.start();
#endif
	PayLoad* payload = new PayLoad();
	payload->load(inputStem);
	string seedfile = inputStem + "_index";
	SpatialIndex::IStorageManager* rtreeStorageManager = SpatialIndex::StorageManager::loadDiskStorageManager(seedfile);
	SpatialIndex::id_type indexIdentifier = 1;
	SpatialIndex::ISpatialIndex* seedtree = SpatialIndex::RTree::loadRTree(*rtreeStorageManager, indexIdentifier);
#ifdef PROFILING
	loading.stop();
#endif
	/********************** DO QUERIES **********************/
	vector<Query> queries = loadQueries(queryFile);
	CrawlStatistics* totalStats = new CrawlStatistics();
	for (vector<Query>::iterator query = queries.begin(); query != queries.end();query++)
	{
		CrawlStatistics* cs = new CrawlStatistics();
		queue<int> metapageQueue;
		set<int> visitedMetaPages;
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
		rtreeVisitor visitor(&(*query),inputStem,cs);
		seedtree->seedQuery(query_region, visitor);
		if (query->seedID != -1)
			metapageQueue.push(query->seedID);
#ifdef PROFILING
	seeding.stop();
	crawling.start();
#endif
		/*=================== CRAWLING ======================*/
		while (!metapageQueue.empty())
		{
			/*--------------  VISIT ---------------*/
			int visitPage = metapageQueue.front();
			metapageQueue.pop();
			if(visitedMetaPages.find(visitPage) != visitedMetaPages.end()) continue;
			nodeSkeleton * nss = SeedBuilder::readNode(visitPage, rtreeStorageManager);
			visitedMetaPages.insert(visitPage);
			cs->MetaDataIOs++;

			if (nss->nodeType == SpatialIndex::RTree::PersistentLeaf) //ARE we going to read some non Persistent leaves too??
			{
				for (unsigned i = 0; i < nss->children; i++)
				{
					MetadataEntry m = MetadataEntry(nss->m_pData[i], nss->m_pDataLength[i]);
					cs->MetaDataEntries++;
					if (Box::overlap(m.partitionMbr,query->box))
					{
						for (set<id>::iterator links = m.pageLinks.begin(); links != m.pageLinks.end(); links++)
							metapageQueue.push(*links);

						Box pageMBR(Vertex(nss->m_ptrMBR[i]->m_pLow[0],	nss->m_ptrMBR[i]->m_pLow[1],nss->m_ptrMBR[i]->m_pLow[2]),
								    Vertex(nss->m_ptrMBR[i]->m_pHigh[0],nss->m_ptrMBR[i]->m_pHigh[1],nss->m_ptrMBR[i]->m_pHigh[2]));

						if (Box::overlap(pageMBR,query->box))
						{
							vector<SpatialObject*> objects;
							payload->getPage(objects, m.pageId);
							cs->PayLoadIOs++;

							for (vector<SpatialObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
							{
								if (Box::overlap(query->box, (*it)->getMBR()))
								{
//									if (payload->objType==VERTEX)
//										cout << "Vertex: " << *((Vertex*)(*it)) << "\n";
//									else
//										cout << "Cone: " << *((Cone*)(*it)) << "\n";

									cs->ResultPoints++;
								}
								else
									cs->UseLessPoints++;
								delete (*it);
							}
						}
					}
				}
			}
			delete nss;
		}
#ifdef PROFILING
	crawling.stop();
#endif
	//cs->print();
	totalStats->add(cs);
	delete cs;
	}
	//cout <<"\n";
	totalStats->print();
	delete totalStats;

	delete seedtree;
	delete rtreeStorageManager;
	delete payload;

#ifdef PROFILING
	total.stop();
	cout << "Profiling: Seeding: " << seeding << ", Crawling: " << crawling << ", Loading: " << loading << ", Total: " << total << "\n";
#endif

}
