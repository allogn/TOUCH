#ifndef STR_INDEX_HPP
#define STR_INDEX_HPP

#include "Metadata.hpp"
#include "DataFileReader.hpp"
#include "PayLoad.hpp"
#include "SeedBuilder.hpp"

namespace FLAT
{

#define PAGE_SIZE 4096
#define SORTING_FOOTPRINT_MB 400

	class RegionWrapper
	{
	public:
		SpatialIndex::Region region;
		int id;

		RegionWrapper(SpatialIndex::Region r, int i) {
			region = r;
			id = i;
		}
	};

	class rtreeIntersections : public SpatialIndex::IVisitor
	{
	public:
		vector<RegionWrapper> me;

		virtual void visitNode(const SpatialIndex::INode& in) {
		}

		virtual bool doneVisiting() {
			return false;
		};

		virtual void visitData(const SpatialIndex::IData& in) {
				byte *b;
				uint32_t l;
				in.getData(l, &b);

				int val = *reinterpret_cast<int *>(b);

				SpatialIndex::Region mbr;
				SpatialIndex::IShape* mb;
				in.getShape(&mb);
				mb->getMBR(mbr);

				me.push_back(RegionWrapper(mbr, val));

				delete mb;
				delete[] b;
		}

		virtual void visitData(const SpatialIndex::IData& in, SpatialIndex::id_type id) {
		}

		virtual void visitData(std::vector<const SpatialIndex::IData *>& v
								__attribute__((__unused__))) {
		}
	};

	class MetadataWrapper
	{
	public:
		MetadataEntry* md;
		bool begin;

		MetadataWrapper(MetadataEntry* metadataStructure, bool high) {
			md = metadataStructure;
			begin = high;
		}
	};

	/*
	 * Class responsible for managing the Index structure, i.e Metadata, SeedTree and Payload
	 * This is an STR Style FLAT indexing
	 */
	class STRIndex
	{
	public:
		vector<MetadataEntry*>* metadataStructure;     	// Array of meta data entries leaf Level of Seed Tree
		vector<MetadataWrapper*>* metadataVector;
		PayLoad*    	payload;	   		   			// The Structure managing the payload
		int objectCount;
		int objectSize;
		SpatialObjectType objectType;
		int pageCount;
		Box universe;
		float binCount;
		int objectPerPage;
		int objectPerXBins;
		int objectPerYBins;

		STRIndex();

		~STRIndex();

		void buildIndex(SpatialObjectStream* input,string indexFileStem);

		void loadIndex(string indexFileStem);

	private:
		void initialize(SpatialObjectStream* input,string indexFileStem);

		void doTessellation(SpatialObjectStream* input);

		void induceConnectivity();

		void induceConnectivityFaster();

	};
}
#endif
