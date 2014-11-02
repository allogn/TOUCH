
/*
 * AABBCylinder.cpp
 *
 *  Created on: Sep 17, 2009
 *      Author: Farhan before Thomas
 */


#include "AABBCylinder.h"

#include <limits>
namespace bbp {
	Box<Micron> AABBCylinder::calculateAABBForCylinder(Vector_3D<Micron> begin, float r1, Vector_3D<Micron> end, float r2)
	{
		//Farhan's implementation of the tightest bounding box
		float cx1=0,cy1=0,cz1=0,cx2=0,cy2=0,cz2=0;

		cx1 = begin.x();
		cx2 = end.x();
		cy1 = begin.y();
		cy2 = end.y();
		cz1 = begin.z();
		cz2 = end.z();

		// Tightest Bounding Box
		float D=0,Px=0,Py=0,Pz=0;

                D = sqrtf ( ((cx1-cx2)*(cx1-cx2)) + ((cy1-cy2)*(cy1-cy2))
                            + ((cz1-cz2)*(cz1-cz2)) );
                if (fabs (D) >= std::numeric_limits<float>::epsilon ())
                {
                    Px = sinf(acosf(fabsf (cx1-cx2) / D)) ;
                    Py = sinf(acosf(fabsf (cy1-cy2) / D)) ;
                    Pz = sinf(acosf(fabsf (cz1-cz2) / D)) ;
                }
                else
		{
                    Px=0;Py=0;Pz=0;
                }

		// Old way of Computing Bounding Box comment out if using Tightest Box
		//Px = 1;
	    //Py = 1;
	    //Pz = 1;

		//------------ remaining calculations

		// Small Box Around Endpoint 1
		float x11=0,y11=0,z11=0,x12=0,y12=0,z12=0;
		x11 = cx1-(Px*r1);
		y11 = cy1-(Py*r1);
		z11 = cz1-(Pz*r1);

		x12 = cx1+(Px*r1);
		y12 = cy1+(Py*r1);
		z12 = cz1+(Pz*r1);

		// Small Box Around Endpoint 2
		float x21=0,y21=0,z21=0,x22=0,y22=0,z22=0;
		x21 = cx2-(Px*r2);
		y21 = cy2-(Py*r2);
		z21 = cz2-(Pz*r2);

		x22 = cx2+(Px*r2);
		y22 = cy2+(Py*r2);
		z22 = cz2+(Pz*r2);

		// Combining both boxes to get Minimum bounding box
		float x1=0,y1=0,z1=0,x2=0,y2=0,z2=0;
		if (x11<x21) x1 = x11; else x1 = x21;
		if (y11<y21) y1 = y11; else y1 = y21;
		if (z11<z21) z1 = z11; else z1 = z21;

		if (x12>x22) x2 = x12; else x2 = x22;
		if (y12>y22) y2 = y12; else y2 = y22;
		if (z12>z22) z2 = z12; else z2 = z22;

		// Transforming into Box Coordinates
		float x = x1 + (fabsf(x2-x1))/2;
		float y = y1 + (fabsf(y2-y1))/2;
		float z = z1 + (fabsf(z2-z1))/2;

		float dx = x2-x1;
		float dy = y2-y1;
		float dz = z2-z1;

		Box<Micron> bounding_box(x, y, z, dx, dy, dz);

		return bounding_box;
	}
}

