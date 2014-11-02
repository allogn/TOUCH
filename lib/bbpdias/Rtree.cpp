#ifndef R_TREE_
#define R_TREE_

#include <cstring>
#include <iostream>
#include <algorithm>
#include <SpatialIndex.h>
#include "libbbpdias/spatialindex_tools.hpp"
#include "libbbpdias/Vertex.hpp"
#include "libbbpdias/Box.hpp"
#include "Query.cpp"
#include "Data.cpp"

namespace dias {
/*
 * R tree
 * Based on Spatial Indexing Library
 * each Leaf is a bucket of Data with MAX DATA size
 *
 * Author: Farhan Tauheed
 */

class rtreeVisitor : public SpatialIndex::IVisitor
{
public:
    Query* query;
    //bool seedFound;
    //bool seedMode;
    //bool rangeMode;
    //bool validateMode;

    rtreeVisitor(Query* queryPtr)
//    rtreeVisitor(Query* queryPtr/*,bool seed,bool range,bool validate*/)
		{
			query = queryPtr;
    		//seedMode     = seed;
    		//rangeMode    = range;
    		//validateMode = validate;
    		//seedFound=false;
		}

    virtual void visitNode (const SpatialIndex::INode& in)
		{
    	query->VisitNode(in.getChildrenCount());
/*
    	if (validateMode==true) return;

    	if (rangeMode==true) {query->VisitRangeNode();return;}

    	if (seedFound==false && seedMode==true)
    		if (in.isLeaf()==false)query->VisitNode();*/
		}

    virtual void visitData (const SpatialIndex::IData& in)
		{
    	SpatialIndex::Region mbr;
    	SpatialIndex::IShape* mb;
    	in.getShape(&mb  );
    	mb->getMBR(mbr);
    	Data d = Data(Vertex((spaceUnit)mbr.getLow(0),(spaceUnit)mbr.getLow(1),(spaceUnit)mbr.getLow(2)),(int)in.getIdentifier());
//        std::cerr << d << '\n';
    	query->VisitLeaf(d);
    	delete mb;

    	/*if (validateMode==true) {query->VisitValidateLeaf(d);return;}

    	if (rangeMode==true) {query->VisitRangeLeaf(d);return;}

    	if (Box::enclose((*query).box,d.midPoint)==true && seedFound==false &&  seedMode==true)
			{
    		query->VisitLeaf(d);
    		seedFound=true;
			}*/
		}

    virtual void visitData (std::vector<const SpatialIndex::IData *>& v
                            __attribute__((__unused__)))
		{
		}
};


    class Rtree
    {
    public:
        std::string baseName;
        SpatialIndex::ISpatialIndex *tree;
        SpatialIndex::IStorageManager* storageManager;
        bool newTree;

        Rtree(const std::string &filename,bool create, unsigned cache_size = 0)
            {
                SpatialIndex::id_type id = 1;
                newTree = create;
                baseName = filename;
                if (create == true)
                {
//std::cout << "\nCreating R*Tree: " << baseName << "\n";
                    storageManager
                        = SpatialIndex::StorageManager::createNewDiskStorageManager(
                            baseName, default_page_size);
                    SpatialIndex::IStorageManager & storage = *storageManager;
                    if (cache_size > 0)
                    {
// FIXME: memory leak
                        SpatialIndex::StorageManager::IBuffer * buf
                            = SpatialIndex::StorageManager::createNewRandomEvictionsBuffer
                            (storage, cache_size, false);
                        storage = *buf;
                        tree = SpatialIndex::RTree::createNewRTree (
                            *buf,
                            fill_factor,
                            dias::index_node_capacity,
                            dias::leaf_node_capacity,
                            3,
                            SpatialIndex::RTree::RV_RSTAR,
                            id
                            );
                    }
                    else
                    {
                        tree = SpatialIndex::RTree::createNewRTree (
                            storage,
                            fill_factor,
                            dias::index_node_capacity,
                            dias::leaf_node_capacity,
                            3,
                            SpatialIndex::RTree::RV_RSTAR,
                            id
                            );
                    }
                }
                else
                {
                    storageManager
                        = SpatialIndex::StorageManager::loadDiskStorageManager(
                            baseName);
                    SpatialIndex::IStorageManager & storage = *storageManager;
                    if (cache_size > 0)
                    {
                        SpatialIndex::StorageManager::IBuffer * buf
                            = SpatialIndex::StorageManager::createNewRandomEvictionsBuffer
                            (storage, cache_size, false);
                        storage = *buf;
                        tree = SpatialIndex::RTree::loadRTree(*buf, id);
                    }
                    else
                    {
                        tree = SpatialIndex::RTree::loadRTree(storage, id);
                    }
                }
            }

