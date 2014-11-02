#ifndef TILESORT
#define TILESORT

#include <cstring>

#include <boost/shared_ptr.hpp>
#include "SVAReader.hpp"
#include "libbbpdias/TemporaryFile.hpp"

using namespace std;

namespace dias {

class TileSort
{
public:
	unsigned size;
	vector<Farhan::TemporaryFile*> buckets;
	vector<Vertex_info*> buffer;
	Farhan::TemporaryFile* tilesorted;
	int VerticesPerPage;
	int objects;
	Vertex_info* next;

	TileSort(unsigned recordsInMemory,int verticesPerPage)
	{
		size = recordsInMemory;
		buffer.reserve(size);
		buckets.clear();
		buffer.clear();
		VerticesPerPage = verticesPerPage;
		tilesorted = new Farhan::TemporaryFile();
		next=NULL;
	}

	~TileSort()
	{
		for (std::vector<Farhan::TemporaryFile*>::iterator it = buckets.begin();it !=buckets.end();++it)
			delete *it;
		buckets.clear();
		for (std::vector<Vertex_info*>::iterator it = buffer.begin();it !=buffer.end();++it)
			delete *it;
		buffer.clear();
		delete tilesorted;
	}

	void Rewind()
	{
		tilesorted->rewindForReading();
	}

	void Insert(Vertex_info* v)
	{
		buffer.push_back(v);
		if (buffer.size() >= size)
			makeBucket();
	}

	void makeBucket()
	{
		std::sort(buffer.begin(), buffer.end(), Vertex_info::xAsc());
		Farhan::TemporaryFile* tf = new Farhan::TemporaryFile();
        std::cerr << "\tSortTile Temporary file: " << tf->m_sFile << "\n";
		for (std::vector<Vertex_info*>::iterator it = buffer.begin();it !=buffer.end();++it)
		{
            (*it)->store(*tf);
			delete (*it);
		}
		buffer.clear();
		tf->rewindForReading();
		buckets.push_back(tf);
	}

	bool hasNext()
	{
		next = new Vertex_info();
		if (next->load(*tilesorted)==false)
			return false;
		return true;
	}

	Vertex_info* getNext()
	{
		return next;
	}

	int sortTile()
	{
		Farhan::TemporaryFile* xsorted = new Farhan::TemporaryFile();

		int count = sort(xsorted,0);
		double split = pow ((count+0.0)/(VerticesPerPage+0.0),1.0/3.0);

		xsorted->rewindForReading();

	    int YbucketSize = count / split;
	    int ZbucketSize = YbucketSize  / split;
	    int counter=0;
		vector<Vertex_info*> minibuffer;

	    while (counter < count)
	    {
	    	  for (int i=0;i<YbucketSize;i++)
	    	  {
	    		  Vertex_info* temp = new Vertex_info();
	    		  if (temp->load(*xsorted)==false)
	    		  {
	  				delete temp;
	  				temp=NULL;
	  				break;
	    		  }
	    		  buffer.push_back(temp);
	       		  counter++;
	       		  if (counter >= count) break;
	    	  }

	    	  std::sort(buffer.begin(),buffer.end(),Vertex_info::yAsc());

	    	  int zcounter=0;
	    	  while (zcounter<buffer.size())
	    	  {
	    		  for (int i=0;i<ZbucketSize;i++)
		    	  {
		    		  if (zcounter>=buffer.size()) break;
		    		  minibuffer.push_back(buffer.at(zcounter++));
		    	  }

	    		  std::sort(minibuffer.begin(),minibuffer.end(),Vertex_info::zAsc());

		    	  for (std::vector<Vertex_info*>::const_iterator mt = minibuffer.begin();mt != minibuffer.end();++mt)
		    	  {
		    		  (*mt)->store(*tilesorted);
		    		  delete *mt;
		    	  }
	    		  minibuffer.clear();
	    	  }
	    	  buffer.clear();
	    }
		for (vector<Farhan::TemporaryFile*>::iterator it = buckets.begin();it !=buckets.end();++it) delete *it;
		buckets.clear();
		for (vector<Vertex_info*>::iterator it = buffer.begin();it !=buffer.end();++it) delete *it;
		buffer.clear();

		delete xsorted;
		tilesorted->rewindForReading();
		return counter;
	}

	int sort(Farhan::TemporaryFile* merge,int sortDimension)
	{
		int count=0;
		if (buffer.empty()==false)
			makeBucket();

		int Bsize = buckets.size();
		buffer.reserve(Bsize);
		int eofCount=0;
		int outindex=0;

		for (int i=0;i<Bsize;i++)
		{
			Vertex_info* temp = new Vertex_info();
			if (temp->load(*(buckets.at(i)))==false)
			{
				delete buckets.at(i);
				buckets.at(i)=NULL;
				delete temp;
				temp=NULL;
				eofCount++;
			}
			buffer.push_back(temp);
		}

		while (eofCount<Bsize)
		{
			// set pointer to first item
			for (int i=0;i<Bsize;i++)
				if (buckets.at(i)!=NULL) {outindex=i;break;}
			if (outindex==Bsize) break;

			// find min item
			for (int i=0;i<Bsize;i++)
				if (buckets.at(i)!=NULL)
					if (buffer.at(i)->vi.coords[sortDimension] < buffer.at(outindex)->vi.coords[sortDimension]) outindex=i;

			//write X Sorted Content.
			buffer.at(outindex)->store(*merge);
			count++;
			delete buffer.at(outindex);

			Vertex_info* temp = new Vertex_info();
			if (temp->load(*(buckets.at(outindex)))==false)
			{
				delete buckets.at(outindex);
				buckets.at(outindex)=NULL;
				delete temp;
				temp=NULL;
				eofCount++;
			}
			buffer.at(outindex) = temp; // TODO Assignment operator there?
		}

		for (vector<Farhan::TemporaryFile*>::iterator it = buckets.begin();it !=buckets.end();++it) delete *it;
		buckets.clear();
		for (vector<Vertex_info*>::iterator it = buffer.begin();it !=buffer.end();++it) delete *it;
		buffer.clear();

		return count;
	}

};

}

#endif
