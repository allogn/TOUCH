#include "STRIndex.h"

STRIndex::STRIndex()
{
        objectCount=0;
        objectSize=0;
        pageCount=0;
        binCount=0;
        objectPerPage=0;
        objectPerXBins=0;
        objectPerYBins=0;
        payload = new FLAT::PayLoad();
        metadataStructure = new vector<FLAT::MetadataEntry*>();
}

STRIndex::~STRIndex()
{
        delete payload;
        delete metadataStructure;
}

void STRIndex::buildIndex(FLAT::SpatialObjectStream* input,string indexFileStem)
{
#ifdef PROFILING
        FLAT::Timer tesselation,seeding,linker;
        tesselation.start();
#endif
#ifdef INFORMATION
        cout << "\n == STR TESSELLATION ==\n\n";
#endif
        initialize(input,indexFileStem);
        doTessellation(input);
#ifdef PROFILING
        tesselation.stop();
        cout << "Tessellation Duration: " << tesselation << "\n";
        linker.start();
#endif
#ifdef INFORMATION
        cout << "\n == LINKER RTREE BUILDING ==\n\n";
#endif
        FLAT::MetaDataStream* metaStream = new FLAT::MetaDataStream(metadataStructure);
        SpatialIndex::IStorageManager* rtreeStorageManager = SpatialIndex::StorageManager::createNewMemoryStorageManager();
        FLAT::uint32 fanout = (FLAT::uint32)floor(PAGE_SIZE-76+0.0)/(objectSize+12+0.0);

        SpatialIndex::id_type indexIdentifier=1;
        SpatialIndex::ISpatialIndex *linkerTree = SpatialIndex::RTree::createAndBulkLoadNewRTree (
                SpatialIndex::RTree::BLM_STR,
                *metaStream,
                *rtreeStorageManager,
                        0.9999, fanout,
                        fanout, DIMENSION,
                SpatialIndex::RTree::RV_RSTAR,
                indexIdentifier);
#ifdef PROFILING
        linker.stop();
        cout << "Linker Creation Duration: " << linker << "\n";
        seeding.start();
#endif
#ifdef INFORMATION
        cout << "\n == BUILDING SEED INDEX WHILE INDUCING LINKS ==\n\n";
#endif
        FLAT::MetaDataStream* metaDataStream = new FLAT::MetaDataStream(metadataStructure,linkerTree);

        FLAT::SeedBuilder::buildSeedTree(indexFileStem,metaDataStream);

#ifdef DEBUG
        cout << "TOTAL PAGES: " << metaDataStream->pages <<endl;
        cout << "TOTAL LINKS ADDED: " << metaDataStream->links <<endl;
        cout << "SUMMED VOLUME: " << metaDataStream->sumVolume <<endl;
        cout << "AVERAGE LINKS: " << ((metaDataStream->links+0.0) / (metaDataStream->pages+0.0)) << endl;
        cout << "AVERAGE VOLUME: " << ((metaDataStream->sumVolume+0.0) / (metaDataStream->pages+0.0)) << endl;

        for (int i=0;i<100;i++)
                cout << metaDataStream->frequency[i] << "\n";

        //cout << "OVERFLOW VOLUME: " << metaDataStream->overflow <<endl;
        //for (int i=0;i<100;i++)
        //	cout << metaDataStream->volumeDistributon[i] << "\t" << metaDataStream->volumeLink[i] << "\t"
                 //    << ( (metaDataStream->volumeLink[i]+0.0)/(metaDataStream->volumeDistributon[i]+0.0)) << "\n" ;
#endif
        delete metaDataStream;
#ifdef PROFILING
        seeding.stop();
        cout << "Building Seed Structure & Links Duration: " << seeding << "\n";
#endif
}

void STRIndex::initialize(FLAT::SpatialObjectStream* input,string indexFileStem)
{
        objectCount     = input->objectCount;
        objectSize      = input->objectByteSize;
        objectType		= input->objectType;
        universe        = input->universe;
        //FLAT::Box::infiniteFLAT::Box(universe); // to make open ended Partition MBRs in the corner of universe... dont do it

#ifdef BBP
        objectPerPage = (FLAT::uint64)floor((PAGE_SIZE-4.0) / (objectSize+0.0)); // minus 4 bytes because each page has an int counter with it
#else
        objectPerPage = (FLAT::uint64)floor((PAGE_SIZE-4.0) / (objectSize+0.0));
        //objectPerPage   = 67; // to Degrade FLAT to change back remove comment
#endif
        pageCount       = (FLAT::uint64)ceil( (objectCount+0.0) / (objectPerPage+0.0) );
        binCount        = pow (pageCount,1.0/(3+0.0));

        objectPerXBins  = (FLAT::uint64)ceil((objectCount+0.0) / binCount);
        objectPerYBins  = (FLAT::uint64)ceil((objectPerXBins+0.0) / binCount);

#ifdef DEBUG
                cout << "MINIMUM PAGES NEED TO STORE DATA: "<<pageCount <<endl
                 << "PAGES BINS PER DIMENSION: " << binCount << endl
                 << "OBJECTS IN EVERY X BIN: " << objectPerXBins << endl
                 << "OBJECTS IN EVERY Y BIN: " << objectPerYBins << endl
                 << "OBJECTS IN EVERY Z BIN or PAGE: " << objectPerPage << endl;
#endif
        metadataStructure->reserve(pageCount);
        payload->create(indexFileStem,PAGE_SIZE,objectPerPage,objectSize,objectType);
}

