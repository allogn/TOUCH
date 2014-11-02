/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible author:     Juan Hernando Vieites
        Contributing author:    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_ROTATION_H
#define BBP_ROTATION_H

#include <iostream>
#include "BBP/Common/Types.h"
#include "Orientation.h"
#include "Vector_3D.h"
#include "BBP/Common/Math/Matrix.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Geometric transformation of a 3D vector.
/*!
    Matrix representing the affine transformation.
         0  1  2  3     x    x*0 + y*1 + z*3 + w*3
         4  5  6  7     y    x*4 + y*5 + z*6 + w*7
         8  9 10 11  *  z =  ...
        12 13 14 15     w

    \todo Review with Juan if double precision is 
    really needed or if float is sufficient. (TT)
    \ingroup Math Geometry
*/
template <typename Space_Unit>
class Transform_3D
    : public Matrix<Space_Unit, 4, 4>
{
public:
    Transform_3D()
    {
        this->identity();
    }

    //! Affine transformation of a 3D vector.
    Vector_3D<Space_Unit> operator * 
        (const Vector_3D<Space_Unit> & rhs) const
    {
        return Vector_3D<Space_Unit> (
             (Space_Unit)(rhs.x() * this->_matrix[0] + 
                          rhs.y() * this->_matrix[1] + 
                          rhs.z() * this->_matrix[2] +
                                    this->_matrix[3]),
             (Space_Unit)(rhs.x() * this->_matrix[4] + 
                          rhs.y() * this->_matrix[5] + 
                          rhs.z() * this->_matrix[6] + 
                                    this->_matrix[7]),
             (Space_Unit)(rhs.x() * this->_matrix[8] + 
                          rhs.y() * this->_matrix[9] + 
                          rhs.z() * this->_matrix[10] +
                                    this->_matrix[11])
            );
    }

    //! Multiply two transformation matrices.
    Transform_3D operator * (const Transform_3D<Space_Unit> & rhs) const
    {
        Transform_3D result;

        // first row
        result._matrix[0] = this->_matrix[0] * rhs._matrix[0] +
                            this->_matrix[1] * rhs._matrix[4] +
                            this->_matrix[2] * rhs._matrix[8] +
                            this->_matrix[3] * rhs._matrix[12];
        result._matrix[1] = this->_matrix[0] * rhs._matrix[1] +
                            this->_matrix[1] * rhs._matrix[5] +
                            this->_matrix[2] * rhs._matrix[9] +
                            this->_matrix[3] * rhs._matrix[13];
        result._matrix[2] = this->_matrix[0] * rhs._matrix[2] +
                            this->_matrix[1] * rhs._matrix[6] +
                            this->_matrix[2] * rhs._matrix[10] +
                            this->_matrix[3] * rhs._matrix[14];
        result._matrix[3] = this->_matrix[0] * rhs._matrix[3] +
                            this->_matrix[1] * rhs._matrix[7] +
                            this->_matrix[2] * rhs._matrix[11] +
                            this->_matrix[3] * rhs._matrix[15];

        // second row
        result._matrix[4] = this->_matrix[4] * rhs._matrix[0] +
                            this->_matrix[5] * rhs._matrix[4] +
                            this->_matrix[6] * rhs._matrix[8] +
                            this->_matrix[7] * rhs._matrix[12];
        result._matrix[5] = this->_matrix[4] * rhs._matrix[1] +
                            this->_matrix[5] * rhs._matrix[5] +
                            this->_matrix[6] * rhs._matrix[9] +
                            this->_matrix[7] * rhs._matrix[13];
        result._matrix[6] = this->_matrix[4] * rhs._matrix[2] +
                            this->_matrix[5] * rhs._matrix[6] +
                            this->_matrix[6] * rhs._matrix[10] +
                            this->_matrix[7] * rhs._matrix[14];
        result._matrix[7] = this->_matrix[4] * rhs._matrix[3] +
                            this->_matrix[5] * rhs._matrix[7] +
                            this->_matrix[6] * rhs._matrix[11] +
                            this->_matrix[7] * rhs._matrix[15];

        // third row
        result._matrix[8] = this->_matrix[8] * rhs._matrix[0] +
                            this->_matrix[9] * rhs._matrix[4] +
                            this->_matrix[10] * rhs._matrix[8] +
                            this->_matrix[11] * rhs._matrix[12];
        result._matrix[9] = this->_matrix[8]  * rhs._matrix[1] +
                            this->_matrix[9]  * rhs._matrix[5] +
                            this->_matrix[10] * rhs._matrix[9] +
                            this->_matrix[11] * rhs._matrix[13];
        result._matrix[10] = this->_matrix[8]  * rhs._matrix[2] +
                            this->_matrix[9]  * rhs._matrix[6] +
                            this->_matrix[10] * rhs._matrix[10] +
                            this->_matrix[11] * rhs._matrix[14];
        result._matrix[11] = this->_matrix[8]  * rhs._matrix[3] +
                            this->_matrix[9]  * rhs._matrix[7] +
                            this->_matrix[10] * rhs._matrix[11] +
                            this->_matrix[11] * rhs._matrix[15];

        // forth row
        result._matrix[12] = this->_matrix[12] * rhs._matrix[0] +
                             this->_matrix[13] * rhs._matrix[4] +
                             this->_matrix[14] * rhs._matrix[8] +
                             this->_matrix[15] * rhs._matrix[12];
        result._matrix[13] = this->_matrix[12] * rhs._matrix[1] +
                             this->_matrix[13] * rhs._matrix[5] +
                             this->_matrix[14] * rhs._matrix[9] +
                             this->_matrix[15] * rhs._matrix[13];
        result._matrix[14] = this->_matrix[12] * rhs._matrix[2] +
                             this->_matrix[13] * rhs._matrix[6] +
                             this->_matrix[14] * rhs._matrix[10] +
                             this->_matrix[15] * rhs._matrix[14];
        result._matrix[15] = this->_matrix[12] * rhs._matrix[3] +
                             this->_matrix[13] * rhs._matrix[7] +
                             this->_matrix[14] * rhs._matrix[11] +
                             this->_matrix[15] * rhs._matrix[15];
        return result;
    }

    inline bool operator == (const Transform_3D & rhs) const
    {
        for (size_t i = 0; i < 16; i++)
        {
            if (this->_matrix[i] != rhs._matrix[i])
            {
                return false;
            }
        }
        return true;
    }
    
    inline bool operator != (const Transform_3D & rhs) const
    {
        return !(*this == rhs);
    }

    //http://devmaster.net/wiki/Transformation_matrices
    Transform_3D* inverse() const
    {
    	Transform_3D *result = new Transform_3D();
    	float tx,ty,tz;

    	result->_matrix[0]  =  this->_matrix[0];
    	result->_matrix[1]  =  this->_matrix[4];
    	result->_matrix[2]  =  this->_matrix[8];
    	tx 			       =  -this->_matrix[3];

    	result->_matrix[4]  =  this->_matrix[1];
    	result->_matrix[5]  =  this->_matrix[5];
    	result->_matrix[6]  =  this->_matrix[9];
    	ty 			       =  -this->_matrix[7];

    	result->_matrix[8]  =  this->_matrix[2];
    	result->_matrix[9]  =  this->_matrix[6];
    	result->_matrix[10] =  this->_matrix[10];
    	tz 				   =  -this->_matrix[11];

    	result->_matrix[12] =  this->_matrix[12];
    	result->_matrix[13] =  this->_matrix[13];
    	result->_matrix[14] =  this->_matrix[14];
    	result->_matrix[15] =  this->_matrix[15];

    	result->_matrix[3]   =  tx*result->_matrix[0] + ty*result->_matrix[1] + tz*result->_matrix[2];
    	result->_matrix[7]   =  tx*result->_matrix[4] + ty*result->_matrix[5] + tz*result->_matrix[6];
    	result->_matrix[11]  =  tx*result->_matrix[8] + ty*result->_matrix[9] + tz*result->_matrix[10];
    	return result;
    }

    inline void print() const
    {
        std::cout << "Matrix =\n{";
        for (int y = 0; y < 4; ++y)
        {
            std::cout << "\t{ ";
            for (int x = 0; x < 4; ++x)
            {
                std::cout << this->_matrix[x + y * 4] << " ";
            }
            std::cout << "}\n";
        }
        std::cout << "}" << std::endl;
    }
};

