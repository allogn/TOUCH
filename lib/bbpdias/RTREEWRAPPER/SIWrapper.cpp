#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/reference_existing_object.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "SI.hpp"

using namespace boost::python;

int f(int x) {
  return x+3;
}


BOOST_PYTHON_MODULE(SI)
{
   class_<SI>("SpatialIndex", no_init) 
	;

    class_<std::vector<string> >("stl_vector_of_strings")
        .def(vector_indexing_suite<std::vector<string> >())
;

 //   class_<bbp::Box<Micron>>("Box_Micron")
  //  ;


   def("loadIndex", &SI::loadIndex, return_value_policy<reference_existing_object>());
   def("indexData", &SI::indexData);
   def("rangeQuery", &SI::rangeQuery);

   def("f",f);
}
