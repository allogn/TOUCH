#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/reference_existing_object.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "FLATIndex.hpp"
#include <boost/python/numeric.hpp>

using namespace boost::python;
using namespace FLAT;
using namespace std;

std::vector< std::vector<float> > foo(int x)
{
	std::vector< std::vector<float> > ret(x,vector<float>(x) );
	for (int j=0;j<x;j++)
		for (int i=0;i<x;i++)
		{
			ret[i][j] =1;
		}
  	return ret;
}

BOOST_PYTHON_MODULE(FLATIndex)
{
   class_<std::vector<std::vector<float> > > ("v2_double")
        .def(vector_indexing_suite<std::vector<std::vector<float> > >());

   class_<std::vector<float> > ("v_double")
        .def(vector_indexing_suite<std::vector<float> >());

   class_<FLATIndex>("SpatialIndex", no_init);

   class_<std::vector<string> >("stl_vector_of_strings")
        .def(vector_indexing_suite<std::vector<string> >());

   class_<std::vector<int> >("neuron_id_set")
        .def(vector_indexing_suite<std::vector<int> >());

   def("loadIndex", &FLATIndex::loadIndex, return_value_policy<reference_existing_object>());
   def("loadIndexMemory", &FLATIndex::loadIndexMemory, return_value_policy<reference_existing_object>());
   def("buildIndex", &FLATIndex::buildIndex);
   def("windowQuery", &FLATIndex::windowQuery);
   def("vicinityQuery", &FLATIndex::vicinityQuery);
   def("getNeuronIds", &FLATIndex::getNeuronIds);
   def("unLoadIndex", &FLATIndex::unLoadIndex);
   def("foo",foo);
}