// ----------------------------------------------------------------------------

//! Translation of a 3D vector.
/*!
    This is an affine transformation (preserving parallelism).
    Reference: http://en.wikipedia.org/wiki/Euler_angles
    \todo review inheritance to avoid misuse.
    \ingroup Math Geometry
*/
template <typename Space_Unit>
class Translation_3D
    : public Transform_3D<Space_Unit>
{
public:
    inline Translation_3D(void)
    {
    }

    inline Translation_3D(const Vector_3D<Space_Unit> & translation_vector)
    {
        this->_matrix[3] = translation_vector.x();
        this->_matrix[7] = translation_vector.y();
        this->_matrix[11] = translation_vector.z();
    }

    //! Rotate a 3D vector.
    inline Vector_3D<Space_Unit> operator * 
        (const Vector_3D<Space_Unit> & rhs) const
    {
        return Vector_3D<Micron> 
            (rhs.x() + this->_matrix[3],
             rhs.y() + this->_matrix[7],
             rhs.z() + this->_matrix[11]);
    }

    //! Multiply two transformation matrices.
    Transform_3D<Space_Unit> 
        operator * (const Transform_3D<Space_Unit> & rhs) const
    {
        return Transform_3D<Space_Unit>::operator *(rhs);
    }
};

// ----------------------------------------------------------------------------

