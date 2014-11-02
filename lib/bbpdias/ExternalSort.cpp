#ifndef EXTSORT
#define EXTSORT

#include "Data.cpp"
#include "TemporaryFile.cpp"

using namespace std;
using namespace Farhan;
using namespace dias;



class ExternalSort
{
public:
	int size;
	vector<TemporaryFile*> buckets;
	vector<Data*> buffer;

	ExternalSort(int recordsInMemory)
	{
		size = recordsInMemory;
		buffer.reserve(size);
		buckets.clear();
		buffer.clear();
	}

	~ExternalSort()
	{
	//cout << "\nDestroying External sort";
		for (vector<TemporaryFile*>::iterator it = buckets.begin();it !=buckets.end();++it)
			delete *it;
		buckets.clear();
		for (vector<Data*>::iterator it = buffer.begin();it !=buffer.end();++it)
			delete *it;
		buffer.clear();
	}

	void makeBucket()
	{
		std::sort(buffer.begin(), buffer.end(), Data::hilbertAsc());
		TemporaryFile* tf = new TemporaryFile();
		//cout << "\n storing file";
		for (vector<Data*>::iterator it = buffer.begin();it !=buffer.end();++it)
		{
			(*it)->store(*tf);
			delete (*it);
		}
		buffer.clear();
		tf->rewindForReading();
		buckets.push_back(tf);
	}

	void insert(const Data& d)
	{
		Data* a = new Data(d);
		buffer.push_back(a);

		// this will create the initial, sorted buckets
		if (buffer.size() >= size)
			makeBucket();
	}

	int sort(ofstream &file)
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
			Data* temp = new Data;
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

					d1[0] = buffer.at(i)->midPoint[0];
					d1[1] = buffer.at(i)->midPoint[1];
					d1[2] = buffer.at(i)->midPoint[2];

					d2[0] = buffer.at(outindex)->midPoint[0];
					d2[1] = buffer.at(outindex)->midPoint[1];
					d2[2] = buffer.at(outindex)->midPoint[2];

					if (hilbert_ieee_cmp(3,d1,d2)>=0)  outindex=i;
					//if (buffer.at(i)->midPoint[0] < buffer.at(outindex)->midPoint[0]) outindex=i;
					}
			//write output
			file << buffer.at(outindex)->midPoint[0] << " " << buffer.at(outindex)->midPoint[1] << " " << buffer.at(outindex)->midPoint[2]<< endl;
			if (count%10000000==0 &&count>0) cout << "\n Points Sorted: " << count;
			count++;
			delete buffer.at(outindex);

			Data* temp = new Data;
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

	int sortTile(ofstream &file,int split)
	{
		TemporaryFile* merge = new TemporaryFile();
		int count=0;
		if (buffer.empty()==false)
			makeBucket();

		int Bsize = buckets.size();
		buffer.reserve(Bsize);
		int eofCount=0;
		int outindex=0;

		for (int i=0;i<Bsize;i++)
		{
			Data* temp = new Data;
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
					if (buffer.at(i)->midPoint[0] < buffer.at(outindex)->midPoint[0]) outindex=i;

			//write output
			//file << buffer.at(outindex)->midPoint[0] << " " << buffer.at(outindex)->midPoint[1] << " " << buffer.at(outindex)->midPoint[2]<< endl;
			buffer.at(outindex)->store(*merge);
			if (count%10000000==0) cout << "\n Points xSorted: " << count;
			count++;
			delete buffer.at(outindex);

			Data* temp = new Data;
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

		for (vector<TemporaryFile*>::iterator it = buckets.begin();it !=buckets.end();++it) delete *it;
		buckets.clear();
		for (vector<Data*>::iterator it = buffer.begin();it !=buffer.end();++it) delete *it;
		buffer.clear();

		merge->rewindForReading();

	    int YbucketSize = count / split;
	    int ZbucketSize = YbucketSize  / split;
	    int counter=0;
	    bool yflip=true;
	    bool zflip=true;
		vector<Data*> minibuffer;

	    while (counter < count)
	    {
	    	  for (int i=0;i<YbucketSize;i++)
	    	  {
	    		  Data* temp = new Data;
	    		  if (temp->load(*merge)==false)
	    		  {
	  				delete temp;
	  				temp=NULL;
	  				break;
	    		  }
	    		  buffer.push_back(temp);
	       		  counter++;
	       		  if (counter >= count) break;
	    	  }

	    	  if (yflip==true) {std::sort(buffer.begin(),buffer.end(),Data::ySortAsc());yflip=false;}
	    	  else             {std::sort(buffer.begin(),buffer.end(),Data::ySortDec());yflip=true;}

	    	  int zcounter=0;
	    	  while (zcounter<buffer.size())
	    	  {
	    		  for (int i=0;i<ZbucketSize;i++)
		    	  {
		    		  if (zcounter>=buffer.size()) break;
		    		  minibuffer.push_back(buffer.at(zcounter++));
		    	  }

	    		  if (zflip==true) {std::sort(minibuffer.begin(),minibuffer.end(),Data::zSortAsc());zflip=false;}
		    	  else             {std::sort(minibuffer.begin(),minibuffer.end(),Data::zSortDec());zflip=true;}

		    	  for (std::vector<Data*>::const_iterator mt = minibuffer.begin();mt != minibuffer.end();++mt)
		    	  {
		    		  file << (*mt)->midPoint[0] << " " << (*mt)->midPoint[1] << " " << (*mt)->midPoint[2]<< endl;
		    		  if (counter%10000000==0) cout << "\n Points yzSorted: " << counter;
		    		  delete *mt;
		    	  }
	    		  minibuffer.clear();
	    	  }
	    	  buffer.clear();
	    }
	    delete merge;
		return count;
	}



};






#endif
