#include "STRIndex.hpp"
#include "ExternalSort.hpp"
#include "Timer.hpp"

namespace FLAT
{
	bool metaDataComparatorX(MetadataWrapper * i, MetadataWrapper * j) {

		Vertex vi, vj;

		if(i->begin)
			vi = i->md->partitionMbr.low;
		else
			vi = i->md->partitionMbr.high;

		if(j->begin)
			vj = j->md->partitionMbr.low;
		else
			vj = j->md->partitionMbr.high;

		if(vi[0] == vj[0]) {

			if(i->begin && !j->begin)
				return true;
			else if(!i->begin && j->begin)
				return false;
		}
		return vi[0] < vj[0];
	}

	STRIndex::STRIndex()
	{
		objectCount=0;
		objectSize=0;
		pageCount=0;
		binCount=0;
		objectPerPage=0;
		objectPerXBins=0;
		objectPerYBins=0;
		payload = new PayLoad();
		metadataStructure = new vector<MetadataEntry*>();
		metadataVector = new vector<MetadataWrapper*>();
	}

	STRIndex::~STRIndex()
	{
		delete payload;
		for (vector<MetadataWrapper*>::iterator it = metadataVector->begin();
						it != metadataVector->end(); ++it)
					delete (*it);

		delete metadataStructure;
		delete metadataVector;
	}

	void STRIndex::buildIndex(SpatialObjectStream* input,string indexFileStem)
	{
#ifdef PROFILING
		Timer tesselation,connectivity,seeding;
		tesselation.start();
#endif
#ifdef PROGRESS
		cout << "\n == STR TESSELLATION ==\n\n";
#endif
		initialize(input,indexFileStem);
		doTessellation(input);
#ifdef PROFILING
		tesselation.stop();
		cout << "Tessellation Duration: " << tesselation << "\n";
		connectivity.start();
#endif
#ifdef PROGRESS
//		cout << "\n == INDUCE CONNECTIVITY ==\n\n";
#endif
//		induceConnectivityFaster();
#ifdef PROFILING
//		connectivity.stop();
//		cout << "Inducing Links Duration: " << connectivity << "\n";
		seeding.start();
#endif
#ifdef PROGRESS
		cout << "\n == BUILDING SEED INDEX WHILE INDUCING LINKS ==\n\n";
#endif
		MetaDataStream* metaDataStream = new MetaDataStream(metadataStructure,objectPerXBins,objectPerYBins,objectPerPage);
		SeedBuilder::buildSeedTree(indexFileStem,metaDataStream);
#ifdef DEBUG
		cout << "TOTAL PAGES: " << metaDataStream->pages <<endl;
		cout << "TOTAL LINKS ADDED: "<< metaDataStream->links <<endl <<endl;
		for (int i=0;i<100;i++)
			cout << metaDataStream->frequency[i] << "\n";
#endif
		delete metaDataStream;
#ifdef PROFILING
		seeding.stop();
		cout << "Building Seed Structure & Links Duration: " << seeding << "\n";
#endif
	}

	void STRIndex::initialize(SpatialObjectStream* input,string indexFileStem)
	{
		objectCount     = input->objectCount;
		objectSize      = input->objectByteSize;
		objectType		= input->objectType;
		universe        = input->universe;

		objectPerPage   = (int)floor((PAGE_SIZE-4.0) / (objectSize+0.0)); // minus 4 bytes because each page has an int counter with it
		pageCount       = (int)ceil( (objectCount+0.0) / (objectPerPage+0.0) );
		binCount        = pow (pageCount,1.0/(3+0.0));

		objectPerXBins  = (int)ceil((objectCount+0.0) / binCount);
		objectPerYBins  = (int)ceil((objectPerXBins+0.0) / binCount);

#ifdef DEBUG
			cout << "MINIMUM PAGES NEED TO STORE DATA: "<<pageCount <<endl
			 << "PAGES BINS PER DIMENSION: " << binCount << endl
			 << "OBJECTS IN EVERY X BIN: " << objectPerXBins << endl
			 << "OBJECTS IN EVERY Y BIN: " << objectPerYBins << endl
			 << "OBJECTS IN EVERY Z BIN or PAGE: " << objectPerPage << endl;
#endif

		payload->create(indexFileStem,PAGE_SIZE,objectPerPage,objectSize,objectType);
	}

