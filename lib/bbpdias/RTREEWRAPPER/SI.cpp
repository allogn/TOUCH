#include "SI.hpp"
#include "bbp_tools.hpp"


using namespace bbp;
using namespace std;
using namespace SpatialIndex;
using namespace dias;


class rtreeVisitor : public SpatialIndex::IVisitor
{
public:

    vector<string> * _result;

    rtreeVisitor(vector<string>* result)
		{
			_result = result;
		}

    virtual void visitNode(const SpatialIndex::INode& in) 
	{
	}

    virtual bool doneVisiting() 
    {
    	return false;
    }

    virtual void visitData(const SpatialIndex::IData& in) 
    {
		byte *b;
		uint32_t l;
		in.getData(l, &b);

		int neuron = SI::readInteger(0, b);
		int section = SI::readInteger(4, b);
		int segment = SI::readInteger(8, b);

		std::stringstream ss;
		ss << "NEURON ID: " << neuron << " SECTION ID:" << section << " SEGMENT ID:" << segment;

		/*IShape* r;
		in.getShape(&r);
		Region mbr;
		r->getMBR(mbr);
		
		cout << "Data : (" << ss.str() 
                     << ") MBR= [" << mbr.m_pLow[0] << "," << mbr.m_pLow[1] << "," << mbr.m_pLow[2] << "] ["
                     << mbr.m_pHigh[0] << "," << mbr.m_pHigh[1] << "," << mbr.m_pHigh[2] << "]\n";
		*/
		_result->push_back(ss.str());

    }

    virtual void visitData(const SpatialIndex::IData& in, SpatialIndex::id_type id) {

    }

    virtual void visitData(std::vector<const SpatialIndex::IData *>& v
                            __attribute__((__unused__))) 
	{
	}
};

class stream_base : public IDataStream
{
protected:
  unsigned limit;
  unsigned i;
  bool have_next;

  std::auto_ptr<global_transformer> gtrafo;

  Neurons::iterator n_i;
  Neurons::iterator n_e;

  stream_base (Neurons * n, unsigned _limit) :
    limit(_limit),
    i(0),
    have_next(true),
    n_i(n->begin()),
    n_e(n->end())
  {
  }

  virtual void start_neuron ()
  {
    gtrafo.reset(new global_transformer(n_i->global_transform()));
  }

  void advance_neuron ()
  {
    n_i++;
    if (n_i == n_e)
      {
	have_next = false;
	return;
      }
    start_neuron();
  }

public:
  int objs_loaded() const
  {
    return i;
  }

  virtual bool hasNext()
  {
    return have_next;
  }

  virtual uint32_t size()
  {
    abort();
  }

  virtual void rewind()
  {
    abort();
  }
};

class cylinder_stream : public stream_base
{
private:
  std::auto_ptr<Sections> sections;
  unsigned section_count;
  unsigned segment_count;
  Sections::iterator s_i;
  Sections::iterator s_e;
  std::auto_ptr<Segments> segments;
  Segments::const_iterator g_i;
  Segments::const_iterator g_e;

  void init_segments ()
  {
    segments.reset(new Segments(s_i->segments()));
    g_i = segments->begin();
    g_e = segments->end();
    if (g_i == g_e)
      advance_section();
  }

  virtual void start_neuron ()
  {
    stream_base::start_neuron();
    sections.reset(new Sections(n_i->morphology().all_sections()));
    s_i = sections->begin();
    s_e = sections->end();
    init_segments();
  }

  void advance_section ()
  {
    s_i++;
    if (s_i == s_e)
      {
	advance_neuron();
	return;
      }
    init_segments();
  }

public:
  cylinder_stream (Neurons * n, unsigned _limit) :
    stream_base(n, _limit)
  {
    start_neuron();
  }

  virtual IData* getNext()
  {
    assert (g_i != g_e);
    assert (have_next);

    Region r = get_segment_mbr(*g_i, *gtrafo).as_region();

    int length = 3*sizeof(int);
    byte * buffer = new byte[length];

    SI::writeInteger(n_i->gid(), 0, buffer);
    SI::writeInteger(s_i->id(), 4, buffer);
    SI::writeInteger(g_i->id(), 8, buffer);
    //cout << "INDEXING: " << n_i->gid() << " - " << s_i->id() << " - " << g_i->id() << "\n";

    id_type id = 0;

    IData * res = new SpatialIndex::RTree::Data(length, buffer, r, id);

    i++;
    if (i == limit)
      have_next = false;

    g_i++;
    if (g_i == g_e)
      advance_section();

    return res;
  }
};

