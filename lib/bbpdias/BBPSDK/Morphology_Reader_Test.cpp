#include <iostream>
#include <fstream>
#include <iomanip>

// Requires Blue_Data/include/ and Blue_Data/src in the header search path
#include "BBP/Model/Microcircuit/Readers/Morphology_Reader.h"
#include "BBP/Model/Microcircuit/Containers/Morphologies.h"
#include "Parsers/Morphology_HDF5_File_Parser.h"

using namespace bbp;


void test_reader(const char * file_path);

void test_parser(const char * file_name, std::ostream & out);

int main(int argc, char * argv[])
{
    if (argc > 1) {
        std::cerr << "Dumping h5dump replica to '" << argv[1] << "' ... ";
        std::fstream out(argv[1], std::ios_base::out);
        if (out) {
            test_parser("h5/test.h5", out);
            std::cout << "done" << std::endl;
        } else {
            std::cerr << "Error opening output file for writing." << std::endl;
        }
    }

    test_reader(".");
}

void test_reader_using_target(const char * morphology_path,
                              const char * circuit_path)
{
    Morphology_Reader_Ptr reader = 
        Morphology_Reader::create_reader(morphology_path);
    reader->open();
    Morphologies morphologies;
    Cell_Target target;
    target.insert(1);
    reader->load(morphologies, target, circuit_path);
    bbp_assert(morphologies.size() == 1);

}

void test_reader(const char * file_path)
{
    Morphology_Reader_Ptr reader = Morphology_Reader::create_reader(file_path);
    reader->open();
    Morphologies morphologies;
    std::set<Label> names;
    names.insert("test");
    reader->load(morphologies, names);
    bbp_assert(morphologies.size() == 1);

    bbp_assert(morphologies.find("test") != morphologies.end());
    const Morphology &morphology = *morphologies.find("test");

    std::cerr << morphology << std::endl;
    std::cerr << "Offsets " << morphology.soma_offset() << ' '
              << morphology.axon_offset() << ' ' 
              << morphology.basal_dendrite_offset() << ' '
              << morphology.apical_dendrite_offset() << std::endl;

#ifndef NDEBUG        
    Morphology_Point_ID point_count = 0;
    for (Section_ID i = 0; i < morphology.number_of_sections(); i++)
        point_count += morphology.section_point_counts()[i];
    bbp_assert(point_count == morphology.number_of_points());

    for (Section_ID i = 0; i < morphology.number_of_sections(); i++) {
        Morphology_Point_ID offset = morphology.section_start_points()[i];
        Morphology_Point_ID max = morphology.section_point_counts()[i];

        const Section_Normalized_Distance * distances = 
            &morphology.point_relative_distances()[offset];
        for (Morphology_Point_ID j = 0; j < max - 1; j++) {
            bbp_assert(distances[j] >= 0 && distances[j] <= 1);
            bbp_assert(distances[j] <= distances[j + 1]);
                
        }
        bbp_assert(distances[max - 1] == 1);
    }
#endif
}

void test_parser(const char * file_name, std::ostream & out)
{
    Morphology_HDF5_File_Parser  parser;
    Morphology_Point_ID       point_count;
    Section_ID                section_count;
    Morphology_Point_ID_Array section_start_points;
    Section_Type_Array           section_types;
    Section_ID_Array          section_parents;
    Vector_3D_Micron_Array         point_positions;
    Micron_Array                 point_diameters;

    parser.read_morphology(file_name, point_count, section_count,
                           section_start_points, section_types, section_parents,
                           point_positions, point_diameters);

    // Mimicing h5dump test.h5 output to use diff
    out << std::setw(2);
    out << "HDF5 \"test.h5\" {" << std::endl
        << "GROUP \"/\" {" << std::endl
        << "   DATASET \"points\" {" << std::endl
        << "      DATATYPE  H5T_IEEE_F64LE" << std::endl
        << "      DATASPACE  SIMPLE { ( " << point_count << ", 4 ) / ( " 
        << point_count << ", 4 ) }" << std::endl
        << "      DATA {" << std::endl;
    for (int i = 0; i < point_count ; i++) {
        Vector_3D<Micron> v = point_positions[i];
        out << "      (" << i << ",0): " 
            << v.x() << ", " << v.y() << ", " << v.z() << ", "
            << point_diameters[i] << (i == point_count - 1 ? "" : ",") 
            << std::endl;
    }
    out << "      }" << std::endl
        << "   }" << std::endl
        << "   DATASET \"structure\" {" << std::endl
        << "      DATATYPE  H5T_STD_I32LE" << std::endl
        << "      DATASPACE  SIMPLE { ( " << section_count
        << ", 3 ) / ( " << section_count << ", 3 ) }" << std::endl
        << "      DATA {" << std::endl;
    for (int i = 0; i < section_count ; i++) {
        out << "      (" << i << ",0): " 
            << section_start_points[i] <<", "
            << section_types[i] << ", "
            << (section_parents[i] == UNDEFINED_SECTION_ID ? 
                -1 : section_parents[i])
            << (i == section_count - 1 ? "" : ",") << std::endl;
    }
    out << "      }" << std::endl
        << "   }" << std::endl
        << "}" << std::endl
        << "}" << std::endl;
}