	void STRIndex::doTessellation(SpatialObjectStream* input)
	{
		ExternalSort* xSort = new ExternalSort(SORTING_FOOTPRINT_MB,0,objectType);
		ExternalSort* ySort = new ExternalSort(SORTING_FOOTPRINT_MB,1,objectType);
		ExternalSort* zSort = new ExternalSort(SORTING_FOOTPRINT_MB,2,objectType);
		Box Partition = universe;
		uint32_t PageCount=0;
		uint32_t dataCount=0;

#ifdef DEBUG
		int idx=0,idy=0,idz=0;
#endif
		// SORTING AND BINNING
#ifdef PROGRESS
		cout << "\nREADING INPUT...." << endl;
#endif
		while (input->hasNext())
			xSort->insert(input->getNext());
#ifdef PROGRESS
		cout << "\nSORTING...." << endl;
#endif
		xSort->sort();

		int xCount=0,oldCountX=0;

		while(xSort->hasNext())
		{
			SpatialObject* xtemp = xSort->getNext();
			ySort->insert(xtemp);
			xCount++;

			if (xCount%(int)objectPerXBins==0 || xCount==objectCount)
			{
				if (xCount>objectPerXBins) Partition.low[0] = Partition.high[0];
				if (xCount==objectCount) Partition.high[0] = universe.high[0];
				else Partition.high[0] = xtemp->getSortDimension(0);
				ySort->sort();
				int yCount=0,oldCountY=0;

				while(ySort->hasNext())
				{
					SpatialObject* ytemp = ySort->getNext();
					zSort->insert(ytemp);
					yCount++;

					if (yCount%(int)objectPerYBins==0 || yCount==xCount-oldCountX)
					{
						if (yCount>objectPerYBins) Partition.low[1] = Partition.high[1];
						if (yCount==xCount-oldCountX) Partition.high[1] = universe.high[1];
						else Partition.high[1] = ytemp->getSortDimension(1);
						zSort->sort();

						/////////////////// MAKING META AND PAYLOAD PAGES ////////////////////
						vector<SpatialObject*> items;
						Box PageMBR;
						int zCount=0;

						while (zSort->hasNext())
						{
							SpatialObject* temp = zSort->getNext();
							items.push_back(temp);
							zCount++;
							dataCount++;
#ifdef PROGRESS
							if (dataCount%10000000==0) cout << "INDEXING OBJECTS: " << dataCount << " DONE"<< endl;
#endif
							if (zCount%objectPerPage==0 || zCount==yCount-oldCountY)
							{
								Box::boundingBox(PageMBR,items);
								if (zCount>objectPerPage) Partition.low[2] = Partition.high[2];
								if (zCount==yCount-oldCountY) Partition.high[2] = universe.high[2];
								else Partition.high[2] = temp->getSortDimension(2);
								MetadataEntry* metaEntry = new MetadataEntry();
								metaEntry->pageMbr = PageMBR;
								metaEntry->partitionMbr = Partition + PageMBR;
								metaEntry->pageId = PageCount;
#ifdef DEBUG
								metaEntry->i = idx; metaEntry->j = idy; metaEntry->k =idz;
#endif
								metadataStructure->push_back(metaEntry);

								metadataVector->push_back(new MetadataWrapper(metaEntry, true));
								metadataVector->push_back(new MetadataWrapper(metaEntry, false));

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

				if (Box::overlap( metadataStructure->at(i)->partitionMbr , metadataStructure->at(j)->partitionMbr ))
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

	void STRIndex::induceConnectivity()
	{
//		for (vector<MetadataEntry*>::iterator first = metadataStructure->begin();first != metadataStructure->end(); ++first)
//		{
//			for (vector<MetadataEntry*>::iterator second = first; second != metadataStructure->end(); ++second)
//			{
//				if (first!=second)
//				{
//#ifdef DEBUG
//					comparisons++;
//#endif
//					if (Box::overlap( (*first)->partitionMbr , (*second)->partitionMbr ))
//					{
//						(*first)->pageLinks.insert(j);
//						(*second)->pageLinks.insert(i);
//
//#ifdef DEBUG
//						links+=2;
////						cout << "NEIGHBOURING Tiles..... id("<< i<< ") =[" << (*first)->i << "," << (*first)->j << "," << (*first)->k << "]"// MBR" << (*first)->partitionMbr << "\n"
////						<< "  id("<< j<< ") =[" << (*second)->i << "," << (*second)->j << "," << (*second)->k << "]\n";// << (*second)->partitionMbr << "\n";
//#endif
//					}
//				}
//				j++;
//			}
//			i++;
//#ifdef PROGRESS
//			if (i%100000==0) cout << "INDUCING LINKS: "<< i << " PAGES DONE" << endl;
//#endif
//			j=i;
//		}

		uint32_t links=0;
		for (unsigned j = 0; j < metadataStructure->size(); j++) {
			metadataStructure->at(j)->pageId = j;
		}

		std::sort(metadataVector->begin(), metadataVector->end(), metaDataComparatorX);

		SpatialIndex::id_type indexIdentifier = 0;
		SpatialIndex::IStorageManager* memory = SpatialIndex::StorageManager::createNewMemoryStorageManager();
		SpatialIndex::ISpatialIndex *sweepLineTree = SpatialIndex::RTree::createNewRTree(*memory, 0.9, 100, 100, 3, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);

		for(vector<MetadataWrapper*>::iterator it = metadataVector->begin(); it!=metadataVector->end(); ++it ) {

			double low[3], high[3];

			low[0] = (*it)->md->partitionMbr.low[0];
			low[1] = (*it)->md->partitionMbr.low[1];
			low[2] = (*it)->md->partitionMbr.low[2];
			high[0] = (*it)->md->partitionMbr.high[0];
			high[1] = (*it)->md->partitionMbr.high[1];
			high[2] = (*it)->md->partitionMbr.high[2];

			SpatialIndex::Region* r = new SpatialIndex::Region(low, high, 3);

			if((*it)->begin) {
				rtreeIntersections rv;
				sweepLineTree->intersectsWithQuery(*r, rv);

				for(vector<RegionWrapper>::iterator mit = rv.me.begin(); mit!=rv.me.end(); ++mit) {
//					if((*it)->md->pageLinks.find(metadataStructure->at((*mit).id)->pageId) == (*it)->md->pageLinks.end()) {
//						cout << "error" << endl;
//					}
//
//					if(metadataStructure->at((*mit).id)->pageLinks.find((*it)->md->pageId) == metadataStructure->at((*mit).id)->pageLinks.end()) {
//						cout << "error" << endl;
//					}

					(*it)->md->pageLinks.insert(metadataStructure->at((*mit).id)->pageId);
					metadataStructure->at((*mit).id)->pageLinks.insert((*it)->md->pageId);
					links+=2;
				}

				byte* tmp = (byte*)&(*it)->md->pageId;

				byte* buf;

				buf = new byte[sizeof(int)];

				for(unsigned j=0; j<sizeof(int); j++) {
					buf[j] = tmp[j];
				}

				sweepLineTree->insertData(sizeof(int), buf, *r, (*it)->md->pageId);

				delete[] buf;
			} else {
				sweepLineTree->deleteData(*r, (*it)->md->pageId);
			}

			delete r;
		}

		delete sweepLineTree;
		delete memory;

#ifdef DEBUG
		uint32_t pages = metadataStructure->size();
		cout << "TOTAL PAGES: " << pages <<endl;
		cout << "TOTAL LINKS ADDED: "<< links <<endl;

		int frequency[100];
		for (int i=0;i<100;i++) frequency[i]=0;
		for (uint32_t j=0;j<pages;j++)
		{
			if (metadataStructure->at(j)->pageLinks.size()>80)
			{
				cout << "\nid("<< j<< ") = [" <<  metadataStructure->at(j)->i << "," << metadataStructure->at(j)->j << "," << metadataStructure->at(j)->k << "] \tLINKS:" << metadataStructure->at(j)->pageLinks.size() << " \tMBR" << metadataStructure->at(j)->partitionMbr << "\n";

				for (set<id>::iterator i = metadataStructure->at(j)->pageLinks.begin();i !=  metadataStructure->at(j)->pageLinks.end(); ++i)
				//for (uint32_t i=0;i<metadataStructure->at(j)->pageLinks.size();i++)
					cout << "\tid("<< *i << ") = [" <<  metadataStructure->at(*i)->i << "," << metadataStructure->at(*i)->j << "," << metadataStructure->at(*i)->k << "] \tLINKS:" << metadataStructure->at(*i)->pageLinks.size() << " \tMBR" << metadataStructure->at(*i)->partitionMbr << "\n";
			}
			if (metadataStructure->at(j)->pageLinks.size()<100)
				frequency[metadataStructure->at(j)->pageLinks.size()]++;
		}

//		for (int i=0;i<100;i++)
//			cout << "Links: " << i << " Frequency: " << frequency[i] << "\n";
#endif

//		for (vector<MetadataEntry*>::iterator first = metadataStructure->begin(); first != metadataStructure->end(); ++first) {
//			if ((*first)->pageLinks.size() > 0) {
//				cout << "PLS " << (*first)->pageId << " " << (*first)->pageLinks.size() << endl;
//			}
//		}
	}

	void STRIndex::loadIndex(string indexFileStem)
	{
		payload->load(indexFileStem);
		vector<SpatialObject*> items;
		for (int i=0;i<41;i++)
		{
			payload->getPage(items,i);
			items.clear();
		}
	}

}
