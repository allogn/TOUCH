#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Neuron.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_hierarchy_3.h>
#include <CGAL/Triangulation_3.h>
#include <CGAL/Tetrahedron_3.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <iomanip>
#include "libbbpdias/Box.hpp"
#include "Data.cpp"
#include "Query.cpp"
#include "cpu_timer.cpp"

using namespace std;
using namespace dias;

template < class GT, class Vb = CGAL::Triangulation_vertex_base_3<GT> >
class MyVertex
  : public Vb
{
public:
  typedef typename Vb::Vertex_handle  Vertex_handle;
  typedef typename Vb::Cell_handle    Cell_handle;
  typedef typename Vb::Point          Point;

  template < class TDS2 >
  struct Rebind_TDS {
    typedef typename Vb::template Rebind_TDS<TDS2>::Other  Vb2;
    typedef MyVertex<GT, Vb2>                        Other;
  };

  MyVertex() {}

  MyVertex(const Point& p)
    : Vb(p) {}

  MyVertex(const Point& p, Cell_handle c)
    : Vb(p, c) {}

  int id;
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
//typedef CGAL::Triangulation_vertex_base_3<K>             Vb;
typedef MyVertex<K> mVb;
typedef CGAL::Triangulation_hierarchy_vertex_base_3<mVb>  Vbh;
typedef CGAL::Triangulation_data_structure_3<Vbh>        Tds;

typedef CGAL::Delaunay_triangulation_3<K,Tds>            Dt;
typedef CGAL::Triangulation_hierarchy_3<Dt>              Dh;
typedef Dh::Point Point;

#define QUERIES 200

class GraphNode
{
public:
	dias::Data data;
	dias::Box voronoiBB;
	vector<dias::Vertex> voronoi;
	vector<int> neighbours;
};

int main(int argc __attribute__((unused)), const char* argv[])
{

	string BBPfile = argv[1];
	string outputFile = argv[2];
	string maxObjects = argv[3];
    tpie::cpu_timer c;

	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><< MAKING QUERIES <><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    Box queryextent = Box(Vertex(98.2538, 1005.14, 92.9046),
        			      Vertex(452.301, 1385.56, 456.743));
    vector<Query> queries;

    for (int i=0;i<QUERIES;i++)
    {
        dias::Box random;
    	dias::Box::randomBox(queryextent,50,random);
    	dias::Query query = dias::Query(random);
    	query.leafAccesses = -1;
    	queries.push_back(query);
    }

	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><><> LOADING DATA <><><><><><><><><><><><><><><
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    std::vector<Point> data;
    std::cout << "Extracting Data" << "\n";

    c.start();
	const bbp::URI blue_config_filename(argv[1]);
	bbp::Experiment experiment;
    experiment.open(blue_config_filename);
    bbp::Microcircuit & microcircuit = experiment.microcircuit();
    const bbp::Targets & targets = experiment.targets();
    const bbp::Cell_Target target = targets.cell_target("Column");
    microcircuit.load(target, bbp::NEURONS | bbp::MORPHOLOGIES);
    int maxsegments =  atoi(maxObjects.c_str());
    int segmentid=0;

    bbp::Neurons & myNeurons = microcircuit.neurons();
    bbp::Neurons::iterator myNeuronsEnd = myNeurons.end();
    for (bbp::Neurons::iterator i = myNeurons.begin(); i != myNeuronsEnd; ++i)
      {
    	if (segmentid>=maxsegments) break;
    	bbp::Transform_3D<bbp::Micron> trafo = i->global_transform();
    	bbp::Sections mySections = i->morphology().all_sections();
    	bbp::Sections::iterator mySectionsEnd = mySections.end();
      	for (bbp::Sections::iterator s = mySections.begin(); s != mySectionsEnd; ++s)
      	{
      		if (segmentid>=maxsegments) break;
      		bbp::Segments segments = s->segments();
      		bbp::Segments::const_iterator segments_end = segments.end();
      		for (bbp::Segments::const_iterator j = segments.begin(); j != segments_end ; ++j)
      		{
      			if (segmentid>=maxsegments) break;
      			bbp::Vector_3D<bbp::Micron> gbegin = trafo * j->begin().center();
         		bbp::Vector_3D<bbp::Micron> gend   = trafo * j->end().center();

           		Vertex begin = Vertex(gbegin.x(),gbegin.y(),gbegin.z());
        		Vertex end   = Vertex(gend.x(),gend.y(),gend.z());
        		Cone   cone  = Cone(begin,end,j->begin().radius(),j->end().radius());
        		Vertex center;
        		Cone::center(cone,center);

    			for (vector<dias::Query>::iterator q = queries.begin();q!=queries.end();++q)
    				if (dias::Box::enclose(q->box,center))
        			{
    					if (q->leafAccesses<0) q->leafAccesses = segmentid;
    					q->points++;
         			}
    			data.push_back(Point(center[0],center[1],center[2]));

      			segmentid++;
      		}
      	}
      }

    c.stop();
    std::cout << "Data Extraction Done: " << c << " secs\n\n";

	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><< TRIANGULATING <>><><><><><><><><><><><><><><>
	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    std::cout << "Triangulating\n";
    c.reset();
    c.start();
    Dh T;
    T.insert(data.begin(),data.end());
    data.clear();
    c.stop();
    std::cout << "Triangulation Done: " << c << " secs\n\n";

    // <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    // <><><><><><><><><><><>< SAVING TO SVA FORMAT ><><><><><><><><><><><><><>
   	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    std::cout << "Saving to SVA Format\n";
    c.reset();
    c.start();
    ofstream svaFile(outputFile.c_str());
    svaFile << fixed << showpoint << setprecision(2);

    int ids=1;
    for (Dh::Finite_vertices_iterator v = T.finite_vertices_begin(); v != T.finite_vertices_end(); ++v)
		{
    	v->id = ids++;
    	svaFile << "v " << v->point().x() << " " << v->point().y() << " " << v->point().z() << endl;
		}

    for (Dh::Finite_cells_iterator c = T.finite_cells_begin(); c != T.finite_cells_end(); ++c)
		{
		Dh::Vertex_handle v1 = c->vertex(0);
		Dh::Vertex_handle v2 = c->vertex(1);
		Dh::Vertex_handle v3 = c->vertex(2);
		Dh::Vertex_handle v4 = c->vertex(3);

		svaFile << "c " << v1->id << " " << v2->id << " " << v3->id <<" " << v4->id << endl;
		}

    c.stop();
    std::cout << "Saving to SVA Format Done: " << c<<" secs\n\n";

    exit(0);

    // <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    // <><><><><><><><><><><>< CONVERTING TO GRAPH <><><><><><><><><><><><><><>
   	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    GraphNode* graph[segmentid];

    std::cout << "Processing Data\n";
    for (Dh::Finite_vertices_iterator v = T.finite_vertices_begin(); v != T.finite_vertices_end(); ++v)
		{
    	graph[v->id] = new GraphNode();
    	graph[v->id]->data.id = v->id;
    	Point p = v->point();
    	graph[v->id]->data.midPoint = dias::Vertex(p.x(),p.y(),p.z());

    	std::vector<Dh::Vertex_handle> vl;
    	std::vector<Dh::Cell_handle> cl;

    	T.finite_adjacent_vertices(v,std::back_inserter(vl));
    	T.finite_incident_cells(v,std::back_inserter(cl));

    	for (std::vector<Dh::Vertex_handle>::const_iterator a = vl.begin(); a != vl.end(); ++a)
    		graph[v->id]->neighbours.push_back((**a).id);

    	for (std::vector<Dh::Cell_handle>::const_iterator c = cl.begin(); c != cl.end(); ++c)
			{
    		const Dh::Tetrahedron t = T.tetrahedron(*c);
    		Dh::Point circumcenter = CGAL::circumcenter(t);
    		dias::Vertex vr(circumcenter.x(),circumcenter.y(),circumcenter.z());
    		graph[v->id]->voronoi.push_back(vr);
			}
    	dias::Box::boundingBox(graph[v->id]->voronoiBB,graph[v->id]->voronoi);
		}
    std::cout << "Done Processing\n\n";

    // <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    // <><><><><><><><><><><>< BREDTH FIRST SEARCH <><><><><><><><><><><><><><>
   	// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    std::cout << "Crawling Graph\n";

    for (vector<dias::Query>::iterator q = queries.begin();q!=queries.end();++q)
    {
        std::cout << "\n\nQuery: " << q->box << " Results: " << q->points << " Seed: " << q->leafAccesses;

        int PointsVisited=0;
        int QueryResult=0;
        queue<int> que;
        vector<int> visited;

        if (q->leafAccesses>=0)
        {
        que.push(q->leafAccesses);
        visited.push_back(q->leafAccesses);
        }
		while (que.empty()==false)
		{
		//visit node
			int visitor = que.front();
			que.pop();
			PointsVisited++;
			//std::cout << "\nVisiting: " << visitor << "Data Point: " <<graph[visitor]->data.midPoint;

			if (dias::Box::enclose(q->box,graph[visitor]->data.midPoint))
				QueryResult++;

		//check candidate for BFS
			for (vector<int>::iterator i = graph[visitor]->neighbours.begin();i!=graph[visitor]->neighbours.end();++i)
				if (dias::Box::overlap(graph[*i]->voronoiBB,q->box))
				{
                                    vector<int>::size_type j=0;
					for (j=0;j<visited.size();j++)
						if (*i==visited.at(j)) break;
					if (j==visited.size())
					{
						que.push(*i);
						visited.push_back(*i);
					}
				}
		}
        std::cout << "\nResult Set: " << QueryResult << " PointsCrawled: " << PointsVisited << " Overhead%: " << 100-(((QueryResult+0.0) / (PointsVisited+0.0))*100) ;


    }

    return 0;
}
