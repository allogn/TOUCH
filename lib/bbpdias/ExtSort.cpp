#ifndef EXTRSORT
#define EXTRSORT

#include <boost/shared_ptr.hpp>

#include "delaunay_connectivity.hpp"
#include "libbbpdias/TemporaryFile.hpp"
#include "libbbpdias/Hilbert.hpp"
#include "Vertex_info.hpp"

namespace dias {

typedef void (*process_sorted_output_fn) (vertex_id,
                                          const struct vertex_info &);
    
class ExtSort
{
public:
	unsigned size;
	std::vector<Farhan::TemporaryFile*> buckets;
	std::vector<Vertex_info*> buffer;

	ExtSort(unsigned recordsInMemory)
	{
		size = recordsInMemory;
		buffer.reserve(size);

		buckets.clear();
		buffer.clear();
	}

	~ExtSort()
	{
	//cout << "\nDestroying External sort";
		for (std::vector<Farhan::TemporaryFile*>::iterator it = buckets.begin();it !=buckets.end();++it)
			delete *it;
		buckets.clear();
		for (std::vector<Vertex_info*>::iterator it = buffer.begin();it !=buffer.end();++it)
			delete *it;
		buffer.clear();
	}

	void makeBucket()
	{

		std::cerr << "\nmaking a bucket";

		std::sort(buffer.begin(), buffer.end(), Vertex_info::hilbertAsc());
#ifdef VERBOSE
                std::cerr << "\nsorted bucket";
#endif
		Farhan::TemporaryFile* tf = new Farhan::TemporaryFile();

                std::cerr << "\n bucket file: " << tf->m_sFile;

		for (std::vector<Vertex_info*>::iterator it = buffer.begin();it !=buffer.end();++it)
		{
                        (*it)->store(*tf);
			delete (*it);
		}
		buffer.clear();
		tf->rewindForReading();
		buckets.push_back(tf);
	}

	void insert(Vertex_info* v)
	{
		buffer.push_back(v);

		// this will create the initial, sorted buckets
		if (buffer.size() >= size)
			makeBucket();
	}

    int sort (process_sorted_output_fn process_sorted_output)
	{

		std::cerr << "\nstart sorting";

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
					{
					double d1[3],d2[3];
					d1[0] = buffer.at(i)->vi.coords[0];
					d1[1] = buffer.at(i)->vi.coords[1];
					d1[2] = buffer.at(i)->vi.coords[2];

					d2[0] = buffer.at(outindex)->vi.coords[0];
					d2[1] = buffer.at(outindex)->vi.coords[1];
					d2[2] = buffer.at(outindex)->vi.coords[2];

					if (hilbert_ieee_cmp(3,d1,d2)>=0)  outindex=i;
					}


			//Sorted Output
                        (process_sorted_output) (buffer.at(outindex)->vid,
                                                 buffer.at(outindex)->vi);
#ifdef VERBOSE
                        std::cerr << buffer.at(outindex)->vi.coords[0] << " " << buffer.at(outindex)->vi.coords[1] << " " << buffer.at(outindex)->vi.coords[2]<< std::endl;
#endif
                        if (count%10000000==0 &&count>0) std::cout << "\n Points Sorted: " << count;
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
			buffer.at(outindex) = temp;
		}
		return count;
	}

};

}

#endif
