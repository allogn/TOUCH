#ifndef STR_INDEX_HPP
#define STR_INDEX_HPP

#include "Metadata.hpp"
#include "DataFileReader.hpp"
#include "PayLoad.hpp"
#include "SeedBuilder.hpp"
#include "ExternalSort.hpp"
#include "Timer.hpp"

#define SORTING_FOOTPRINT_MB 400

/*
 * Class responsible for managing the Index structure, i.e Metadata, SeedTree and Payload
 * This is an STR Style FLAT indexing
 */
class STRIndex
{
public:
        vector<FLAT::MetadataEntry*>* metadataStructure;     	// Array of meta data entries leaf Level of Seed Tree
        FLAT::PayLoad*    	payload;	   		   			// The Structure managing the payload
        FLAT::uint64 objectCount;
        FLAT::uint32 objectSize;
        FLAT::SpatialObjectType objectType;
        FLAT::uint64 pageCount;
        FLAT::Box universe;
        float binCount;
        FLAT::uint64 objectPerPage;
        FLAT::uint64 objectPerXBins;
        FLAT::uint64 objectPerYBins;

        STRIndex();

        ~STRIndex();

        void buildIndex(FLAT::SpatialObjectStream* input,string indexFileStem);

        void loadIndex(string indexFileStem);

private:
        void initialize(FLAT::SpatialObjectStream* input,string indexFileStem);

        void doTessellation(FLAT::SpatialObjectStream* input);

        void induceConnectivity();

        void induceConnectivityFaster();

};
#endif
