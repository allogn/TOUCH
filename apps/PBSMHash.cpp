// The class for doing the Partition Based Spatial Merge Join
class PBSMHash
{

private:

	HashTable hashTableA, hashTableB;
	Box universe;
	Vertex universeWidth;
	int resolution;
	uint64 totalGridCells;

	uint64 gridLocation2Index(const int x,const int y,const int z)
	{
		return  x + (y*resolution) + (z*resolution*resolution);
	}

	void vertex2GridLocation(const Vertex& v,int& x,int& y,int &z)
	{
		x = (v[0] > universe.low[0])?(int)floor( (v[0] - universe.low[0]) / universeWidth[0]) : 0;
		y = (v[1] > universe.low[1])?(int)floor( (v[1] - universe.low[1]) / universeWidth[1]) : 0;
		z = (v[2] > universe.low[2])?(int)floor( (v[2] - universe.low[2]) / universeWidth[2]) : 0;

		// if cell not valid assign last corner cells
		if (x>=resolution) x=resolution-1;
		if (y>=resolution) y=resolution-1;
		if (z>=resolution) z=resolution-1;

	}

	bool vertex2GridLocation(const Vertex& v,int& x,int& y,int &z, bool islower)
	{
		x = (v[0] > universe.low[0])?(int)floor( (v[0] - universe.low[0]) / universeWidth[0]) : 0;
		y = (v[1] > universe.low[1])?(int)floor( (v[1] - universe.low[1]) / universeWidth[1]) : 0;
		z = (v[2] > universe.low[2])?(int)floor( (v[2] - universe.low[2]) / universeWidth[2]) : 0;

		// if cell not valid assign last corner cells
		if (x>=resolution) x=resolution-1;
		if (y>=resolution) y=resolution-1;
		if (z>=resolution) z=resolution-1;

		if ( (!islower) && ( x<0 || y<0 || z<0 ))
			return true;
		if( islower && ( x>=resolution || y>=resolution || z>=resolution))
			return true;
		return false;

	}

public:

	PBSMHash(const Box& universeExtent, uint64 partitionPerDim)
	{
		stats.initialize.start();
		resolution = partitionPerDim;
		totalGridCells = pow(resolution,3);
		universe = universeExtent;
		Vertex difference;
		Vertex::differenceVector(universe.high,universe.low,difference);

		stats.gridSize = totalGridCells;

		for (int i=0;i<DIMENSION;++i)
			universeWidth[i] = ceil( (double)difference[i]/(double)resolution );

		stats.initialize.stop();
	}

	~PBSMHash()
	{
		for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
			delete it->second;
		for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
			delete it->second;

	}

	void build(SpatialObjectList& a, SpatialObjectList& b)
	{
		stats.building.start();
		double exp = epsilon * 0.5;
		for(SpatialObjectList::iterator A=a.begin(); A!=a.end(); ++A)
		{
			Box mbr = (*A)->getMBR();

			Box::expand(mbr,exp);

			int xMin,yMin,zMin;
			int xMax,yMax,zMax;
			vertex2GridLocation(mbr.low,xMin,yMin,zMin);
			vertex2GridLocation(mbr.high,xMax,yMax,zMax);

			for(int x=xMin; x<=xMax; x++)
				for(int y=yMin; y<=yMax; y++)
					for(int z=zMin; z<=zMax; z++)
					{
						uint64 index = gridLocation2Index(x,y,z);
						HashTable::iterator it= hashTableA.find(index);
						if (it==hashTableA.end())
						{
							HashValue* soList = new HashValue();
							soList->push_back(*A);
							hashTableA.insert( ValuePair(index,soList) );
						}
						else
						{
							it->second->push_back(*A);
						}
					}

		}
		for(SpatialObjectList::iterator B=b.begin(); B!=b.end(); ++B)
		{
			Box mbr = (*B)->getMBR();

			Box::expand(mbr,exp);
			if (!Box::overlap(mbr,universe))
			{
				stats.filtered ++;
				continue;
			}

			int xMin,yMin,zMin;
			int xMax,yMax,zMax;
			bool filtered = false;
			filtered |= vertex2GridLocation(mbr.low,xMin,yMin,zMin,true);
			filtered |= vertex2GridLocation(mbr.high,xMax,yMax,zMax,false);

			if(filtered)
				stats.filtered ++;
			else
			{
			for(int x=xMin; x<=xMax; x++)
				for(int y=yMin; y<=yMax; y++)
					for(int z=zMin; z<=zMax; z++)
					{
						uint64 index = gridLocation2Index(x,y,z);
						HashTable::iterator it= hashTableB.find(index);
						if (it==hashTableB.end())
						{
							HashValue* soList = new HashValue();
							soList->push_back(*B);
							hashTableB.insert( ValuePair(index,soList) );
						}
						else
						{
							it->second->push_back(*B);
						}
					}
			}
		}
		stats.building.stop();
	}
	//join two given cells at the given level
	void joincells(const uint64 indexA, const uint64 indexB, ResultList& results)
	{
		// join objects in the cells A and B
		SpatialObjectList A,B;

		HashTable::iterator hA = hashTableA.find(indexA);
		if (hA==hashTableA.end()) return;
		A = *( hA->second );

		HashTable::iterator hB = hashTableB.find(indexB);
		if (hB==hashTableB.end()) return;
		B = *( hB->second );
		JOIN(A,B,results);
	}
	void probe(ResultList& results)
	{
		//For every cell of A join it with its corresponding cell of B
		stats.probing.start();
		for(int i = 0 ; i < resolution ; i++)
			for(int j = 0 ; j < resolution ; j++)
				for(int k = 0 ; k < resolution ; k++)
				{
					uint64 index = gridLocation2Index(i,j,k);
					// Join indexA of the two hash tables A and B
					joincells(index, index, results);
				}

		stats.probing.stop();
	}
	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
	{
		stats.analyzing.start();
		stats.footprint += dsA.capacity()*(sizeof(SpatialObject*));
		stats.footprint += dsB.capacity()*(sizeof(SpatialObject*));
		uint64 sum=0,sqsum=0,sumA,sumB;
		for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
		{
			uint64 ptrs=((SpatialObjectList*)(it->second))->size();
			sumA += ptrs;
			sqsum += ptrs*ptrs;
			if (stats.max<ptrs) stats.max = ptrs;
		}

		for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
		{
			uint64 ptrs=((SpatialObjectList*)(it->second))->size();
			sumB += ptrs;
			sqsum += ptrs*ptrs;
			if (stats.max<ptrs) stats.max = ptrs;
		}
		sum = sumA+sumB;
		stats.footprint += sum*sizeof(SpatialObject*) + 2*sizeof(HashValue)*stats.gridSize;
		stats.avg = (sum+0.0) / (2*stats.gridSize+0.0);
		stats.repA = (double)(sumA)/(double)size_dsA;
		stats.repB = (double)(sumB)/(double)size_dsB;
		stats.percentageEmpty = (double)(2*stats.gridSize - hashTableA.size()- hashTableB.size()) / (double)(2*stats.gridSize)*100.0;
		double differenceSquared=0;
		differenceSquared = ((double)sqsum/(double)(2*stats.gridSize))-stats.avg*stats.avg;
		stats.std = sqrt(differenceSquared);
		stats.analyzing.stop();
	}
};
