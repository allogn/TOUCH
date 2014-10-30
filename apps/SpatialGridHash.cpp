// The class for doing the Spatial Grid Join
class SpatialGridHash
{

private:

	HashTable gridHashTable;
	Box universe;
	Vertex universeWidth;
	int64 resolution;

	int64 gridLocation2Index(const int x,const int y,const int z)
	{
		return (x + (y*resolution) + (z*resolution*resolution));
	}

	void vertex2GridLocation(const Vertex& v,int& x,int& y,int &z)
	{
		x = (int)floor( (v[0] - universe.low[0]) / universeWidth[0]);
		y = (int)floor( (v[1] - universe.low[1]) / universeWidth[1]);
		z = (int)floor( (v[2] - universe.low[2]) / universeWidth[2]);

		if (x<0) x=0; if (x>=resolution) x=resolution-1;
		if (y<0) y=0; if (y>=resolution) y=resolution-1;
		if (z<0) z=0; if (z>=resolution) z=resolution-1;
	}
	bool vertex2GridLocation(const Vertex& v,int& x,int& y,int &z, bool islower)
	{
		x = (int)floor( (v[0] - universe.low[0]) / universeWidth[0]);
		y = (int)floor( (v[1] - universe.low[1]) / universeWidth[1]);
		z = (int)floor( (v[2] - universe.low[2]) / universeWidth[2]);

		if ( (!islower) && ( x<0 || y<0 || z<0 ))
			return true;
		if( islower && ( x>=resolution || y>=resolution || z>=resolution))
			return true;

		// if cell not valid assign last corner cells
		if (x<0) x=0; if (x>=resolution) x=resolution-1;
		if (y<0) y=0; if (y>=resolution) y=resolution-1;
		if (z<0) z=0; if (z>=resolution) z=resolution-1;

		return false;

	}

	void getOverlappingCells(SpatialObject* sobj,vector<uint64>& cells)
	{
		Box mbr = sobj->getMBR();
		//cout<<mbr.low << " " << mbr.high << " " << epsilon<< endl;
		if(algorithm == algo_SGrid)
		{
			Box::expand(mbr,epsilon);
		}
		//cout<<mbr.low << " " << mbr.high << " " << epsilon<< endl;

		int xMin,yMin,zMin;
		vertex2GridLocation(mbr.low,xMin,yMin,zMin);

		int xMax,yMax,zMax;
		vertex2GridLocation(mbr.high,xMax,yMax,zMax);

		for (int i=xMin;i<=xMax;++i)
			for (int j=yMin;j<=yMax;++j)
				for (int k=zMin;k<=zMax;++k)
					cells.push_back( gridLocation2Index(i,j,k) );

	}

	void getCellMbr(Box& extent,const int x,const int y,const int z)
	{
		extent.low[0] = x*universeWidth[0]; extent.high[0] = (x+1)*universeWidth[0];
		extent.low[1] = y*universeWidth[1]; extent.high[1] = (y+1)*universeWidth[1];
		extent.low[2] = z*universeWidth[2]; extent.high[2] = (z+1)*universeWidth[2];

		extent.low = extent.low + universe.low;
		extent.high = extent.high + universe.low;
	}

	bool getProjectedCells(SpatialObject* sobj,vector<uint64>& cells)
	{
		Box mbr = sobj->getMBR();

		if (!Box::overlap(mbr,universe)) return false;

		int xMin,xMax,yMin,yMax,zMin,zMax;
		vertex2GridLocation(mbr.low,xMin,yMin,zMin);
		vertex2GridLocation(mbr.high,xMax,yMax,zMax);

/*
		bool filtered = false;
		filtered |= vertex2GridLocation(mbr.low,xMin,yMin,zMin,true);
		filtered |= vertex2GridLocation(mbr.high,xMax,yMax,zMax,false);

		if(filtered)
			stats.filtered ++;
		else
			*/
		for (int i=xMin;i<=xMax;++i)
			for (int j=yMin;j<=yMax;++j)
				for (int k=zMin;k<=zMax;++k)
				{
					cells.push_back( gridLocation2Index(i,j,k) );
				}
		return true;
	}
	bool getProjectedCells(Box& mbr,vector<uint64>& cells)
	{

		if (!Box::overlap(mbr,universe)) return false;

		int xMin,xMax,yMin,yMax,zMin,zMax;
		vertex2GridLocation(mbr.low,xMin,yMin,zMin);
		vertex2GridLocation(mbr.high,xMax,yMax,zMax);

/*
		bool filtered = false;
		filtered |= vertex2GridLocation(mbr.low,xMin,yMin,zMin,true);
		filtered |= vertex2GridLocation(mbr.high,xMax,yMax,zMax,false);

		if(filtered)
			stats.filtered ++;
		else
			*/
		for (int i=xMin;i<=xMax;++i)
			for (int j=yMin;j<=yMax;++j)
				for (int k=zMin;k<=zMax;++k)
				{
					cells.push_back( gridLocation2Index(i,j,k) );
				}
		return true;
	}

public:

	SpatialGridHash(const Box& universeExtent,const int gridResolutionPerDimension)
	{
		stats.initialize.start();

		resolution = gridResolutionPerDimension;
		universe = universeExtent;
		Vertex difference;
		Vertex::differenceVector(universe.high,universe.low,difference);
		for (int i=0;i<DIMENSION;++i)
			universeWidth[i] = difference[i]/resolution;
		stats.gridSize = (uint64)(resolution * resolution) * resolution;

		stats.initialize.stop();
	}

