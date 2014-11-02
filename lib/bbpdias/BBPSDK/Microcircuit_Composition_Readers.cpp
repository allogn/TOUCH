#include <iostream>
#include <fstream>
#include <iomanip>

#include "BBP/Model/Microcircuit/Readers/Microcircuit_Composition_Reader.h"
#include "BBP/Model/Microcircuit/Containers/Neurons.h"


using namespace bbp;


void test_reader(const char * file_path);

int main(int argc, char * argv[])
{
    test_reader(argv[1]);
}

void test_reader(const char * file_path)
{
    Neurons neurons;
    Structure_Dataset_Ptr structure = Structure_Dataset_Ptr (new Structure_Dataset());
    Microcircuit_Composition_Reader_Ptr reader = Microcircuit_Composition_Reader::create_reader(file_path);
    bbp_assert(reader.get());

    reader->open();

    Cell_Target target;
    target.insert(1);
    target.insert(3);

    reader->load(neurons, target, structure);

//    for (Neurons::const_iterator neuron = neurons.begin();
//         neuron != neurons.end();
//         ++neuron)
//    {
//        std::cout << *neuron << std::endl;
//    }
}
