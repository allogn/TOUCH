#include "unlimited_degree_hilbertbtree.hpp"

#include <exception>
#include <boost/foreach.hpp>

#include "libbbpdias/tools.hpp"

using namespace dias;

unlimited_degree_hilbert_btree::unlimited_degree_hilbert_btree (
    SpatialIndex::IStorageManager & storage,
    unsigned page_size, SpatialIndex::id_type header_page_id,
    const SpatialIndex::Region & world_extent,
    size_t dimension,
    const std::string & hilbert_value_output_fn) :
    hilbert_btree<float_vect, hilbert_less> (storage, page_size,
                                             header_page_id, world_extent,
                                             dimension,
                                             hilbert_value_output_fn)
{
}

unlimited_degree_hilbert_btree::unlimited_degree_hilbert_btree (
    SpatialIndex::IStorageManager & storage,
    SpatialIndex::id_type header_page_id,
    const SpatialIndex::Region & world_extent,
    size_t dimension,
    const std::string & hilbert_value_output_fn) :
    hilbert_btree<float_vect, hilbert_less> (storage, header_page_id,
                                             world_extent, dimension,
                                             hilbert_value_output_fn)
{
}

void unlimited_degree_hilbert_btree::insertData (
    size_t len,
    const byte* pData __attribute__((unused)),
    const SpatialIndex::IShape & shape,
    SpatialIndex::id_type shapeIdentifier)
{
    const SpatialIndex::Point * point
        = dynamic_cast<const SpatialIndex::Point *>(&shape);

    log_hilbert_value (point_to_hilbert_value (*point,
                                               sizeof (bitmask_t) * 8 / 3));

// FIXME: should scale here and leave floating point?
    float_vect p;
    p[0] = point->m_pCoords[0];
    p[1] = point->m_pCoords[1];
    p[2] = point->m_pCoords[2];
    tree.insert (p, payload (shapeIdentifier, len));
}

void unlimited_degree_hilbert_btree::pointLocationQuery (
    const SpatialIndex::Point& query,
    SpatialIndex::IVisitor& v)
{
    float_vect p;
    p[0] = query.m_pCoords[0];
    p[1] = query.m_pCoords[1];
    p[2] = query.m_pCoords[2];

    query_and_visit_query_results (p, v);
}

boost::shared_ptr<SpatialIndex::ISpatialIndex>
unlimited_degree_hilbert_btree::make_unlimited_degree_hilbert_btree (
    SpatialIndex::IStorageManager & storage,
    unsigned page_size, SpatialIndex::id_type header_page_id,
    const SpatialIndex::Region & world_extent,
    size_t dimension, const std::string & hilbert_value_output_fn)
{
    return boost::shared_ptr<SpatialIndex::ISpatialIndex> (
        new unlimited_degree_hilbert_btree (storage, page_size, header_page_id,
                                            world_extent, dimension,
                                            hilbert_value_output_fn));
}

boost::shared_ptr<SpatialIndex::ISpatialIndex>
unlimited_degree_hilbert_btree::load_unlimited_degree_hilbert_btree (
    SpatialIndex::IStorageManager & storage,
    SpatialIndex::id_type header_page_id,
    const SpatialIndex::Region & world_extent,
    size_t dimension, const std::string & hilbert_value_output_fn)
{
    return boost::shared_ptr<SpatialIndex::ISpatialIndex> (
        new unlimited_degree_hilbert_btree (storage, header_page_id,
                                            world_extent, dimension,
                                            hilbert_value_output_fn));
}
