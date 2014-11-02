#include "tools.hpp"
#include "bbp_tools.hpp"

#include <cassert>
#include <iostream>
#include <limits>

#include <boost/format.hpp>

#include "AABBCylinder.h"
#include "mbr.hpp"

using namespace bbp;

namespace dias {

void serialize_vect_as_float (const dias::vect & v, byte ** serialized_ptr)
{
    for (int i = 0; i < 3; i++)
    {
        dias::serialize (static_cast<float>(v[i]), serialized_ptr);
    }
}

inline float sqr(float x)
{
  return x * x;
}

static bbp::Vector_3D<bbp::Micron> reference(0, 0, 0);

static float transform_distance (const dias::global_transformer & gtrafo,
                          float distance)
{
  bbp::Vector_3D<bbp::Micron> point_away(reference);
  point_away.x() += distance;
  bbp::Vector_3D<bbp::Micron> t1(gtrafo * reference);
  bbp::Vector_3D<bbp::Micron> t2(gtrafo * point_away);
  return sqrtf (sqr(t1.x() - t2.x()) + sqr(t1.y() - t2.y())
                + sqr(t1.z() - t2.z()));
}

dias::mbr get_segment_mbr (const bbp::Segment & s,
                           const dias::global_transformer & gtrafo)
{
  bbp::Vector_3D<bbp::Micron> gbegin = gtrafo * s.begin().center();
  bbp::Vector_3D<bbp::Micron> gend = gtrafo * s.end().center();

  float transformed_begin_radius = transform_distance(gtrafo,
                                                      s.begin().radius());
  float transformed_end_radius = transform_distance(gtrafo, s.end().radius());

  bbp::Box<bbp::Micron> box
    = bbp::AABBCylinder::calculateAABBForCylinder(gbegin,
                                                  transformed_begin_radius,
                                                  gend,
                                                  transformed_end_radius);
  vect low, high;
  low[0] = box.center().x() - box.dimensions().x() / 2;
  high[0] = box.center().x() + box.dimensions().x() / 2;
  low[1] = box.center().y() - box.dimensions().y() / 2;
  high[1] = box.center().y() + box.dimensions().y() / 2;
  low[2] = box.center().z() - box.dimensions().z() / 2;
  high[2] = box.center().z() + box.dimensions().z() / 2;

  return dias::mbr (low, high);
}

dias::vect get_segment_center (const bbp::Segment & s,
                               const dias::global_transformer & gtrafo)
{
    bbp::Vector_3D<bbp::Micron> gbegin = gtrafo * s.begin().center();
    bbp::Vector_3D<bbp::Micron> gend = gtrafo * s.end().center();

    dias::vect result;
    result[0] = (gbegin.x () + gend.x ()) / 2;
    result[1] = (gbegin.y () + gend.y ()) / 2;
    result[2] = (gbegin.z () + gend.z ()) / 2;

    return result;
}


SpatialIndex::Region
get_transformed_cube_mbr (const dias::micron_array & low,
                          const dias::micron_array & high,
                          const dias::global_transformer & inverse_transform)
{
    // Eight vertices of a cube
    Vector_3D<Micron> v[8];
    v[0] = Vector_3D<Micron>(low[0],low[1],low[2]);
    v[1] = Vector_3D<Micron>(high[0],low[1],low[2]);
    v[2] = Vector_3D<Micron>(low[0],high[1],low[2]);
    v[3] = Vector_3D<Micron>(low[0],low[1],high[2]);
    v[4] = Vector_3D<Micron>(high[0],high[1],high[2]);
    v[5] = Vector_3D<Micron>(low[0],high[1],high[2]);
    v[6] = Vector_3D<Micron>(high[0],low[1],high[2]);
    v[7] = Vector_3D<Micron>(high[0],high[1],low[2]);

    // Inverse Transform all
    for (int i=0; i<8 ;i++)
        v[i] = inverse_transform * v[i];

    // Build Covering Bounding Box on the Transformed MBR
    dias::mbr r;

    for (int i=0;i<8;i++)
    {
        r.expand (v[i]);
    }

    return r.as_region ();
}

dias::mbr get_triangle_mbr (const dias::triangle_ids & tids,
                            const dias::vertex_array & vertices,
                            const dias::global_transformer & gtrafo)
{
  assert (tids.size() == 3);

  dias::mbr result;

  for (size_t i = 0; i < tids.size(); i++)
  {
      const bbp::Vector_3D<bbp::Micron> v = gtrafo * vertices[tids[i]];
      result.expand (v);
  }
  return result;
}

dias::vect get_triangle_center(const dias::triangle_ids & tids,
                               const dias::vertex_array & vertices,
                               const dias::global_transformer & gtrafo)
{
    assert (tids.size() == 3);

    const bbp::Vector_3D<bbp::Micron> v1 = gtrafo * vertices[tids[0]];
    const bbp::Vector_3D<bbp::Micron> v2 = gtrafo * vertices[tids[1]];
    const bbp::Vector_3D<bbp::Micron> v3 = gtrafo * vertices[tids[2]];

    dias::vect result;

    result[0] = (v1.x () + v2.x () + v3.x ()) / 3;
    result[1] = (v1.y () + v2.y () + v3.y ()) / 3;
    result[2] = (v1.z () + v2.z () + v3.y ()) / 3;

    return result;
}

}