void STRIndex::doTessellation(FLAT::SpatialObjectStream* input)
{
        FLAT::ExternalSort* xSort = new FLAT::ExternalSort(SORTING_FOOTPRINT_MB,0,objectType);
        FLAT::ExternalSort* ySort = new FLAT::ExternalSort(SORTING_FOOTPRINT_MB,1,objectType);
        FLAT::ExternalSort* zSort = new FLAT::ExternalSort(SORTING_FOOTPRINT_MB,2,objectType);
        FLAT::Box Partition = universe;
        FLAT::uint64 PageCount=0;
        FLAT::uint64 dataCount=0;

#ifdef DEBUG
        int idx=0,idy=0,idz=0;
#endif
        // SORTING AND BINNING
#ifdef INFORMATION
        cout << "READING INPUT...." << endl;
#endif
        while (input->hasNext())
                xSort->insert(input->getNext());
#ifdef INFORMATION
        cout << "\nSORTING...." << endl;
#endif
        xSort->sort();

        FLAT::uint64 xCount=0,oldCountX=0;

        while(xSort->hasNext())
        {
                FLAT::SpatialObject* xtemp = xSort->getNext();
                ySort->insert(xtemp);
                xCount++;
                if (xCount%objectPerXBins==0 || xCount==objectCount)
                {
                        if (xCount>objectPerXBins) Partition.low[0] = Partition.high[0];
                        if (xCount==objectCount) Partition.high[0] = universe.high[0];
                        else Partition.high[0] = xtemp->getSortDimension(0);
                        ySort->sort();
                        FLAT::uint64 yCount=0,oldCountY=0;

                        while(ySort->hasNext())
                        {
                                FLAT::SpatialObject* ytemp = ySort->getNext();
                                zSort->insert(ytemp);
                                yCount++;

                                if (yCount%objectPerYBins==0 || yCount==xCount-oldCountX)
                                {
                                        if (yCount>objectPerYBins) Partition.low[1] = Partition.high[1];
                                        if (yCount==xCount-oldCountX) Partition.high[1] = universe.high[1];
                                        else Partition.high[1] = ytemp->getSortDimension(1);
                                        zSort->sort();

                                        /////////////////// MAKING META AND PAYLOAD PAGES ////////////////////
                                        vector<FLAT::SpatialObject*> items;
                                        FLAT::Box PageMBR;
                                        FLAT::uint64 zCount=0;

                                        while (zSort->hasNext())
                                        {
                                                FLAT::SpatialObject* temp = zSort->getNext();
                                                //cout << ((Cone*)temp) <<" - " << temp->getSortDimension(0) << " - " << temp->getSortDimension(1) << " - "<< temp->getSortDimension(2) <<endl;
                                                items.push_back(temp);
                                                zCount++;
                                                dataCount++;
#ifdef PROGRESS
                                                if (dataCount%10000000==0) cout << "INDEXING OBJECTS: " << dataCount << " DONE"<< endl;
#endif
                                                if (zCount%objectPerPage==0 || zCount==yCount-oldCountY)
                                                {
                                                        FLAT::Box::boundingBox(PageMBR,items);
                                                        if (zCount>objectPerPage) Partition.low[2] = Partition.high[2];
                                                        if (zCount==yCount-oldCountY) Partition.high[2] = universe.high[2];
                                                        else Partition.high[2] = temp->getSortDimension(2);
                                                        FLAT::MetadataEntry* metaEntry = new FLAT::MetadataEntry();
                                                        metaEntry->pageMbr = PageMBR;
                                                        metaEntry->partitionMbr = Partition + PageMBR;
                                                        metaEntry->pageId = PageCount;
#ifdef DEBUG
                                                        metaEntry->i = idx; metaEntry->j = idy; metaEntry->k =idz;
                                                        //cout <<metaEntry->pageId << " ["<< idx << "," << idy << "," << idz << "] \t" << metaEntry->partitionMbr  << "\t" << metaEntry->pageMbr<< endl;
#endif
                                                        metadataStructure->push_back(metaEntry);

                                                        PageCount++;
                                                        payload->putPage(items);
                                                        items.clear();
                                                        if (zCount==yCount-oldCountY) break;
#ifdef DEBUG
                                                        idz++;
#endif
                                                }

                                        }
                                        ////////////////////////////////////////////////////////////////////
                                        Partition.low[2] = universe.low[2];
                                        zSort->clean();
                                        oldCountY = yCount;
                                        if (yCount==xCount-oldCountX) break;
#ifdef DEBUG
                                        idy++;idz=0;
#endif
                                }
                        }
                        Partition.low[1] = universe.low[1];
                        ySort->clean();
                        oldCountX = xCount;
                        if (xCount==objectCount) break;
#ifdef DEBUG
                        idx++;idy=0;idz=0;
#endif
                }
        }
        Partition.low[0] = universe.low[0];
        xSort->clean();

#ifdef INFORMATION
        cout << "PAGES USED FOR INDEX: " << PageCount << endl;
        cout << "OBJECTS INDEXED: " << dataCount << endl;
#endif
        delete zSort;
        delete ySort;
        delete xSort;
}

