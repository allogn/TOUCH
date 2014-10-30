//The class for doing the Size Separation Spatial Join
class S3Hash : public JoinAlgorithm
{

private:

	HashTable hashTableA, hashTableB;
	Box universe;
	Vertex* universeWidth;
	int levels;
	int* resolution;
	uint64* indexOffset;	// Starting index of every level in the hash tables
	uint64 totalGridCells;

	uint64 gridLocation2Index(const int x,const int y,const int z, const int level)
	{
		return  indexOffset[level]+(x + (y*resolution[level]) + (z*resolution[level]*resolution[level]));
	}
	void vertex2GridLocation(const Vertex& v,int& x,int& y,int &z, const int level)
	{
		x = (v[0] > universe.low[0])?(int)floor( (v[0] - universe.low[0]) / universeWidth[level][0]):0;
		y = (v[1] > universe.low[1])?(int)floor( (v[1] - universe.low[1]) / universeWidth[level][1]):0;
		z = (v[2] > universe.low[2])?(int)floor( (v[2] - universe.low[2]) / universeWidth[level][2]):0;

		// if cell not valid assign last corner cells
		if (x>=resolution[level]) x=resolution[level]-1;
		if (y>=resolution[level]) y=resolution[level]-1;
		if (z>=resolution[level]) z=resolution[level]-1;
	}

public:

	S3Hash(const Box& universeExtent, int level)
	{

		stats.initialize.start();
		levels = level;
		resolution = (int*)malloc(sizeof(int)*levels);
		indexOffset = (uint64*)malloc(sizeof(uint64)*levels);
		totalGridCells = 1;
		universeWidth = (Vertex*)malloc(sizeof(Vertex)*levels);

		universe = universeExtent;
		resolution[0] = 1;
		indexOffset[0] = 0;
		Vertex difference;
		Vertex::differenceVector(universe.high,universe.low,difference);

		for(int l = 1 ; l < levels ; l++)
		{
			resolution[l] = resolution[l-1]*base;
			indexOffset[l] = totalGridCells;
			totalGridCells += pow(resolution[l],DIMENSION);
		}
		cout << "Total cells: " << totalGridCells << endl;
		stats.gridSize = totalGridCells;

		for(int l = 0 ; l < levels ; l++)
		{
			for (int i=0;i<DIMENSION;++i)
				universeWidth[l][i] = ceil( (double)difference[i]/(double)resolution[l] );
		}

		stats.initialize.stop();
	}

	~S3Hash()
	{

		for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
			delete it->second;
		for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
			delete it->second;

		delete indexOffset;
		delete resolution;
		//delete universeWidth;

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
			int level;
			for(level = levels - 1; level >= 0 ; level --)
			{
				vertex2GridLocation(mbr.low,xMin,yMin,zMin,level);
				vertex2GridLocation(mbr.high,xMax,yMax,zMax,level);
				if(xMin==xMax && yMin==yMax && zMin == zMax )
					break;
			}
			uint64 index = gridLocation2Index(xMin,yMin,zMin,level);
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
		for(SpatialObjectList::iterator B=b.begin(); B!=b.end(); ++B)
		{
			Box mbr = (*B)->getMBR();

			Box::expand(mbr,exp);

			int xMin,yMin,zMin;
			int xMax,yMax,zMax;
			int level;
			for(level = levels - 1; level >= 0 ; level --)
			{
				vertex2GridLocation(mbr.low,xMin,yMin,zMin,level);
				vertex2GridLocation(mbr.high,xMax,yMax,zMax,level);
				if(xMin==xMax && yMin==yMax && zMin == zMax )
					break;
			}
			uint64 index = gridLocation2Index(xMin,yMin,zMin,level);
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
		stats.building.stop();
	}
	//join two given cells at the given level
	void joincells(const uint64 indexA, const uint64 indexB, ResultPairs& results)
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
	void probe(ResultPairs& results)
	{
		//For every cell in level i of A join it with cells touching it from level 0 to L of B
		stats.probing.start();
		for(int levelA = levels-1; levelA >= 0 ; levelA--)
		{
			for(int Ai = 0 ; Ai < resolution[levelA] ; Ai++)
				for(int Aj = 0 ; Aj < resolution[levelA] ; Aj++)
					for(int Ak = 0 ; Ak < resolution[levelA] ; Ak++)
					{
						uint64 indexA = gridLocation2Index(Ai,Aj,Ak,levelA);

						int xMin,yMin,zMin;
						//int xMax,yMax,zMax;
						//xMin = Ai*universeWidth[levelA][0];
						//yMin = Aj*universeWidth[levelA][1];
						//zMin = Ak*universeWidth[levelA][2];

						//xMax = (Ai+1)*universeWidth[levelA][0];
						//yMax = (Aj+1)*universeWidth[levelA][1];
						//zMax = (Ak+1)*universeWidth[levelA][2];


						for(int levelB = 0; levelB < levelA ; levelB++)
						{
							int Bi = xMin/universeWidth[levelB][0];
							int Bj = yMin/universeWidth[levelB][1];
							int Bk = zMin/universeWidth[levelB][2];
							uint64 indexB = gridLocation2Index(Bi,Bj,Bk,levelB);
							// Join indexA of the hash tables A with indexB of Hash Table B
							joincells(indexA, indexB, results);
						}
						// Join indexA of the two hash tables A and B
						joincells(indexA, indexA, results);

						for(int levelB = levelA+1; levelB < levels ; levelB++)
						{
							int bucketPerDim = pow(2,levelB-levelA);//res[levelB]/res[levelA]
							for(int Bi=0 ; Bi<bucketPerDim ; Bi++)
								for(int Bj=0 ; Bj<bucketPerDim ; Bj++)
									for(int Bk=0 ; Bk<bucketPerDim ; Bk++)
									{
										uint64 indexB = gridLocation2Index(Ai*bucketPerDim+Bi,Aj*bucketPerDim+Bj,Ak*bucketPerDim+Bk,levelB);
										// Join indexA of the A hash tables A with indexB of Hash Table B
										joincells(indexA, indexB, results);
									}
						}
					}
		}

		stats.probing.stop();
	}

	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
	{
		stats.analyzing.start();
		stats.footprint += dsA.capacity()*(sizeof(SpatialObject*));
		stats.footprint += dsB.capacity()*(sizeof(SpatialObject*));
		uint64 sum=0,sqsum=0;
		for (HashTable::iterator it = hashTableA.begin(); it!=hashTableA.end(); ++it)
		{
			uint64 ptrs=((SpatialObjectList*)(it->second))->size();
			sum += ptrs;
			sqsum += ptrs*ptrs;
			if (stats.max<ptrs) stats.max = ptrs;
		}
		for (HashTable::iterator it = hashTableB.begin(); it!=hashTableB.end(); ++it)
		{
			uint64 ptrs=((SpatialObjectList*)(it->second))->size();
			sum += ptrs;
			sqsum += ptrs*ptrs;
			if (stats.max<ptrs) stats.max = ptrs;
		}
		stats.footprint += sum*sizeof(SpatialObjectList) +  sizeof(HashValue)*stats.gridSize;
		stats.avg = (sum+0.0) / (stats.gridSize+0.0);
		stats.percentageEmpty = (double)(stats.gridSize - hashTableA.size()- hashTableB.size()) / (double)(stats.gridSize)*100.0;
		double differenceSquared=0;
		differenceSquared = ((double)sqsum/(double)stats.gridSize)-stats.avg*stats.avg;
		stats.std = sqrt(differenceSquared);
		stats.analyzing.stop();
	}
};
