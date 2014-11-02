/*
 * AABBCylinder.h
 *
 *  Created on: Sep 17, 2009
 *      Author: Thomas
 */

#include <BBP/Common/Math/Geometry/Box.h>
#include <BBP/Model/Microcircuit/Types.h>

#ifndef AABBCYLINDER_H_
#define AABBCYLINDER_H_

namespace bbp
{

template <class T> const T& max ( const T& a, const T& b ) {
  return (b<a)?a:b;
}

template <class T> const T& min ( const T& a, const T& b ) {
  return (a<b)?a:b;
}

class AABBCylinder
{
public:
	static Box<Micron> calculateAABBForCylinder(Vector_3D<Micron> begin, float bradius, Vector_3D<Micron> end, float eradius);
};
}

#endif /* AABBCYLINDER_H_ */
