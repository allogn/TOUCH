/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007. All rights reserved.

        Responsible authors:	Nikolai Chapochnikov
								Thomas Traenkler

*/

#include <stdexcept>
#include "BBP/Filters/Microcircuit/Segment_Voxel_Mapper.h"

namespace bbp {

// ----------------------------------------------------------------------------
void  Segment_Voxel_Mapper::initialize(Experiment& exp)
  {
    input_ptr()->reset(exp.microcircuit_ptr());
  }

// ----------------------------------------------------------------------------

Segment_Voxel_Mapper::Segment_Voxel_Mapper(
		Micron          voxel_size, 
        Box <Micron> &  bounding_box, 
        bool            clean_cut)
:	current_global_segment_index(0), 
	clean_cut					(clean_cut),
	half_pixel					(voxel_size / 2.0),
	voxel_volume				(voxel_size * voxel_size* voxel_size),
    safe_mode(clean_cut)
{
    if (voxel_size > 0.0f)
    {
        output().voxel_size(voxel_size);
        this->voxel_size = voxel_size;
        voxel_size_reciprocal = 1 / voxel_size;
    }
    else
    {
        throw_exception(std::logic_error("Voxel is zero, logical error."),
                        SEVERE_LEVEL, __FILE__, __LINE__);
    }
	minimum = bounding_box.minimum();
	maximum = bounding_box.maximum();
    output().resize(Count (ceil((maximum.x()-minimum.x()) * 
                                voxel_size_reciprocal)),
					Count (ceil((maximum.y()-minimum.y()) *
                                voxel_size_reciprocal)), 
					Count (ceil((maximum.z()-minimum.z()) *
                                voxel_size_reciprocal)));
    
	output().center(minimum + (maximum - minimum) / 2);
}

// ----------------------------------------------------------------------------

void Segment_Voxel_Mapper::insert_neuron (const Neuron & neuron)
{
/*
#ifndef NDEBUG
    std::cout << "Segment_Voxel_Mapper::insert_neuron() with GID " 
        << neuron.gid() << std::endl;
#endif
*/
    // Insert soma into the mapping volume. Registration of the soma should be 
    // done before registration of segments.
    const Soma soma = neuron.soma();
    insert_soma(soma);
    /*!
        \bug Potential bug: This only works if soma is only one segment? (TT)
    */
    output().segment_index_gid_lookup.push_back(Segment_GID(neuron.gid(),0,0));
    ++current_global_segment_index;

    
    // Insert sections into the mapping volume. Type can be specified at 
    // this stage: put axon or dendrite instead of segments.
    const Sections sections = neuron.neurites();

	Sections::const_iterator sections_end =  sections.end();
    for (Sections::const_iterator i = sections.begin(); 
        i != sections_end ; ++i)
    {
/*
#ifndef NDEBUG
        std::cout << "Section " << i->id() << std::endl;
#endif
*/
		Section_ID section(i->id());
        Segments segments = i->segments();
		Segments::const_iterator segments_end = segments.end();
		Segment_ID segment(0);

		// Insert all segments of the current section.
		for (Segments::const_iterator j = segments.begin(); 
            j != segments_end ; ++j)
		{
/*
#ifndef NDEBUG
            std::cout << "Segment " << j->id() << std::endl;
#endif
*/
			insert_segment(*j);
			output().segment_index_gid_lookup.push_back(
				Segment_GID(neuron.gid(), section, segment));
			++current_global_segment_index;
			++segment;
		}	
    }
}

// ----------------------------------------------------------------------------

void Segment_Voxel_Mapper::insert_soma (const Soma & soma)
{
/*
#ifndef NDEBUG
    std::cout << "Segment_Voxel_Mapper::insert_soma()" << std::endl;
#endif
*/
    
    Micron soma_radius_max = 0;
    Micron sum(0);

    // Calculate the sum and maximum distance of points from the soma center.
    Soma::const_iterator my_soma_end = soma.end(); 
    for (Soma::const_iterator i = soma.begin(); i != my_soma_end;i++)
    {
		Micron new_radius((*i - soma.position()).length());
		sum += new_radius;
		if(soma_radius_max < new_radius)
        {
			soma_radius_max = new_radius;
        }
    }
    Micron average_radius = sum / (Micron) soma.size();
    Micron average_radius_squared = pow(average_radius,2);
	Vector_3D<Micron> average_soma_radius_vector(
        average_radius, average_radius ,average_radius);

    // Commented these since they were not used. (TT)
    //Vector_3D<Micron> minimum_position = soma.position() - 
    //  average_soma_radius_vector;
    //Vector_3D<Micron> maximum_position = soma.position() + 
    //  average_soma_radius_vector;

    // convert the position into pixel coordinates from volume minimum.
	Vector_3D<Micron> relative_soma_position = 
        (soma.position() - minimum) * voxel_size_reciprocal;
	
    // the number of pixels of the farest soma point from the soma center.
    Pixel_Coordinate pixel_distance = 
        Pixel_Coordinate( ceil(soma_radius_max * voxel_size_reciprocal));
	Pixel_Coordinate start_x	= 
        Pixel_Coordinate(floor(relative_soma_position.x())) - pixel_distance;
	Pixel_Coordinate end_x	    = 
        Pixel_Coordinate(ceil(relative_soma_position.x()))  + pixel_distance;
	Pixel_Coordinate start_y	= 
        Pixel_Coordinate(floor(relative_soma_position.y())) - pixel_distance;
	Pixel_Coordinate end_y      = 
        Pixel_Coordinate(ceil(relative_soma_position.y()))  + pixel_distance;
	Pixel_Coordinate start_z    = 
        Pixel_Coordinate(floor(relative_soma_position.z())) - pixel_distance;
	Pixel_Coordinate end_z      = 
        Pixel_Coordinate(ceil(relative_soma_position.z()))  + pixel_distance;
	
    // Visit all voxels within a cube with 2* maximum radius as side 
    // length and derive the original coordinates
	for (Pixel_Coordinate x = start_x ; x <= end_x ; x++)
	for (Pixel_Coordinate y = start_y ; y <= end_y ; y++)
	for (Pixel_Coordinate z = start_z ; z <= end_z ; z++)
	{
	    Vector_3D<Micron> current(x,y,z);
	    current = current * voxel_size + minimum;
	    
        /*
	    Vector_3D<Micron> PRelative(current-soma.center);
	    Vector_3D<Micron> PRelativeDirection(PRelative/PRelative.length());
	    float minAngle(3.14f);
	    Vector_3D<Micron> mySomaPoint;
	    for (Soma::iterator i = soma.begin(); i != my_soma_end;i++)
	    {
		    Vector_3D<Micron> SomaPointRelative(*i-soma.center);
		    float angle(acos(PRelativeDirection*
                SomaPointRelative/SomaPointRelative.length()));
		    if(angle < minAngle)
		    {
		        minAngle = angle;
		        mySomaPoint = *i;
		    }
	    }
	    if(PRelativeDirection*PRelative < (mySomaPoint-soma.center)*
            PRelativeDirection)
	    */
	    
        // If within radius and within soma bounding box.
	    if(((current - soma.position()) * ( current - soma.position())) 
            < average_radius_squared
            && minimum.x() < current.x() && current.x() < maximum.x()
            && minimum.y() < current.y() && current.y() < maximum.y()
            && minimum.z() < current.z() && current.z() < maximum.z())
	    {
			output().operator()(x,y,z).push_back(
                Segment_Voxel_Occupancy(
					current_global_segment_index, voxel_volume));
	    }
	}
}

// ----------------------------------------------------------------------------

void Segment_Voxel_Mapper::insert_segment(const Segment & segment)
{
/*
#ifndef NDEBUG
    std::cout << "Segment_Voxel_Mapper::insert_segment() with id " 
        << segment.id() << std::endl;
#endif
*/
    
    const Vector_3D<Micron> & position_begin = segment.begin().center();
    const Vector_3D<Micron> & position_end = segment.end().center();
    const Micron & radius_begin = segment.begin().radius();
    const Micron & radius_end = segment.end().radius();
    
    Micron max_radius =radius_begin;

    if (radius_end > max_radius) 
		max_radius = radius_end;
    
	Vector_3D<Micron> 
        maximum_soma_radius_vector(max_radius, max_radius, max_radius);
   
    // check if segment fully inside target volume window 
    if (minimum.x() < (position_begin.x() - maximum_soma_radius_vector.x()) && 
		(position_begin.x() + maximum_soma_radius_vector.x()) < maximum.x() && 
		minimum.x() < (position_end.x() - maximum_soma_radius_vector.x()) && 
		(position_end.x() + maximum_soma_radius_vector.x()) < maximum.x() &&
        minimum.y() < (position_begin.y()-maximum_soma_radius_vector.y()) && 
		(position_begin.y() + maximum_soma_radius_vector.y()) < maximum.y() && 
		minimum.y() < (position_end.y() - maximum_soma_radius_vector.y()) && 
		(position_end.y() + maximum_soma_radius_vector.y()) < maximum.y() &&        
        minimum.z() < (position_begin.z() - maximum_soma_radius_vector.z()) && 
		(position_begin.z() + maximum_soma_radius_vector.z()) < maximum.z() && 
		minimum.z() < (position_end.z() - maximum_soma_radius_vector.z()) && 
		(position_end.z() + maximum_soma_radius_vector.z()) < maximum.z())
    {
/*
#ifndef NDEBUG
        std::cout << "Segment fully inside volume" << std::endl;
#endif
*/
		if (max_radius <= half_pixel)
		{
			insert_small_segment(segment);
		}
		else
		{
			insert_large_segment(segment);
		} 
    }
    // if segment not entirely, but partly inside the target volume
    else if (clean_cut == true && 
      ((minimum.x() < position_begin.x() && position_begin.x() < maximum.x() &&
      minimum.y() < position_begin.y() && position_begin.y() < maximum.y() && 
      minimum.z() < position_begin.z() && position_begin.z() < maximum.z())
      || 
      (minimum.x() < position_end.x() && position_end.x() < maximum.x() &&
      minimum.y() < position_end.y() && position_end.y() < maximum.y() &&
      minimum.z() < position_end.z() && position_end.z() < maximum.z()))
      )
    {
/*
#ifndef NDEBUG
        std::cout << "Segment half inside volume" << std::endl;
#endif
*/
		if (max_radius <= half_pixel)
		{
			insert_small_segment(segment);
		}
		else
		{
			insert_large_segment(segment);
		} 
    }
    // if segment completely outside target volume
    else
    {
/*
#ifndef NDEBUG
        std::cout << "Segment outside volume" << std::endl;
#endif
*/
    }
}

// ----------------------------------------------------------------------------

void Segment_Voxel_Mapper::insert_large_segment(const Segment & segment)
{
/*
#ifndef NDEBUG
    std::cout << "Segment_Voxel_Mapper::insert_large_segment() with id " 
    << segment.id() << std::endl;
    segment.print();
#endif
*/
    const Vector_3D<Micron> & position_begin = segment.begin().center();
    const Vector_3D<Micron> & position_end = segment.end().center();
    const Micron & radius_begin = segment.begin().radius();
    const Micron & radius_end = segment.end().radius();
    
    Micron max_radius = radius_begin;
    if (radius_end > max_radius) 
        max_radius = radius_end;
    Segment_Voxel_Mapping & out = output();
    
    Vector_3D<Micron> begin_relative_to_box (position_begin - minimum), 
                      end_relative_to_box   (position_end - minimum);

    Vector_3D<Micron> segment_vector = end_relative_to_box 
                                     - begin_relative_to_box;
    Micron length = segment_vector.length();

    if (length > 0)
    {
        // first we find a 3d rectangle that will contain the cylinder
        int x_direction_sign(1), y_direction_sign(1), z_direction_sign(1);
        
        if (segment_vector.x() < 0) 
            x_direction_sign = -1;
        if (segment_vector.y() < 0) 
            y_direction_sign = -1;
        if (segment_vector.z() < 0) 
            z_direction_sign = -1;
        
        Vector_3D<Micron> gap(x_direction_sign * max_radius, 
                              y_direction_sign * max_radius, 
                              z_direction_sign * max_radius);
        Vector_3D<Micron> start(begin_relative_to_box - gap);
        Vector_3D<Micron> end(end_relative_to_box + gap);
        
        bbp_assert(length > 0);
        
        // direction vector of the segment
        Vector_3D<Micron> direction (segment_vector / length);
        
        Micron begin_parallel = begin_relative_to_box * direction;
        Micron end_parallel   = end_relative_to_box   * direction;
        
        
        Pixel_Coordinate start_x((Pixel_Coordinate(floor(
            x_direction_sign * start.x() * voxel_size_reciprocal))) 
                                 * x_direction_sign);
        Pixel_Coordinate end_x((Pixel_Coordinate(ceil(
            x_direction_sign * end.x()*voxel_size_reciprocal))) 
                               * x_direction_sign);
        Pixel_Coordinate start_y((Pixel_Coordinate(floor(
            y_direction_sign * start.y()*voxel_size_reciprocal))) 
                                 * y_direction_sign);
        Pixel_Coordinate end_y((Pixel_Coordinate(ceil(
            y_direction_sign * end.y()*voxel_size_reciprocal))) 
                               * y_direction_sign);
        Pixel_Coordinate start_z((Pixel_Coordinate(floor(
            z_direction_sign * start.z()*voxel_size_reciprocal))) 
                                 * z_direction_sign);
        Pixel_Coordinate end_z((Pixel_Coordinate(ceil(
            z_direction_sign * end.z()*voxel_size_reciprocal))) 
                               * z_direction_sign);
        
        // iterate over all voxels
        for (Pixel_Coordinate x = start_x; x!= end_x; x = x + x_direction_sign)
        for (Pixel_Coordinate y = start_y; y!= end_y; y = y + y_direction_sign)
        for (Pixel_Coordinate z = start_z; z!= end_z; z = z + z_direction_sign)
        {
            // check if segment voxel is within the output volume
            if (!safe_mode || 
                ( 0 <= x && 0 <= y &&  0 <= z && 
                  x < (Pixel_Coordinate) out.resolution().x() && 
                  y < (Pixel_Coordinate) out.resolution().y() &&
                  z < (Pixel_Coordinate) out.resolution().z()))
            {
                Vector_3D<Micron> current(x,y,z);
                current = current * voxel_size;
                float current_parallel = current * direction;
                
                if ((begin_parallel < current_parallel) && 
                    (current_parallel < end_parallel))
                {
                    float current_radius_squared = pow(
                        ( (current_parallel - begin_parallel) * radius_begin 
                         + (end_parallel - current_parallel) * radius_end) 
                        / length, 2);
                    float distance_squared = (current - begin_relative_to_box) 
                        * (current - begin_relative_to_box)
                        - pow( (current_parallel - begin_parallel), 2);
                    
                    // if current voxel is within radius of the segment
                    if (distance_squared < current_radius_squared)
                    {
                        //Micron3 temp;
                        if (out(x,y,z).empty())
                        {
                            out(x,y,z).push_back(
                                    Segment_Voxel_Occupancy
                                        (current_global_segment_index,
                                        voxel_volume));
                        }
                        else /*if ((temp = out.volume_in_voxel(x,y,z))
                                  < voxel_volume)*/
                        {
                            // dont add more than 1 to a voxel, so 
                            // if there are small segments in there
                            // assume the bigger is less volume than
                            // it actually is
                            // note that this does not work correctly
                            // if a small segment is added after
                            out(x,y,z).push_back(
                                Segment_Voxel_Occupancy
                                    (current_global_segment_index, voxel_volume
                                     /*- out.volume_in_voxel(x,y,z)*/));
                        }
                    }
                }
            }
        }
    } // if length > 0
}

// ----------------------------------------------------------------------------

}

//void Segment_Voxel_Mapper::compute_co_occupancy()
//{
//       
//	std::cout << "Computing Cooccupancy of space." << endl;
//    float TotalFilled1(0),TotalFilled2(0); 
//    for(uint32_t x = 0; x < mapping.resolution().x ; x++)
//	for(uint32_t y = 0; y < mapping.resolution().y; y++)
//    for(uint32_t z = 0 ;z < mapping.resolution().z; z++)	
//    {
//		if (mapping(x,y,z).empty() == false)
//		{
//		    ++TotalFilled1;
//		}
//		TotalFilled2 += mapping(x,y,z);	
//    }
//
//    cout << "Number of filled voxels : " << TotalFilled1 << endl;
//    cout << "Number of filled voxels if the neurons wouldn't occupy "
//            "the same space: " << TotalFilled2 << endl;
//    cout << "Ratio : " << TotalFilled1/TotalFilled2 << endl;
//}
