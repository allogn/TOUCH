#include <cstring>

#include "spatialindex_tools.hpp"

#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include "access_count_visitor.hpp"

namespace dias {

float perform_random_point_queries (SpatialIndex::ISpatialIndex * tree,
                                    const SpatialIndex::Region & query_region,
                                    const unsigned number_of_queries)
{
    boost::mt11213b generator (42u);
    const double x[3] = {0.0, 0.0, 0.0};

    access_count_visitor v;
    for (unsigned j = 0; j < number_of_queries; j++)
    {
        SpatialIndex::Point rnd_point (x, query_region.m_dimension);
        for (size_t i = 0; i < query_region.m_dimension; i++)
        {
            boost::uniform_real<> uni_dist (query_region.m_pLow[i],
                                            query_region.m_pHigh[i]);
            boost::variate_generator<boost::mt11213b &,
                boost::uniform_real<> > uni(generator, uni_dist);
            rnd_point.m_pCoords[i] = uni();
        }
        assert (query_region.containsPoint (rnd_point));

        v.new_query();
        tree->pointLocationQuery (rnd_point, v);
    }
    v.print_stats();
    return v.get_avg_io();
}

}
