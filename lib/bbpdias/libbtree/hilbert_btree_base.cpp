#include <cassert>

#include <boost/make_shared.hpp>

#include "hilbert_btree_base.hpp"

using namespace dias;

static inline bitmask_t
scale_point (double p, double lower_p_bound, double p_interval)
{
    return static_cast<bitmask_t> ((p - lower_p_bound)
                                   * std::numeric_limits<bitmask_t>::max ()
                                   / p_interval);
}

hilbert_btree_base::discrete_point
hilbert_btree_base::expand_to_space (const SpatialIndex::Point & p) const
{
    discrete_point result;
    result[0] = scale_point (p.m_pCoords[0], world_start.m_pCoords[0],
                             world_size.m_pCoords[0]);
    result[1] = scale_point (p.m_pCoords[1], world_start.m_pCoords[1],
                             world_size.m_pCoords[1]);
    result[2] = scale_point (p.m_pCoords[2], world_start.m_pCoords[2],
                             world_size.m_pCoords[2]);
    return result;
}

bitmask_t
hilbert_btree_base::point_to_hilbert_value (const SpatialIndex::Point & p,
                                            size_t bits_per_dimension) const
{
    discrete_point d_p = expand_to_space (p);

#if 0
    boost::array<long, 3> c;

// BBP data-specific hack to avoid negative values
// * 500 is for mesh, remove for segments.
    c[0] = static_cast<long>(p.m_pCoords[0] * 500) + 1699 * 500;
    c[1] = static_cast<long>(p.m_pCoords[1] * 500) + 1065 * 500;
    c[2] = static_cast<long>(p.m_pCoords[2] * 500) + 1725 * 500;

    if ((c[0] < 0) || (c[1] < 0) || (c[2] < 0))
        assert (false);

    d_p[0] = c[0];
    d_p[1] = c[1];
    d_p[2] = c[2];
#endif


    return hilbert_c2i (3, bits_per_dimension, d_p.data ());
}

static inline
SpatialIndex::Point get_region_size (const SpatialIndex::Region & r)
{
    double coords[3];
    coords[0] = r.m_pHigh[0] - r.m_pLow[0];
    coords[1] = r.m_pHigh[1] - r.m_pLow[1];
    coords[2] = r.m_pHigh[2] - r.m_pLow[2];
    return SpatialIndex::Point (coords, 3);
}

static inline
SpatialIndex::Point get_low_region_point (const SpatialIndex::Region & r)
{
    return SpatialIndex::Point (r.m_pLow, 3);
}

hilbert_btree_base::hilbert_btree_base (
    const SpatialIndex::Region & world_extent,
    size_t dimension __attribute__((unused)),
    const std::string & hilbert_value_output_fn) :
//    duplicates (0),
    world_size (get_region_size (world_extent)),
    world_start (get_low_region_point (world_extent))
{
    assert (dimension == 3); // FIXME
    if (hilbert_value_output_fn != "")
        hilbert_value_log.open (hilbert_value_output_fn.c_str (),
                                std::ios::binary);
}

hilbert_btree_base::~hilbert_btree_base (void)
{
    hilbert_value_log.close ();
}

void
hilbert_btree_base::log_hilbert_value (const bitmask_t hilbert_value)
{
    if (hilbert_value_log.is_open ())
        hilbert_value_log << hilbert_value << '\n';
}

bool
hilbert_btree_base::deleteData (const SpatialIndex::IShape & shape
                                __attribute__((unused)),
                                SpatialIndex::id_type shapeIdentifier
                                __attribute__((unused)))
{
    assert (false);
    return false;
}

void
hilbert_btree_base::containsWhatQuery (const SpatialIndex::IShape & query
                                       __attribute__((unused)),
                                       SpatialIndex::IVisitor & v
                                       __attribute__((unused)))
{
    assert (false);
}

void
hilbert_btree_base::intersectsWithQuery (const SpatialIndex::IShape& query
                                         __attribute__((unused)),
                                         SpatialIndex::IVisitor& v
                                         __attribute__((unused)))
{
    assert (false);
}

void hilbert_btree_base::nearestNeighborQuery (
    uint32_t k __attribute__((unused)),
    const SpatialIndex::IShape& query __attribute__((unused)),
    SpatialIndex::IVisitor& v __attribute__((unused)),
    SpatialIndex::INearestNeighborComparator& nnc __attribute__((unused)))
{
    assert (false);
}

void
hilbert_btree_base::nearestNeighborQuery (
    uint32_t k __attribute__((unused)),
    const SpatialIndex::IShape& query __attribute__((unused)),
    SpatialIndex::IVisitor& v __attribute__((unused)))
{
    assert (false);
}

void
hilbert_btree_base::selfJoinQuery (
    const SpatialIndex::IShape& s __attribute__((unused)),
    SpatialIndex::IVisitor& v __attribute__((unused)))
{
    assert (false);
}

void
hilbert_btree_base::queryStrategy (
    SpatialIndex::IQueryStrategy& qs __attribute__((unused)))
{
    assert (false);
}

void
hilbert_btree_base::getIndexProperties (
    Tools::PropertySet& out __attribute__((unused))) const
{
    assert (false);
}

void
hilbert_btree_base::addCommand (
    SpatialIndex::ICommand* in __attribute__((unused)),
    SpatialIndex::CommandType ct __attribute__((unused)))
{
    assert (false);
}