void STRIndex::induceConnectivityFaster()
{
        uint32_t pages = metadataStructure->size();
        uint32_t hopFactor = (uint32_t)((floor( (objectPerXBins+0.0) / (objectPerYBins+0.0)) *
                                      ceil ( (objectPerYBins+0.0) / (objectPerPage+0.0))) +
                                      ceil ( ((objectPerXBins%objectPerYBins)+0.0) / (objectPerPage+0.0)));
#ifdef DEBUG
        uint32_t links=0;
#endif
        for (uint32_t i=0;i<pages;++i)
        {
                for (uint32_t j=i+1;j<pages;++j)
                {
                        if (metadataStructure->at(i)->partitionMbr.high[0] < metadataStructure->at(j)->partitionMbr.low[0])
                                break;

                        if ((metadataStructure->at(i)->partitionMbr.high[1] < metadataStructure->at(j)->partitionMbr.low[1]))
                        {
                                uint32_t nextHop = ((j/hopFactor)+1)*hopFactor;
                                if (nextHop < pages)
                                        j = nextHop;
                        }

                        if (FLAT::Box::overlap( metadataStructure->at(i)->partitionMbr , metadataStructure->at(j)->partitionMbr ))
                        {
                                metadataStructure->at(i)->pageLinks.insert(j);
                                metadataStructure->at(j)->pageLinks.insert(i);
#ifdef DEBUG
                                links+=2;
#endif
                        }
                }
#ifdef PROGRESS
                if (i%100000==0 && i>0) cout << "INDUCING LINKS: "<< i << " PAGES DONE" << endl;
#endif
        }

#ifdef DEBUG
        cout << "TOTAL PAGES: " << pages <<endl;
        cout << "TOTAL LINKS ADDED: "<< links <<endl;

//		int frequency[100];
//		for (int i=0;i<100;i++) frequency[i]=0;
//		for (uint32_t j=0;j<pages;j++)
//		{
//			if (metadataStructure->at(j)->pageLinks.size()>100)
//			{
//				cout << "id("<< j<< ") = [" <<  metadataStructure->at(j)->i << "," << metadataStructure->at(j)->j << "," << metadataStructure->at(j)->k << "] \tLINKS:" << metadataStructure->at(j)->pageLinks.size() << " \tMBR" << metadataStructure->at(j)->partitionMbr << "\n";
//
//				//for (set<id>::iterator i = metadataStructure->at(j)->pageLinks.begin();i !=  metadataStructure->at(j)->pageLinks.end(); ++i)
//				//	cout << "\tid("<< *i << ") = [" <<  metadataStructure->at(*i)->i << "," << metadataStructure->at(*i)->j << "," << metadataStructure->at(*i)->k << "] \tLINKS:" << metadataStructure->at(*i)->pageLinks.size() << " \tMBR" << metadataStructure->at(*i)->partitionMbr << "\n";
//			}
//			if (metadataStructure->at(j)->pageLinks.size()<100)
//				frequency[metadataStructure->at(j)->pageLinks.size()]++;
//		}


//		for (int i=0;i<100;i++)
//			cout << "Links: " << i << " Frequency: " << frequency[i] << "\n";
#endif
}

void STRIndex::loadIndex(string indexFileStem)
{
        payload->load(indexFileStem);
        vector<FLAT::SpatialObject*> items;
        for (int i=0;i<41;i++)
        {
                payload->getPage(items,i);
                items.clear();
        }
}