	~SpatialGridHash()
	{
		for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
			delete it->second;
	}

	void build(SpatialObjectList& dsA)
	{
		stats.building.start();
		gridHashTable.clear();
		for(SpatialObjectList::iterator i=dsA.begin(); i!=dsA.end(); ++i)
		{
			vector<uint64> cells;
			getOverlappingCells(*i,cells);
			for (vector<uint64>::iterator j = cells.begin(); j!=cells.end(); ++j)
			{
				HashTable::iterator it= gridHashTable.find(*j);
				if (it==gridHashTable.end())
				{
					HashValue* soList = new HashValue();
					soList->push_back(*i);
					gridHashTable.insert( ValuePair(*j,soList) );
				}
				else
				{
					it->second->push_back(*i);
				}
			}
		}
		stats.building.stop();
		//cout<< "Number of assigned object " << assigned << " from " << dsA.size()<<endl;
	}

	void clear()
	{
		gridHashTable.clear();
		//for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
		// delete	it->second;
	}


	void probe(const SpatialObjectList& dsB,ResultList& results)
	{
		stats.probing.start();
		for(SpatialObjectList::const_iterator i=dsB.begin(); i!=dsB.end(); ++i)
		{
			vector<uint64> cells;
			if (!getProjectedCells( *i , cells ))
			{
				stats.filtered++;
				continue;
			}
			///// Get Unique Objects from Grid Hash in Vicinity
			stats.hashprobe += cells.size();
			for (vector<uint64>::const_iterator j = cells.begin(); j!=cells.end(); ++j)
			{
				HashTable::iterator it = gridHashTable.find(*j);
				if (it==gridHashTable.end()) continue;
				HashValue* soList = it->second;
				for (HashValue::const_iterator k=soList->begin(); k!=soList->end(); ++k)
					if ( istouching( *i , *k) )
					{
						stats.deDuplicate.start();
						uint64 temp = results.size();
						results.insert( ResultPair(*i , *k) );
						if(temp == results.size())
							stats.duplicates++;
						else
							stats.results++;
						stats.deDuplicate.stop();
					}
			}
		}

		stats.probing.stop();
	}

	void probe(TreeNode* leaf,ResultList& results)
	{
		stats.probing.start();

		for (unsigned int child = 0; child < leaf->entries.size(); ++child)
		{
			vector<uint64> cells;
			if (!getProjectedCells( leaf->entries.at(child)->mbr  , cells ))
			{
				stats.filtered++;
				continue;
			}
			///// Get Unique Objects from Grid Hash in Vicinity
			statsTOUCH.hashprobe += cells.size();
			for (vector<uint64>::const_iterator j = cells.begin(); j!=cells.end(); ++j)
			{
				HashTable::iterator it = gridHashTable.find(*j);
				if (it==gridHashTable.end()) continue;
					HashValue* soList = it->second;
				for (HashValue::const_iterator k=soList->begin(); k!=soList->end(); ++k)
					if ( istouching( leaf->entries.at(child)->obj , *k) )
					{
						statsTOUCH.duplicates++;
						results.insert( ResultPair(leaf->entries.at(child)->obj , *k) );
					}
			}

		}
		stats.probing.stop();
	}

	void joinLeafVector(TreeNode* leaf, SpatialObjectList& vect, ResultPairs& results)//What is wrong here ???????
	{
		stats.comparing.start();

		for (vector<TreeEntry*>::iterator it = leaf->entries.begin(); it<leaf->entries.end(); ++it)
		{
			for(uint64 i=0; i<vect.size(); i++)
			if ( istouching(vect.at(i),(*it)->obj) )
				results.addPair( vect.at(i),(*it)->obj );
		}

		stats.comparing.stop();
	}

	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
	{

		stats.analyzing.start();
		stats.footprint += dsA.capacity()*(sizeof(SpatialObject*));
		stats.footprint += dsB.capacity()*(sizeof(SpatialObject*));
		cout << "Cell Width: " << universeWidth  << endl;
//		cout << "Bucket Count: " << gridHashTable.bucket_count() <<endl;
//		cout << "Max Bucket Count: " << gridHashTable.max_bucket_count() << endl;
//		cout << "The average number of elements per bucket: " << gridHashTable.load_factor() << endl;
//		cout << "Max Load Factor: " << gridHashTable.max_load_factor() << endl;
//		cout << "size(" << gridHashTable.size() << ") of the largest possible container: " << gridHashTable.max_size();

		uint64 sum=0,sqsum=0;
		for (HashTable::iterator it = gridHashTable.begin(); it!=gridHashTable.end(); ++it)
		{
			uint64 ptrs=((SpatialObjectList*)(it->second))->size();
			sum += ptrs;
			sqsum += ptrs*ptrs;
			if (stats.max<ptrs) stats.max = ptrs;
		}
		stats.footprint += sum*sizeof(SpatialObject*) +  sizeof(HashValue)*stats.gridSize;
		stats.avg = (sum+0.0) / (stats.gridSize+0.0);
		stats.percentageEmpty = (double)(stats.gridSize - gridHashTable.size()) / (double)(stats.gridSize)*100.0;
		stats.repA = (double)(sum)/(double)size_dsA;
		double differenceSquared=0;
		differenceSquared = ((double)sqsum/(double)stats.gridSize)-stats.avg*stats.avg;
		stats.std = sqrt(differenceSquared);
		stats.analyzing.stop();
	}
};
