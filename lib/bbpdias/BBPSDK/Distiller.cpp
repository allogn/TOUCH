/*
 *  Target.cpp
 *  bc_extBBP-SDK
 *
 *  Created by John Kenyon on 31.10.08.
 *  Copyright 2008 EPFL. All rights reserved.
 *
 */

#include "BBP/Model/Microcircuit/Distiller/Distiller.h"
#include "BBP/Model/Microcircuit/Containers/Neurons.h"
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>


namespace bbp {

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Distiller::Distiller                             (Microcircuit &microcircuit)
    : _microcircuit(&microcircuit)
    {
    }

    Distiller::~Distiller()
    {
        _microcircuit = NULL;
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::get_target                     (std::string name)
    {
        return *_microcircuit->targets().find(name);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::split_cells                   (const Target & SRC)
    {
        Targets ret;
        Target target = prep_target(SRC);
        for(Target::cell_iterator i = target.cell_begin(); i != target.cell_end(); i++)
        {
            Target temp;
            temp.insert(*i);
            ret.push_back(temp);
        }
        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::split_cells                   (std::string SRC)
    {
        return split_cells(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::split_subtargets              (const Target & SRC)
    {
        Targets ret;
        for(size_t i = 0; i < SRC.target_member_size(); i++)
        {
            Target current_target = SRC.target_member(i);
            ret.push_back(current_target);
        }
        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::split_subtargets              (std::string SRC)
    {
        return split_subtargets(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::find_source_synapses           (const Target & SRC)
    {
        Target ret;
        std::set<Cell_GID> results;

        Neurons ns = _microcircuit->neurons();
        Target tgt = prep_target(SRC);
        for ( Target::cell_iterator i = tgt.cell_begin(); i != tgt.cell_end(); i++)
        {
            Neuron &n = *ns.find(*i);
            Neurons pre;
            pre = n.presynaptic_neurons();
            for(Neurons::iterator j = pre.begin(); j != pre.end(); j++)
            {
                results.insert(j->gid());
            }
        }

        for(std::set<Cell_GID>::iterator i = results.begin(); i != results.end(); i++)
        {
            ret.insert(*i);
        }

        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::find_source_synapses           (std::string SRC)
    {
        return find_source_synapses(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::find_destination_synapses      (const Target & SRC)
    {
        Target ret;
        std::set<Cell_GID> results;

        Neurons ns = _microcircuit->neurons();
        Target tgt = prep_target(SRC);
        for ( Target::cell_iterator i = tgt.cell_begin(); i != tgt.cell_end(); i++)
        {
            Neuron &n = *ns.find(*i);
            Neurons post;
            post = n.postsynaptic_neurons();
            for(Neurons::iterator j = post.begin(); j != post.end(); j++)
            {
                results.insert(j->gid());
            }
        }

        for(std::set<Cell_GID>::iterator i = results.begin(); i != results.end(); i++)
        {
            ret.insert(*i);
        }

        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::find_destination_synapses      (std::string SRC)
    {
        return find_destination_synapses(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::find_reciprocal_synapses       (const Target & SRC)
    {
        return intersect(find_source_synapses(SRC),find_destination_synapses(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::find_reciprocal_synapses       (std::string SRC)
    {
        return find_reciprocal_synapses(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_first                   (const Target &SRC)
    {
        Target ret(SRC.name()+"_first",SRC.type());
        Target temp = prep_target(SRC);
        ret.insert(*temp.cell_begin());
        return ret;
    }

    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_first                   (std::string SRC)
    {
        return select_first(get_target(SRC));
    }

    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::random_probability             (const Target & LHS,float percent, int seed)
    {
        if(percent > 1.0f){
            log_message("Attempted to randomly select more than the entire Target",
                DEBUG_LEVEL, __FILE__, __LINE__);
            return LHS;
        }
        Target ret = random_absolute(LHS,int(percent*LHS.flatten().cell_member_size()),seed);
        ret.name("random_probability");
        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::random_probability             (std::string LHS,float percent, int seed)
    {
        return random_probability(get_target(LHS),percent,seed);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::random_absolute                (const Target & LHS, int count, int seed)
    {
        Target ret("random_absolute",CELL_TARGET);
        Target tgt = LHS.flatten();
        if(count >= static_cast<int>(tgt.cell_member_size())){
            log_message("Attempted to randomly select more than the entire Target",
                DEBUG_LEVEL, __FILE__, __LINE__);
            return tgt;
        }
        std::vector<Cell_GID> deck(tgt.cell_begin(),tgt.cell_end());
        if(seed != -1){
            srand(seed);
        }
        std::random_shuffle(deck.begin(),deck.end());
        std::vector<Cell_GID> selection;
        selection.assign(deck.begin(),deck.begin()+count);
        
        for(std::vector<Cell_GID>::iterator i = selection.begin(); i != selection.end(); i++)
        {
            ret.insert(*i);
        }

        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::random_absolute                (std::string LHS, int count, int seed)
    {
        return random_absolute(get_target(LHS),count,seed);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::cast_to_neuron                 (const Target & SRC)
    {
        return Distiller::s_cast_to_neuron(SRC);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::cast_to_neuron                 (std::string SRC)
    {
        return cast_to_neuron(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::cast_to_section                (const Target & SRC)
    {
        return Distiller::s_cast_to_section(SRC);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::cast_to_section                (std::string SRC)
    {
        return cast_to_section(get_target(SRC));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::cast_to_compartment            (const Target & SRC)
    {
        return Distiller::s_cast_to_compartment(SRC);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::cast_to_compartment            (std::string SRC)
    {
        return cast_to_compartment(get_target(SRC));
    }






    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::resolve_to_neuron                 (const Target & SRC)
    {
        return Target();
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::resolve_to_neuron                 (std::string SRC)
    {
    
        return resolve_to_neuron(get_target(SRC));
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::resolve_to_section                (const Target & SRC)
    {
    
        return Target();
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::resolve_to_section                (std::string SRC)
    {
    
        return resolve_to_section(get_target(SRC));
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::resolve_to_compartment            (const Target & SRC)
    {
    
        return Target();
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::resolve_to_compartment            (std::string SRC)
    {
    
        return resolve_to_compartment(get_target(SRC));
    }




    //  ------------------ Logical/Set Theory Operations ---------------------- 

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::add                            (const Target & LHS, const Target & RHS)
    {
        return LHS.add(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::add                            (std::string LHS, const Target & RHS)
    {
        return get_target(LHS).add(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::add                            (const Target & LHS, std::string RHS)
    {
        return LHS.add(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::add                            (std::string LHS, std::string RHS)
    {
        return get_target(LHS).add(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::intersect                      (const Target & LHS, const Target & RHS)
    {
        return LHS.intersect(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::intersect                      (std::string LHS, const Target & RHS)
    {
        return get_target(LHS).intersect(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::intersect                      (const Target & LHS, std::string RHS)
    {
        return LHS.intersect(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::intersect                      (std::string LHS, std::string RHS)
    {
        return get_target(LHS).intersect(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::subtract                       (const Target & LHS, const Target & RHS)
    {
        return LHS.subtract(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::subtract                       (std::string LHS, const Target & RHS)
    {
        return get_target(LHS).subtract(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::subtract                       (const Target & LHS, std::string RHS)
    {
        return LHS.subtract(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::subtract                       (std::string LHS, std::string RHS)
    {
        return get_target(LHS).subtract(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::exclusive                      (const Target & LHS, const Target & RHS)
    {
        return LHS.exclusive(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::exclusive                      (std::string LHS, const Target & RHS)
    {
        return get_target(LHS).exclusive(RHS);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::exclusive                      (const Target & LHS, std::string RHS)
    {
        return LHS.exclusive(get_target(RHS));
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::exclusive                      (std::string LHS, std::string RHS)
    {
        return get_target(LHS).exclusive(get_target(RHS));
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    //  ------------------ Volumetric Operations -----------------------------
    Target Distiller::select_cylindrical_volume      (const Target & SRC, float x, float y, float z, float height, float radius)
    {
        Target all_cells;
        std::vector<Cell_GID> selection;
        Target ret;

        all_cells = prep_target(SRC);
        
        for(Target::cell_iterator i = all_cells.cell_begin(); i != all_cells.cell_end(); i++)
        {
            const Vector_3D< Micron > &p = _microcircuit->neuron(*i).position();
            if(compare_point_to_cylinder(p.x(),p.y(),p.z(),x,y,z,height,radius))
            {
                selection.push_back(*i);
            }
        }
        for(std::vector<Cell_GID>::iterator i = selection.begin(); i != selection.end(); i++)
        {
            ret.insert(*i);
        }
        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_cylindrical_volume      (std::string SRC, float x, float y, float z, float height, float radius)
    {
        return select_cylindrical_volume(get_target(SRC),x,y,z,height,radius);
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    bool Distiller::compare_point_to_cylinder        (float px, float py, float pz, float cx, float cy, float cz, float height, float radius)
    {
        if( pz < cz ){ // Point is BELOW the cylinder
            return false;
        }
        
        if( pz > cz + height){ // Point is ABOVE the cylinder
            return false;
        }
        
        // Pythagorean theorum! a^2 + b^2 = c^2 <> r^2.  This way is faster, no square root.
        if( (cx - px)*(cx - px)  +  (cy - py)*(cy - py)  > radius * radius){ // Within radius
            return false;
        }
        
        return true;
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    bool Distiller::compare_point_to_triangle        (float px, float py, float tx1, float ty1, float tx2, float ty2, float tx3, float ty3)
    {
        float triangle[3][2];
        triangle[0][0] = tx1;
        triangle[0][1] = ty1;
        triangle[1][0] = tx2;
        triangle[1][1] = ty2;
        triangle[2][0] = tx3;
        triangle[2][1] = ty3;

    /* The way this works is a bit complicated.  In this case we are only working in 2D, to tell if a point lies within
       a 2D triangle.  Instead of dealing with a triangle, we deal with 3 infinite lines, and make sure that the point
       is on the correct side of each line.  So we take two points to make a line, and we use the third point to tell
       which side of the line is the "right" side of the line.  If the point passes for all three lines, then it is
       also within the triangle.  Otherwise not.  This will work for any convex polygon, it does not work for concave
       polygons.

       Ok, the math needs a bit more explanation.  First, the dot product is the key to all of this.  If the dot product
       of two vectors is positive that means the angle between them is less than 90 degrees.  More simply, this means 
       the two vectors point in the same-ish direction.  If the dot product is negative, then they point in different
       directions.  So, if we have two points which represent a line, which bisects space, then we will can easily
       figure if a point is on one side or the other based on a normal vector.  So we do a simple vector subtraction
       between the point in question and one point on the line to get our offset vector.  We subtract the two
       points on the line to get a vector which runs along the line.  If we manually rotate that vector by 90 
       degrees, then it becomes our normal vector.  Dot products with vectors on one side will be positive, the
       other will be negative.

       Now we finish up by getting a third vector, between one of the points on the space partition line, and the
       third point on the triangle.  This "guide" vector will tell us which side of the space partition is the 
       "correct" side.  If the point is on the correct side of the space partition, then the dot product between 
       the offset and the normal should have the same sign.  We can save ourselves work by simply finding both
       dot products, and multiplying them together.  If the result is positive, then both original values had the
       same sign, and so were on the same side of the space partition.  If it is negative, then the original values
       were different, and so we know that the point is on the wrong side of the space partition.

       This whole process must be repeated for all three pairs of points acting as a space partition.
       */


        // for each set of points A,B,C and P, we want ((R90 * (A - B)) dot P) * ((R90 * (A - B)) dot C) < 0

        for(int start = 0; start < 3; start++)
        { 
            int next   = (start + 1) % 3;
            int sample = (start + 2) % 3;
            float start_to_next[2];
            start_to_next[0] = triangle[next][0]     - triangle[start][0];
            start_to_next[1] = triangle[next][1]     - triangle[start][1];

            float orthogonal[2];
            orthogonal[0] = -start_to_next[1];
            orthogonal[1] = start_to_next[0];

            float start_to_point[2];
            start_to_point[0] = px                   - triangle[start][0];
            start_to_point[1] = py                   - triangle[start][1];

            float start_to_sample[2];
            start_to_sample[0] = triangle[sample][0] - triangle[start][0];
            start_to_sample[1] = triangle[sample][1] - triangle[start][1];
            
            float ortho_dot_pointv  = orthogonal[0] * start_to_point[0]  + orthogonal[1] * start_to_point[1];
            float ortho_dot_samplev = orthogonal[0] * start_to_sample[0] + orthogonal[1] * start_to_sample[1];
            
            if (ortho_dot_pointv * ortho_dot_samplev < 0)
                return false;
        }
        return true;
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    bool Distiller::compare_point_to_hexagonal      (float px, float py, float pz, float cx, float cy, float cz, float height, float radius)
    {
        if( pz < cz ){ // Point is BELOW the hexagon
            return false;
        }
        
        if( pz > cz + height){ // Point is ABOVE the hexagon
            return false;
        }
        

        if(compare_point_to_hexagon(px,py,cx,cy,height,radius))
            return true;
        else
            return true;
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    bool Distiller::compare_point_to_hexagon (float px, float py, float cx, float cy, float height, float radius)
    {
        typedef std::pair<float,float> point_t;
        typedef std::vector<point_t> point_v;
        point_v hex_points;

        // Create all the points for the hexagon (see the parametric equation for a circle, with 6 points)
        for(int i = 0; i < 6; i++)
        {
            point_t p;
            p.first  = cx + radius * cos(i * M_PI / 3.0f);
            p.second = cy + radius * sin(i * M_PI / 3.0f);
            hex_points.push_back(p);
        }

        for(int i = 1; i < 5; i++)
        {
    //bool Distiller::compare_point_to_triangle        (float px, float py, float tx1, float ty1, float tx2, float ty2, float tx3, float ty3)
            if // Big fancy if statement!
            (
                !compare_point_to_triangle(
                    px,py, 
                    hex_points[0].first,  hex_points[0].second,
                    hex_points[i].first,  hex_points[i].second,
                    hex_points[i+1].first,hex_points[i+2].second
                )
            )
                return false;
        }
        
        return true;
    }
    

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_hexagonal_volume        (const Target & SRC, float x, float y, float z, float height, float radius)
    {
        
        return Target();
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_hexagonal_volume        (std::string SRC, float x, float y, float z, float height, float radius)
    {
        return Target();
    }

    //  ------------------ Morpholgical Operations ---------------------- 




    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Label> Distiller::list_morphologies(const Target &SRC)
    {
        Target temp = prep_target(SRC);
        std::set<Label> ret;
        
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            ret.insert(_microcircuit->neuron(*i).morphology_type().name());
        }
        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Label> Distiller::list_morphologies(std::string SRC)
    {
        return list_morphologies(get_target(SRC));
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Morphology_Type_ID> Distiller::list_morphology_ids(const Target &SRC)
    {
        Target temp = prep_target(SRC);
        std::set<Morphology_Type_ID> ret;
        
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            ret.insert(_microcircuit->neuron(*i).morphology_type().id());
        }
        return ret;  
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Morphology_Type_ID> Distiller::list_morphology_ids(std::string SRC)
    {
        return list_morphology_ids(get_target(SRC));
    }
    

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphology                     (const Target &SRC,Label key)
    {
        Target ret(SRC.name()+"_filtered_morphology_"+key,SRC.type());
        Target temp = prep_target(SRC);
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            if(_microcircuit->neuron(*i).morphology_type().name() == key){
                ret.insert(*i);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphology                     (std::string   SRC,Label key)
    {
        return filter_morphology(get_target(SRC),key);
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphology                     (const Target &SRC,Morphology_Type_ID key)
    {
        Target ret(SRC.name()+"_filtered",SRC.type());
        Target temp = prep_target(SRC);
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            if(_microcircuit->neuron(*i).morphology_type().id() == key){
                ret.insert(*i);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphology                     (std::string   SRC,Morphology_Type_ID key)
    {
        return filter_morphology(get_target(SRC), key);
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphology                     (const Target &SRC,Morphology_Type key)
    {
        Target ret(SRC.name()+"_filtered",SRC.type());
        Target temp = prep_target(SRC);
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            if(_microcircuit->neuron(*i).morphology_type().id() == key.id()){
                ret.insert(*i);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphology                     (std::string   SRC,Morphology_Type key)
    {
        return filter_morphology(get_target(SRC),key);
    }
    



    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    // Unique Morph
    Target Distiller::unique_morphologies(const Target & RHS)
    {
        std::set<Morphology_Type_ID> visited_types;
        Target ret(RHS.name()+"_unique_morph",RHS.type());
        Target temp = RHS.flatten();

        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            Morphology_Type_ID cur_id = _microcircuit->neuron(*i).morphology_type().id();
            if(visited_types.find(cur_id) == visited_types.end())
            {
                ret.insert(*i);
                visited_types.insert(cur_id);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::unique_morphologies(std::string RHS)
    {
        return unique_morphologies(get_target(RHS));
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::unique_morphologies(const Targets &RHS)
    {
        Targets ret;
        for(Targets::iterator i = RHS.begin(); i != RHS.end(); i++)
        {
            ret.push_back(unique_morphologies(*i));
        }

        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::unique_morphologies(std::vector<Target> &RHS)
    {
        Targets ret;
        for(std::vector<Target>::iterator i = RHS.begin(); i != RHS.end(); i++)
        {
            ret.push_back(unique_morphologies(*i));
        }
        return ret;        
    }
    
    //  ------------------ MorphoElectric Operations ---------------------- 
    

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Label> Distiller::list_morphoelectric_types(const Target &SRC)
    {
        Target temp = prep_target(SRC);
        std::set<Label> ret;
        
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            ret.insert(_microcircuit->neuron(*i).electrophysiology_type().name());
        }
        return ret;
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Label> Distiller::list_morphoelectric_types(std::string SRC)
    {
        return list_morphoelectric_types(get_target(SRC));
    }

    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Electrophysiology_Type_ID> Distiller::list_morphoelectric_type_ids(const Target &SRC)
    {
        Target temp = prep_target(SRC);
        std::set<Electrophysiology_Type_ID> ret;
        
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            ret.insert(_microcircuit->neuron(*i).electrophysiology_type().id());
        }
        return ret;        
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    std::set<Electrophysiology_Type_ID> Distiller::list_morphoelectric_type_ids(std::string SRC)
    {
        return list_morphoelectric_type_ids(get_target(SRC));
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphoelectric(const Target &SRC,Label key)
    {
        Target ret(SRC.name()+"_filtered_morphoelectric_"+key,SRC.type());
        Target temp = prep_target(SRC);
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            if(_microcircuit->neuron(*i).electrophysiology_type().name() == key){
                ret.insert(*i);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphoelectric(std::string   SRC,Label key)
    {
        return filter_morphoelectric(get_target(SRC),key);
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphoelectric(const Target &SRC,Electrophysiology_Type_ID key)
    {
        Target ret(SRC.name()+"_filtered",SRC.type());
        Target temp = prep_target(SRC);
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            if(_microcircuit->neuron(*i).electrophysiology_type().id() == key){
                ret.insert(*i);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphoelectric(std::string   SRC,Electrophysiology_Type_ID  key)
    {
        return filter_morphoelectric(get_target(SRC),key);
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphoelectric(const Target &SRC,Electrophysiology_Type key)
    {
        Target ret(SRC.name()+"_filtered_morphoelectric" ,SRC.type());
        Target temp = prep_target(SRC);
        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            if(_microcircuit->neuron(*i).electrophysiology_type().id() == key.id()){
                ret.insert(*i);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::filter_morphoelectric(std::string   SRC,Electrophysiology_Type key)
    {
        return filter_morphoelectric(get_target(SRC),key);
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    // Unique MorphoElectric
    Target Distiller::unique_morphoelectric_types(const Target & RHS)
    {
        std::set<Electrophysiology_Type_ID> visited_types;
        Target ret(RHS.name()+"_unique_morphoelectric",RHS.type());
        Target temp = RHS.flatten();

        for(Target::cell_iterator i = temp.cell_begin(); i != temp.cell_end(); i++)
        {
            Electrophysiology_Type_ID cur_id = _microcircuit->neuron(*i).electrophysiology_type().id();
            if(visited_types.find(cur_id) == visited_types.end())
            {
                ret.insert(*i);
                visited_types.insert(cur_id);
            }
        }
        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::unique_morphoelectric_types(std::string RHS)
    {
        return unique_morphoelectric_types(get_target(RHS));
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::unique_morphoelectric_types(const Targets &RHS)
    {
        Targets ret;
        for(Targets::iterator i = RHS.begin(); i != RHS.end(); i++)
        {
            ret.push_back(unique_morphoelectric_types(*i));
        }

        return ret;
    }
    
    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Targets Distiller::unique_morphoelectric_types(std::vector<Target> &RHS)
    {
        Targets ret;
        for(std::vector<Target>::iterator i = RHS.begin(); i != RHS.end(); i++)
        {
            ret.push_back(unique_morphoelectric_types(*i));
        }
        return ret;        
    }



    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_volume_comparator(const Target &RHS,Volume_Comparator &cmp)
    {
        Target ret(RHS.name()+"_volume_comp",RHS.type());
        Target SRC = prep_target(RHS);

        for(Target::cell_iterator i = SRC.cell_begin(); i != SRC.cell_end(); i++)
        {
            Vector_3D< Micron > p;
            p = _microcircuit->neuron(*i).position();
            if(cmp(p.x(),p.y(),p.z()))
            {
                ret.insert(*i);
            }
        }

        return ret;
    }


    //------------------------------------------------------------------------- 
    //
    //------------------------------------------------------------------------- 
    Target Distiller::select_volume_comparator(std::string SRC,Volume_Comparator &cmp)
    {
        return select_volume_comparator(get_target(SRC),cmp);
    }
 

    Target Distiller::s_add                          (const Target & LHS, const Target & RHS)
    {
        return LHS.add(RHS);
    }

    Target Distiller::s_intersect                    (const Target & LHS, const Target & RHS)
    {
        return LHS.intersect(RHS);
    }

    Target Distiller::s_subtract                     (const Target & LHS, const Target & RHS)
    {
        return LHS.subtract(RHS);
    }

    Target Distiller::s_exclusive                    (const Target & LHS, const Target & RHS)
    {
        return LHS.exclusive(RHS);
    }

    Target Distiller::s_cast_to_neuron               (const Target & SRC)
    {
        Target ret(SRC.name()+"_cast_to_neuron",CELL_TARGET);
        for(Target::cell_iterator i = SRC.cell_begin(); i != SRC.cell_end(); i++)
        {
            ret.insert(*i);
        }

        for(size_t j = 0; j < SRC.target_member_size(); j++)
        {
            ret.insert(s_cast_to_neuron(SRC.target_member(j)));
        }

        return ret;
    }

    Target Distiller::s_cast_to_section              (const Target & SRC)
    {
        Target ret(SRC.name()+"cast_to_neuron",SECTION_TARGET);
        for(Target::cell_iterator i = SRC.cell_begin(); i != SRC.cell_end(); i++)
        {
            ret.insert(*i);
        }

        for(size_t j = 0; j < SRC.target_member_size(); j++)
        {
            ret.insert(s_cast_to_section(SRC.target_member(j)));
        }

        return ret;
    }

    Target Distiller::s_cast_to_compartment          (const Target & SRC)
    {
        Target ret(SRC.name()+"cast_to_neuron",COMPARTMENT_TARGET);
        for(Target::cell_iterator i = SRC.cell_begin(); i != SRC.cell_end(); i++)
        {
            ret.insert(*i);
        }

        for(size_t j = 0; j < SRC.target_member_size(); j++)
        {
            ret.insert(s_cast_to_compartment(SRC.target_member(j)));
        }

        return ret;
    }



}// end namespace bbp

