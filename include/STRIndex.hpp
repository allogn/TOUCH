#ifndef STR_INDEX_HPP
#define STR_INDEX_HPP

#include "Metadata.hpp"
#include "DataFileReader.hpp"
#include "PayLoad.hpp"
#include "SeedBuilder.hpp"

namespace FLAT
{
#define SORTING_FOOTPRINT_MB 400

	/*
	 * Class responsible for managing the Index structure, i.e Metadata, SeedTree and Payload
	 * This is an STR Style FLAT indexing
	 */
	class STRIndex
	{
	public:
		vector<MetadataEntry*>* metadataStructure;     	// Array of meta data entries leaf Level of Seed Tree
		PayLoad*    	payload;	   		   			// The Structure managing the payload
		uint64 objectCount;
		uint32 objectSize;
		SpatialObjectType objectType;
		uint64 pageCount;
		Box universe;
		float binCount;
		uint64 objectPerPage;
		uint64 objectPerXBins;
		uint64 objectPerYBins;

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
