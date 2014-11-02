/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007-2008. All rights reserved.

        Responsible authors:   John Kenyon

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef _VOLUMECOMPARATORS_H_
#define _VOLUMECOMPARATORS_H_


#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>

namespace bbp 
{       


    /*! The Volume_Comparator is an interface (pure virtual base class)
      whose specification requires that the function-call operator 
      be overloaded with the logic to accept a 3D coordinate (x,y,z)
      and returns true or false.  This can then be used to with the
      select_volume_comparator method to grab a user specified
      volume. */
    /*! There are a number of implementations and examples of the
      Volume_Comparator in the Volume_Comparators.h and
      Volume_Comparators.cpp files.*/
    class Volume_Comparator
    {
        public:
            /*! Virtual destructor is expected for all pure virtual classes */
            virtual ~Volume_Comparator(){}

            /*! The call operator is overloaded to implement the logic 
                associated with a given volume.  This method is the main 
                purpose for the Volume_Comparator base class. */
            virtual bool operator()(float x,float y, float z) = 0;
    };


    namespace Volume_Comparators
    {   

//------------------------------------------------------------------------------
        
        class Above : public Volume_Comparator
        {
        public:
            Above(float y);
            virtual ~Above(){}
            virtual bool operator()(float x,float y, float z);
        private:
            float _cy;
        };
        
//------------------------------------------------------------------------------

        class Below : public Volume_Comparator
        {
        public:
            Below(float y);
            virtual ~Below(){}
            virtual bool operator()(float x, float y, float z);
        private:
            float _cy;
        };

//------------------------------------------------------------------------------
        
        class VerticalRange : public Volume_Comparator
        {
        public:
            VerticalRange(float low, float high);
            virtual ~VerticalRange(){}
            virtual bool operator()(float x, float y, float z);
        private:
            Above _lower;
            Below _upper;
        };
        
//------------------------------------------------------------------------------
        
        class Cube : public Volume_Comparator
        {
        public:
            Cube(float cx,float cy, float cz, float width);
            virtual ~Cube(){}
            virtual bool operator()(float x,float y, float z);
        private:
            float _cx,_cy,_cz,_width;
        };

//------------------------------------------------------------------------------

        class Circle : public Volume_Comparator
        {
        public:
            Circle(float cx, float cz, float radius);
            virtual ~Circle(){}
            virtual bool operator()(float x, float y, float z);
        private:
            float _cx,_cz,_radius;
        };
        
//------------------------------------------------------------------------------
        
        class Cylinder : public Volume_Comparator
        {
        public:
            Cylinder(float cx,float cy,float cz, float height, float radius);
            virtual ~Cylinder(){}
            virtual bool operator()(float x,float y, float z);
        private:
            Circle _circle;
            VerticalRange _range;
        };
        
//------------------------------------------------------------------------------

        class Hexagon : public Volume_Comparator
        {
            public:
            Hexagon(float cx, float cz, float radius, float rotation = 0.0f);
            virtual ~Hexagon(){}
            virtual bool operator()(float x, float y, float z);
        private:
            float _cx,_cz,_radius,_rotation;
        };

//------------------------------------------------------------------------------

        class Hexagonal : public Volume_Comparator        
        {
            public:
            Hexagonal(float cx, float cy, float cz, float height, float radius, float rotation = 0.0f);
            virtual ~Hexagonal(){}
            virtual bool operator()(float x,float y, float z);
        private:
            VerticalRange _range;
            Hexagon _hexagon;
            //float _cx,_cy,_radius,_rotation;
        };
        
//------------------------------------------------------------------------------

        class Sphere : public Volume_Comparator
        {
            public:
            Sphere(float cx, float cy, float cz, float radius);
            virtual ~Sphere(){}
            virtual bool operator()(float x, float y, float z);
        private:
            float _cx,_cy,_cz,_radius;
        };

//------------------------------------------------------------------------------

    }
}

#endif /* _VOLUMECOMPARATORS_H_ */

