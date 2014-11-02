// Dump the first morphology in segments and in mesh and compare resulting MBR
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Microcircuit/Containers/Neurons.h>
#include <BBP/Model/Microcircuit/Containers/Sections.h>
#include <BBP/Model/Microcircuit/Containers/Segments.h>

#include "circuit_loader.hpp"
#include "tools.hpp"

int main(int argc, char * argv[])
{
    circuit_loader loader(argv[1], true);
    bbp::Neurons neurons = loader.get_neurons();

    const bbp::Neuron & neuron = *(neurons.begin());
    const bbp::Morphology & morph = neuron.morphology();
    std::cout << boost::format("Loading neuron id = %1% with morphology %2%\n")
        % neuron.gid() % morph.label();

    global_transformer zero_transform;
    zero_transform.identity();

    std::cout <<
        "Morphology to morphology (identity) transformation matrix:\n";
    zero_transform.print();

    std::cout << "Morphology to neuron affine transformation matrix:\n";
    neuron.global_transform().print();

    std::cout << boost::format("Number of sections: %1%\n")
        % morph.all_sections().size();
    int number_of_segments = 0;

    vect mbr_by_segs_low;
    vect mbr_by_segs_high;
    reset_mbr (&mbr_by_segs_low, &mbr_by_segs_high);

    vect morph_mbr_by_segs_low;
    vect morph_mbr_by_segs_high;
    reset_mbr (&morph_mbr_by_segs_low, &morph_mbr_by_segs_high);

    BOOST_FOREACH (const bbp::Section & section, morph.all_sections())
    {
        number_of_segments += section.segments().size();
        bbp::Segments::const_iterator segment = section.segments().begin();
        bbp::Segments::const_iterator end = section.segments().end();
        while (segment != end)
        {
            vect plow, phigh;
            vect morph_plow, morph_phigh;
            get_segment_mbr (*segment, neuron.global_transform(),
                             &plow, &phigh);
            include_mbr (&mbr_by_segs_low, &mbr_by_segs_high, plow, phigh);

            get_segment_mbr (*segment, zero_transform,
                             &morph_plow, &morph_phigh);
            include_mbr (&morph_mbr_by_segs_low, &morph_mbr_by_segs_high,
                         morph_plow, morph_phigh);

            segment++;
        }
    }

    assert (morph.mesh_loaded());
    const bbp::Mesh & mesh = morph.mesh();
    bbp::Array<bbp::Vertex_Index>::const_iterator t = mesh.triangles().begin();
    bbp::Array<bbp::Vertex_Index>::const_iterator e = mesh.triangles().end();
    const bbp::Array<bbp::Vector_3D<bbp::Micron> > & vertices
        = mesh.vertices();

    vect mbr_by_mesh_low;
    vect mbr_by_mesh_high;
    reset_mbr (&mbr_by_mesh_low, &mbr_by_mesh_high);

    vect morph_mbr_by_mesh_low;
    vect morph_mbr_by_mesh_high;
    reset_mbr (&morph_mbr_by_mesh_low, &morph_mbr_by_mesh_high);

    while (t != e)
    {
        vect plow, phigh;
        vect morph_plow, morph_phigh;
        triangle_ids tids = { {*t++, *t++, *t++} };

        get_triangle_mbr (tids, vertices, neuron.global_transform(),
                          &plow, &phigh);
        include_mbr (&mbr_by_mesh_low, &mbr_by_mesh_high, plow, phigh);

        get_triangle_mbr (tids, vertices, zero_transform,
                          &morph_plow, &morph_phigh);
        include_mbr (&morph_mbr_by_mesh_low, &morph_mbr_by_mesh_high,
                     morph_plow, morph_phigh);
    }


    std::cout
        << boost::format("Number of segments: %1%\n") % number_of_segments;
    std::cout << "Neuron MBR by segments:\n";
    output_mbr (std::cout, mbr_by_segs_low, mbr_by_segs_high);
    std::cout << "Morphology MBR by segments:\n";
    output_mbr (std::cout, morph_mbr_by_segs_low, morph_mbr_by_segs_high);

    std::cout << boost::format("Number of mesh triangle vertices: %1%\n")
        % mesh.triangles().size();
    std::cout << boost::format("Number of mesh triangles (mesh): %1%\n")
        % mesh.triangle_count();
    std::cout << boost::format("Number of mesh vertices: %1%\n")
        % vertices.size();
    std::cout << boost::format("Number of mesh vertices (mesh): %1%\n")
        % mesh.vertex_count();
    std::cout << "Neuron MBR by mesh:\n";
    output_mbr (std::cout, mbr_by_mesh_low, mbr_by_mesh_high);
    std::cout << "Morphology MBR by mesh:\n";
    output_mbr (std::cout, morph_mbr_by_mesh_low, morph_mbr_by_mesh_high);
    std::cout << boost::format("Neuron position: %1% %2% %3%\n")
        % neuron.position().x() % neuron.position().y()
        % neuron.position().z();
}