//! Rotation of a 3D vector.
/*!
    This is an affine transformation (preserving parallelism).
    Reference: http://en.wikipedia.org/wiki/Euler_angles
    \todo Review inheritance to avoid misuse.
    \ingroup Math Geometry
*/
template <typename Space_Unit>
class Rotation_3D
    : public Transform_3D<Space_Unit>
{
public:
    inline Rotation_3D(void)
    {
    }

    inline Rotation_3D(const Orientation & orientation)
    {
        // Axis-rotation to matrix calculation
        // from Graphics Gems (Glassner, Academic Press, 1990).
        Space_Unit radians = (Space_Unit) (orientation.rotation * RADIANT_PER_DEGREE);
        Space_Unit c = cos(radians);
        Space_Unit s = sin(radians);
        Space_Unit t = 1 - c;
        Space_Unit sx = s * orientation.axis.x();
        Space_Unit sy = s * orientation.axis.y();
        Space_Unit sz = s * orientation.axis.z();
        Space_Unit tx = t * orientation.axis.x();
        Space_Unit txy = tx * orientation.axis.y();
        Space_Unit txz = tx * orientation.axis.z();
        Space_Unit tyz = t * orientation.axis.y() * orientation.axis.z();
        this->_matrix[0] = tx * orientation.axis.x() + c;
        this->_matrix[1] = txy - sz;
        this->_matrix[2] = txz + sy;
        this->_matrix[3] = 0.0;
        this->_matrix[4] = txy + sz;
        this->_matrix[5] = t * orientation.axis.y() * orientation.axis.y() + c;
        this->_matrix[6] = tyz - sx;
        this->_matrix[7] = 0.0;
        this->_matrix[8] = txz - sy;
        this->_matrix[9] = tyz + sx;
        this->_matrix[10] = t * orientation.axis.z() * orientation.axis.z() + c;
        this->_matrix[11] = 0.0;
        this->_matrix[12] = 0.0;
        this->_matrix[13] = 0.0;
        this->_matrix[14] = 0.0;
        this->_matrix[15] = 1.0;
    }

    /* Construct a rotation using Euler angles in the zxz convention in a right
       handed reference coordinate system.
    */
    inline Rotation_3D(Radiant global_z, Radiant local_x, Radiant local_z)
    {
        //from http://en.wikipedia.org/wiki/Euler_angles
        Space_Unit c1 = cos(local_x);
        Space_Unit s1 = sin(local_x);
        Space_Unit c2 = cos(global_z);
        Space_Unit s2 = sin(global_z);
        Space_Unit c3 = cos(local_z);
        Space_Unit s3 = sin(local_z);
        this->_matrix[0] = c1 * c3 - c2 * s1 * s3;
        this->_matrix[1] = - c3 * s1 - c1 * c2 * s3;
        this->_matrix[2] = s2 * s3;
        this->_matrix[4] = c2 * c3 * s1 + c1 * s3;
        this->_matrix[5] = c1* c2 * c3 - s1 * s3;
        this->_matrix[6] = -c3 * s2;
        this->_matrix[8] = s1 * s2;
        this->_matrix[9] = c1 * s2;
        this->_matrix[10] = c2;
    }

    //! Rotate a 3D vector.
    inline Vector_3D<Space_Unit> operator * 
        (const Vector_3D<Space_Unit> & rhs) const
    {
        return Vector_3D<Space_Unit>
            ((Space_Unit)(rhs.x() * this->_matrix[0] + 
                          rhs.y() * this->_matrix[1] + 
                          rhs.z() * this->_matrix[2]),
             (Space_Unit)(rhs.x() * this->_matrix[4] + 
                          rhs.y() * this->_matrix[5] + 
                          rhs.z() * this->_matrix[6]),
             (Space_Unit)(rhs.x() * this->_matrix[8] + 
                          rhs.y() * this->_matrix[9] + 
                          rhs.z() * this->_matrix[10]));
    }

    //! Multiply two transformation matrices.
    Transform_3D<Space_Unit> 
        operator * (const Transform_3D<Space_Unit> & rhs) const
    {
        return Transform_3D<Space_Unit>::operator *(rhs);
    }

    inline void print() const 
    {
        Radiant z = acos(this->_matrix[10]);
        Radiant X = acos(this->_matrix[9] / 
            sqrt(1 - this->_matrix[10] * this->_matrix[10]));
        Radiant Z = asin(this->_matrix[2] / 
            sqrt(1 - this->_matrix[10] * this->_matrix[10]));

        std::cout << "Rotation_3D (zxz): "
            << z << ", " << X << ", " << Z << std::endl;
    }

    //! Reset matrix to identity matrix.
    /*!
        \deprecated use Matrix<T>::identity() instead!
        \todo This should be renamed to identity (inherited from Matrix 
        class). (TT)
    */
    void reset()
    {
        this->identity();
    }

    //! Rotate around y-axis.
    inline void rotate_y(Degree y)
    {
        Space_Unit radians = y * RADIANT_PER_DEGREE;
        Space_Unit cos_a  =     cos(radians);
        Space_Unit sin_a  =     sin(radians);
        this->_matrix[0] =     cos_a;
        this->_matrix[2] =     sin_a;
        this->_matrix[5] =     1.0;
        this->_matrix[8] =     -sin_a;
        this->_matrix[10] =     cos_a;
        this->_matrix[1] =     0.0;
        this->_matrix[4] =     0.0;
        this->_matrix[6] =     0.0;
        this->_matrix[11] =     0.0;
    }
};

// ----------------------------------------------------------------------------

}
#endif

