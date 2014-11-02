#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Neuron.h>
#include "ExternalSort.cpp"
#include "Data.cpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip.h>

using namespace bbp;
using namespace std;
using namespace dias;



int main(int argc, const char* argv[])
{
	const bbp::URI blue_config_filename(argv[1]);
	bbp::Experiment experiment;
    experiment.open(blue_config_filename);
    bbp::Microcircuit & microcircuit = experiment.microcircuit();
    const bbp::Targets & targets = experiment.targets();
    const bbp::Cell_Target target = targets.cell_target("Column");
    microcircuit.load(target, bbp::NEURONS | bbp::MORPHOLOGIES);

    int memoryfootprint = 400*1024*1024/(sizeof(Data)-4);
    int Max = atoi(argv[2]);
    int steps = atoi(argv[3]);

    cout << "\nLoaded BBP SDK Data";

    for (int i=1;i<steps+1;i++)
    {
    float max[3],min[3];
    max[0]=-100000;min[0]=100000;
    max[1]=-100000;min[1]=100000;
    max[2]=-100000;min[2]=100000;

    ExternalSort* es = new ExternalSort(memoryfootprint);
    int maxObjects = (Max/steps)*i;
    int countObjects=0;
    stringstream strstream;
    strstream << "RawData" << maxObjects << ".txt";
    ofstream dFileT(strstream.str().c_str());
    dFileT << fixed << showpoint << setprecision(6);

    Neurons & myNeurons = microcircuit.neurons();
    Neurons::iterator myNeuronsEnd = myNeurons.end();
      for (Neurons::iterator i = myNeurons.begin(); i != myNeuronsEnd; ++i)
        {
    	  if (countObjects>=maxObjects) break;
        	Transform_3D<Micron> trafo = i->global_transform();

        	/*// Generate Mesh Data
        	const bbp::Mesh &myMesh = i->morphology().mesh();
        	Array<bbp::Vertex_Index>::const_iterator myMeshEnd = myMesh.triangles().end();
        	Array<bbp::Vector_3D<bbp::Micron> > vertices = myMesh.vertices();
        	for (Array<bbp::Vertex_Index>::const_iterator m = myMesh.triangles().begin(); m != myMeshEnd; ++m)
        	{
        		if (countObjects>=maxObjects) break;
            	Vector_3D<bbp::Micron> v1 = trafo * vertices[*m++];
        		Vector_3D<bbp::Micron> v2 = trafo * vertices[*m++];
        		Vector_3D<bbp::Micron> v3 = trafo * vertices[*m];

        		Vertex mid;
        		//http://en.wikipedia.org/wiki/Centroid
        		mid[0] = (v1.x()+v2.x()+v3.x())/3;
        		mid[1] = (v1.y()+v2.y()+v3.y())/3;
        		mid[2] = (v1.z()+v2.z()+v3.z())/3;
        		if (mid[0]>max[0]) max[0] = mid[0];
          		if (mid[1]>max[1]) max[1] = mid[1];
          		if (mid[2]>max[2]) max[2] = mid[2];

          		if (mid[0]<min[0]) min[0] = mid[0];
          		if (mid[1]<min[1]) min[1] = mid[1];
          		if (mid[2]<min[2]) min[2] = mid[2];

        		es->insert(Data(mid,countObjects));
        		countObjects++;
        	}
*/

        	// Generate Segment Data
        	Sections mySections = i->morphology().all_sections();
        	Sections::iterator mySectionsEnd = mySections.end();
        	for (Sections::iterator s = mySections.begin(); s != mySectionsEnd; ++s)
        	{
        	  if (countObjects>=maxObjects) break;
              Segments segments = s->segments();
        		Segments::const_iterator segments_end = segments.end();
        		for (Segments::const_iterator j = segments.begin(); j != segments_end ; ++j)
        		{
        		if (countObjects>=maxObjects) break;
        		bbp::Vector_3D<bbp::Micron> gbegin = trafo * j->begin().center();
           		bbp::Vector_3D<bbp::Micron> gend   = trafo * j->end().center();
           		Vertex mid;
           		mid[0] = (gbegin.x()+gend.x())/2;
          		mid[1] = (gbegin.y()+gend.y())/2;
          		mid[2] = (gbegin.z()+gend.z())/2;

          		if (mid[0]>max[0]) max[0] = mid[0];
          		if (mid[1]>max[1]) max[1] = mid[1];
          		if (mid[2]>max[2]) max[2] = mid[2];

          		if (mid[0]<min[0]) min[0] = mid[0];
          		if (mid[1]<min[1]) min[1] = mid[1];
          		if (mid[2]<min[2]) min[2] = mid[2];

          		es->insert(Data(mid,countObjects));
          		countObjects++;
        		}
        	}
        }

      dFileT << "# npoints " << countObjects << endl;
      dFileT << "# bb_min_f "<< min[0] << " " << min[1] << " " << min[2] <<endl;
      dFileT << "# bb_max_f "<< max[0] << " " << max[1] << " " << max[2] <<endl;

      //int c =  es->sortTile(dFileT,split);
      int c =  es->sort(dFileT);
      dFileT.close();

      delete es;
      cout << "\nFile Saved: " << strstream.str();
    }
}
