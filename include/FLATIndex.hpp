#ifndef FLAT_INDEX_HPP_
#define FLAT_INDEX_HPP_

#include <SpatialIndex.h>
#include "PayLoad.hpp"
#include "SeedBuilder.hpp"
#include "SpatialQuery.hpp"

namespace FLAT
{
	class seedVisitor : public SpatialIndex::IVisitor
	{
	public:
		SpatialQuery* query;
		bool done;
		PayLoad* payload;

		seedVisitor(SpatialQuery* query, string indexFileStem)
		{
			this->query = query;
			done = false;
			payload   = new PayLoad();
			payload->load(indexFileStem);
		}

		~seedVisitor()
		{
			delete payload;
		}

		virtual void visitNode(const SpatialIndex::INode& in)
		{
	    	if (in.isLeaf())
	    	{
	    		query->stats.FLAT_metaDataIOs++;
	    	}
	    	else
	    	{
	    		query->stats.FLAT_seedIOs++;
	    	}
		}

		virtual void visitData(const SpatialIndex::IData& in)
		{
		}

		virtual void visitUseless()
		{
		}

		virtual bool doneVisiting()
		{
			return done;
		}

		virtual void visitData(const SpatialIndex::IData& in, SpatialIndex::id_type id)
		{
			FLAT::uint8 *b;
			uint32 l;
			in.getData(l, &b);

			MetadataEntry m = MetadataEntry(b, l);
			delete[] b;

			vector<SpatialObject*> so;
			payload->getPage(so, m.pageId);
			for (vector<SpatialObject*>::iterator it = so.begin(); it != so.end(); ++it)
				if (Box::overlap(query->Region, (*it)->getMBR()))
				{
					//cout << "Found seed : " << id << endl;
					done = true;
					query->stats.FLAT_seedId = id;
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

	class kNNVisitor : public SpatialIndex::IVisitor
	{
	public:
		SpatialQuery* query;
		bool done;

		kNNVisitor(SpatialQuery* query, string indexFileStem)
		{
			this->query = query;
			done = false;
		}

		~kNNVisitor()
		{
		}

		virtual void visitNode(const SpatialIndex::INode& in)
		{
//			cout << "visited node\n";
//			uint32 children = in.getChildrenCount();
//
//			Box Parent;
//			SpatialIndex::IShape* shape;
//			in.getShape(&shape);
//			SpatialIndex::Region reg;
//			shape->getMBR(reg);
//			for (int d=0;d<DIMENSION;d++)
//			{
//				Parent.low[d] = reg.m_pLow[d];
//				Parent.high[d] = reg.m_pHigh[d];
//			}
//			cout << "PARENT NODE: LEVEL= " << in.getLevel() << " ID= " << in.getIdentifier() << "   MBR= " << Parent << endl;
//			delete shape;
//
//			for (int i=0;i<children;i++)
//			{
//				Box Child;
//				SpatialIndex::IShape* childShape;
//				in.getChildShape(i,&childShape);
//				SpatialIndex::Region reg;
//				childShape->getMBR(reg);
//				for (int d=0;d<DIMENSION;d++)
//				{
//					Child.low[d] = reg.m_pLow[d];
//					Child.high[d] = reg.m_pHigh[d];
//				}
//				cout << "\tCHILD NODE: ID= " << in.getChildIdentifier(i) << "   MBR= " << Child << endl;
//				delete childShape;
//			}

	    	if (in.isLeaf())
	    	{
	    		query->stats.FLAT_metaDataIOs++;
	    	}
	    	else
	    	{
	    		query->stats.FLAT_seedIOs++;
	    	}
		}

		virtual void visitData(const SpatialIndex::IData& in)
		{
		}

		virtual void visitUseless()
		{
		}

		virtual bool doneVisiting()
		{
			return done;
		}

		virtual void visitData(const SpatialIndex::IData& in, SpatialIndex::id_type id)
		{
			done = true;
			query->stats.FLAT_seedId = id;
		}

		virtual void visitData(std::vector<const SpatialIndex::IData *>& v
								__attribute__((__unused__)))
		{
		}
	};

	class kNNEntry
	{
	public:

		SpatialObject* sobj;
		MetadataEntry* me;
		bigSpaceUnit minDist;
		id metapageId;
		id metaentryId;
		bool isMetaPage;

		kNNEntry(SpatialObject* object,Vertex P,id metaPageId,id metaEntryId)
		{
			sobj = object;
			minDist = sobj->pointDistance(P);
			isMetaPage=false;
			me = NULL;
			metapageId = metaPageId;
			metaentryId = metaEntryId;
		}

		kNNEntry(MetadataEntry* metaEntry,Vertex P,id metaPageId)
		{
			me = metaEntry;
			minDist = me->partitionMbr.pointDistance(P);
			isMetaPage=true;
			sobj=false;
			metapageId = metaPageId;
		}

		~kNNEntry()
		{}

		struct ascending : public std::binary_function<kNNEntry*, kNNEntry*, bool>
		{
			bool operator()(const kNNEntry* __x, const kNNEntry* __y) const
			{
				return __x->minDist > __y->minDist;
			}
		};
	};

	class FLATIndex
	{
	public:
	   string indexName;
	   PayLoad* payload;
	   SpatialIndex::ISpatialIndex* seedtree;
	   SpatialIndex::IStorageManager* rtreeStorageManager;

	   static void buildIndexWithCount(std::string sourceConfig,string type,std::string indexStem,int count);

	   static void buildIndex(std::string sourceConfig,string type,std::string indexStem);

	   static FLATIndex* loadIndex(std::string indexStem);

	   static FLATIndex* loadIndexMemory(std::string indexStem);

	   static std::vector< vector<float> > windowQuery(FLATIndex* index,float xlo,float ylo,float zlo,float xhi,float yhi,float zhi);

	   static std::vector< vector<float> > vicinityQuery(FLATIndex* index,float centerX,float centerY,float centerZ,float vicinityRadius);

	   static std::vector< vector<float> > kNNQuery(FLATIndex* index,float centerX,float centerY,float centerZ,uint32 k);

	   static std::vector<int> getNeuronIds(FLATIndex* index,float xlo,float ylo,float zlo,float xhi,float yhi,float zhi);

	   static void rangeQuery(FLATIndex* index,SpatialQuery& query,vector<SpatialObject*>& results);

	   static void kNearestNeighbour(FLATIndex* index,SpatialQuery& query,vector<SpatialObject*>& results, uint32 k);

	   static void unLoadIndex(FLATIndex* index);
	};
}
#endif
