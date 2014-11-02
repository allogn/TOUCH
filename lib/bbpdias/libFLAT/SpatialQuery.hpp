#ifndef SPATIAL_QUERY
#define SPATIAL_QUERY


#include <vector>
#include "Box.hpp"

//#define RESULTS

namespace FLAT {
/*
 * Query structure for Spatial Indexing
 * Query Results
 * and Performance Statistics
 *
 * Author: Farhan Tauheed
 */
    class SpatialQuery
    {
    public:
        Box box;
        std::vector<SpatialObject*> results;    // List of Data in Query by Algorithm

        int points;
        int IOs;

        SpatialQuery() :
        points (0),
        IOs(0)
            {
            }

        SpatialQuery (const Box & b) :
            box (b),
            points (0),
            IOs (0)
            {
            }

	void VisitLeaf(const Data& data);
	void VisitNode(int dataCount);
    };

}
#endif