class mesh_stream : public stream_base
{
private:
  bbp::Array<bbp::Vertex_Index>::const_iterator t;
  bbp::Array<bbp::Vertex_Index>::const_iterator e;

  vertex_array vertices;

  virtual void start_neuron()
  {
    stream_base::start_neuron();
    const bbp::Mesh & mesh = n_i->morphology().mesh();
    t = mesh.triangles().begin();
    e = mesh.triangles().end();
    vertices = mesh.vertices();
  }

public:
  mesh_stream (Neurons * n, unsigned _limit) :
    stream_base(n, _limit)
  {
    start_neuron();
  }

  virtual IData* getNext()
  {
    assert (t != e);
    assert (have_next);

    triangle_ids tids = { {*t++, *t++, *t++} };
    Region r = get_triangle_mbr (tids, vertices, *gtrafo).as_region();

    int length = 0;
    byte * buffer = new byte[length];

    id_type id = 0;

    IData * result = new SpatialIndex::RTree::Data(length, buffer, r, id);

    i++;
    if (i == limit)
      have_next = false;

    if (t == e)
      advance_neuron();

    return result;
  }

};


SI::SI(SpatialIndex::ISpatialIndex * tree)
{
	_tree = tree;
}

SI* SI::loadIndex(std::string index)
{
	try
	{
	SpatialIndex::id_type indexIdentifier=1;

	IStorageManager* diskfile = StorageManager::loadDiskStorageManager(index);

	ISpatialIndex * tree = RTree::loadRTree(*diskfile, indexIdentifier);

	SI * psi = new SI(tree);
	return psi;
	}
	catch(...)
	{
		cout << "Cannot Load Index file";
		exit(1);
	}
}

void SI::indexData(std::string source, std::string destination)
{
	URI blue_config_filename(source);
	try
	{
		Experiment experiment;
		experiment.open(blue_config_filename);
		Microcircuit & microcircuit = experiment.microcircuit();
		const Targets & targets = experiment.targets();
		Cell_Target target = targets.cell_target("Column");
		microcircuit.load(target, NEURONS | MORPHOLOGIES);
		Neurons myNeurons = microcircuit.neurons();

	stream_base * data_stream = new cylinder_stream(&myNeurons,-1);

	SpatialIndex::id_type indexIdentifier=1;

	IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(destination, 4096);

	RTree::createAndBulkLoadNewRTree(RTree::BLM_STR, *data_stream, *diskfile, 0.7, 100, 100, 3, RTree::RV_RSTAR, indexIdentifier);

	std::cout << "\nTOTAL Objects Indexed: " << data_stream->objs_loaded() << std::endl;
	//std::cout << "Universe Bounds: " << data_stream->universe << std::endl;

	delete diskfile;
	}
	catch(...)
	{
		cout << "Cannot Load BBP circuit";
		exit(1);
	}
}

vector<string> SI::rangeQuery(SI* si,float xlo,float ylo,float zlo,float xhi,float yhi,float zhi)
{
	try
	{
		vector<string> result;
			double low[3], high[3];

		//low[0] = query.minimum().x();
		//low[1] = query.minimum().y();
		//low[2] = query.minimum().z();

		//high[0] = query.maximum().x();
		//high[1] = query.maximum().y();
		//high[2] = query.maximum().z();

		low[0] = xlo;
		low[1] = ylo;
		low[2] = zlo;

		high[0] = xhi;
		high[1] = yhi;
		high[2] = zhi;

		SpatialIndex::Region qr = SpatialIndex::Region(low, high, 3);

		rtreeVisitor rv = rtreeVisitor(&result);

		si->_tree->intersectsWithQuery(qr, rv);

		std::cout << "\nRange Query results: " << result.size();

		return result;
	}
	catch(...)
	{
		cout << "Cannot access Index pointer";
		exit(1);
	}

}