	~Rtree()
            {
                delete tree;
                delete storageManager;
            }

            void Insert(const Data &data)
                {
                    if (newTree==false) return;

                    double mid[DIMENSION];
                    for (int i=0;i<DIMENSION;i++)
                        mid[i] = (double)data.midPoint[i];

                    SpatialIndex::Region mbr = SpatialIndex::Region(mid,mid,3);
                    tree->insertData (0, NULL, mbr, data.id);
                }

	void Analyze()
		{
	    if (!tree->isIndexValid()) std::cerr << "R-tree internal checks failed!\n"; else std::cout << "OK\n";
	    SpatialIndex::IStatistics * tree_stats;
	    tree->getStatistics (&tree_stats);
	    std::cout << *tree_stats;
		}

        void Seed(Query &query)
            {
                rtreeVisitor visitor = rtreeVisitor(&query);
                double lo[DIMENSION],hi[DIMENSION];
                for (int i=0;i<DIMENSION;i++)
                {
                    lo[i] = (double)query.box.low[i];
                    hi[i] = (double)query.box.high[i];
                }
                SpatialIndex::Region query_region = SpatialIndex::Region(lo,hi,DIMENSION);
//tree->containsWhatQuery(query_region,visitor);
                SpatialIndex::IStatistics * stats;
                tree->getStatistics (&stats);
                size_t reads_before = stats->getReads ();
                delete stats;
                tree->seedQuery (query_region, visitor);
                tree->getStatistics (&stats);
                size_t reads_after = stats->getReads ();
                delete stats;
                query.containerAccesses = reads_after - reads_before;
            }

	void RangeSearch(Query &query)
            {
                rtreeVisitor visitor = rtreeVisitor(&query);
                double lo[DIMENSION],hi[DIMENSION];
                for (int i=0;i<DIMENSION;i++)
                {
                    lo[i] = (double)query.box.low[i];
                    hi[i] = (double)query.box.high[i];
                }
                SpatialIndex::Region query_region = SpatialIndex::Region(lo,hi,DIMENSION);
// TODO: the search algorithm reads nodes before putting them on the search
// queue but the search algorithm is finished with non-empty queue so we
// don't get to register some of the read nodes.
                SpatialIndex::IStatistics * stats;
                tree->getStatistics (&stats);
                size_t reads_before = stats->getReads ();
                delete stats;
                tree->intersectsWithQuery (query_region, visitor);
                tree->getStatistics (&stats);
                size_t reads_after = stats->getReads ();
                delete stats;
                query.containerAccesses = reads_after - reads_before;
                }

        unsigned get_reads (void) const
            {
                return storageManager->get_reads();
            }

        unsigned get_writes (void) const
            {
                return storageManager->get_writes();
            }


/*
	void Validate(Query &query)
		{
		rtreeVisitor visitor = rtreeVisitor(&query,false,false,true);
		double lo[DIMENSION],hi[DIMENSION];
		for (int i=0;i<DIMENSION;i++)
			{
			lo[i] = (double)query.box.low[i];
			hi[i] = (double)query.box.high[i];
			}
		SpatialIndex::Region query_region = SpatialIndex::Region(lo,hi,3);
		tree->intersectsWithQuery(query_region,visitor);
		}*/
	};
}
#endif
