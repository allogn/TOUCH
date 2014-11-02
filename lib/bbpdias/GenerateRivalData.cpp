#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Neuron.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

#include "Cone.cpp"

using namespace std;

int main(int argc, const char* argv[])
{
	const bbp::URI blue_config_filename(argv[1]);
	bbp::Experiment experiment;
	experiment.open(blue_config_filename);
	bbp::Microcircuit & microcircuit = experiment.microcircuit();
	const bbp::Targets & targets = experiment.targets();
	const bbp::Cell_Target target = targets.cell_target("Column");
	microcircuit.load(target, bbp::NEURONS | bbp::MORPHOLOGIES);

	int Max = atoi(argv[2]);
	int steps = atoi(argv[3]);
	string type = argv[4];

	cout << "\nLoaded BBP SDK Data";

	for (int i=1;i<steps+1;i++)
	{
		int maxObjects = (Max/steps)*i;
		int countObjects=0;
		stringstream strstream;
		strstream << "Raw" << type << "Data" << maxObjects << ".txt";
		ofstream dFileT(strstream.str().c_str());
		dFileT << fixed << showpoint << setprecision(6);

		bbp::Neurons & myNeurons = microcircuit.neurons();
		bbp::Neurons::iterator myNeuronsEnd = myNeurons.end();
		for (bbp::Neurons::iterator i = myNeurons.begin(); i != myNeuronsEnd; ++i)
		{
			if (countObjects>=maxObjects) break;
			bbp::Transform_3D<bbp::Micron> trafo = i->global_transform();

			if (type.compare("mesh")==0)
			{
				// Generate Mesh Data
				const bbp::Mesh &myMesh = i->morphology().mesh();
				bbp::Array<bbp::Vertex_Index>::const_iterator myMeshEnd = myMesh.triangles().end();
				bbp::Array<bbp::Vector_3D<bbp::Micron> > vertices = myMesh.vertices();
				for (bbp::Array<bbp::Vertex_Index>::const_iterator m = myMesh.triangles().begin(); m != myMeshEnd; ++m)
				{
					if (countObjects>=maxObjects) break;
					bbp::Vector_3D<bbp::Micron> v1 = trafo * vertices[*m++];
					bbp::Vector_3D<bbp::Micron> v2 = trafo * vertices[*m++];
					bbp::Vector_3D<bbp::Micron> v3 = trafo * vertices[*m];

                                        dias::Box boundingBox;
                                        vector<dias::Vertex> triangle;
                                        triangle.push_back(dias::Vertex(v1.x(),v1.y(),v1.z()));
                                        triangle.push_back(dias::Vertex(v2.x(),v2.y(),v2.z()));
                                        triangle.push_back(dias::Vertex(v3.x(),v3.y(),v3.z()));
                                        dias::Box::boundingBox(boundingBox,triangle);

					dFileT << boundingBox.low[0] << " " << boundingBox.low[1] << " " << boundingBox.low[2] << " "
					<< boundingBox.high[0] << " " << boundingBox.high[1] << " " << boundingBox.high[2] << endl;

					countObjects++;
				}
			}
			else
			{
				// Generate Segment Data
				bbp::Sections mySections = i->morphology().all_sections();
				bbp::Sections::iterator mySectionsEnd = mySections.end();
				for (bbp::Sections::iterator s = mySections.begin(); s != mySectionsEnd; ++s)
				{
					if (countObjects>=maxObjects) break;
					bbp::Segments segments = s->segments();
					bbp::Segments::const_iterator segments_end = segments.end();
					for (bbp::Segments::const_iterator j = segments.begin(); j != segments_end ; ++j)
					{
						if (countObjects>=maxObjects) break;
						bbp::Vector_3D<bbp::Micron> gbegin = trafo * j->begin().center();
						bbp::Vector_3D<bbp::Micron> gend   = trafo * j->end().center();

                                                dias::Vertex begin = dias::Vertex(gbegin.x(),gbegin.y(),gbegin.z());
                                                dias::Vertex end   = dias::Vertex(gend.x(),gend.y(),gend.z());
                                                dias::Cone   segment  = dias::Cone(begin,end,j->begin().radius(),j->end().radius());

                                                dias::Box boundingBox;
                                                dias::Cone::boundingBox(segment,boundingBox);

						dFileT << boundingBox.low[0] << " " << boundingBox.low[1] << " " << boundingBox.low[2] << " "
							   << boundingBox.high[0] << " " << boundingBox.high[1] << " " << boundingBox.high[2] << endl;

						countObjects++;
					}
				}
			}
		}

		dFileT.close();

		cout << "\nFile Saved: " << strstream.str();
	}
}
