class RtreeJoin
{

private:

	uint64 totalGridCells;
	vector<TreeNode*> tree;
	vector<TreeEntry*> nextInput;
	TreeEntry* root;

	unsigned int leafsize, nodesize;
	unsigned int totalnodes;
	int Levels;


		void writeNode(vector<TreeEntry*> objlist,int Level, vector<TreeNode*>& tree)
		{
			TreeNode* prNode = new TreeNode(Level);
			Box mbr;
			uint64 childIndex;
			totalnodes ++;
			int count =0;
			for (vector<TreeEntry*>::iterator it=objlist.begin(); it!=objlist.end(); ++it)
			{
				prNode->entries.push_back(*it);
				if (count==0) mbr = (*it)->mbr;
				else Box::combine((*it)->mbr,mbr,mbr);
				count++;
			}
			childIndex = tree.size();

			tree.push_back(prNode);
			nextInput.push_back(new TreeEntry(mbr,childIndex));
		}

		void createTreeLevel(vector<TreeEntry*>& input,int Level, vector<TreeNode*>& tree)
		{
			unsigned int nodeSize;
			if (Level==0) nodeSize = leafsize;
			else nodeSize = nodesize;

			if(PartitioningType != No_Sort)
			{
				statsTOUCH.sorting.start();
				std::sort(input.begin(),input.end(),Comparator());
				statsTOUCH.sorting.stop();
			}

			if(PartitioningType == STR_Sort)
			{
				//First sort by X then assigns the objects to partitions then sort by Y each partition separately then again assigns the objects to partitions then sort by Z objects of each partition separately
				statsTOUCH.sorting.start();
				cout<< "Node size " << nodeSize << endl;
				uint64 itemsD1 = nodeSize * nodeSize;
				uint64 itemsD2 = nodeSize;
				std::sort(input.begin(),input.end(),Comparator());
				uint64 i=0;
				while(true)
				{
					if((i+1)*itemsD1 < input.size())
						std::sort(input.begin()+i*itemsD1, input.begin()+(i+1)*itemsD1 ,ComparatorY());
					else
					{
						std::sort(input.begin()+i*itemsD1, input.end() ,ComparatorY());
						break;
					}
					i++;
				}
				i=0;
				while(true)
				{
					if((i+1)*itemsD2 < input.size())
						std::sort(input.begin()+i*itemsD2, input.begin()+(i+1)*itemsD2 ,ComparatorZ());
					else
					{
						std::sort(input.begin()+i*itemsD2, input.end() ,ComparatorZ());
						break;
					}
					i++;
				}
				statsTOUCH.sorting.stop();
			}

			cout << "Sort "<< input.size()<< " items in " << statsTOUCH.sorting << endl;


			vector<TreeEntry*> entries;
			for (vector<TreeEntry*>::iterator it=input.begin();it!=input.end();++it)
			{
				if (entries.size()<nodeSize)
				{
					entries.push_back(*it);
					if (entries.size()>=nodeSize)
					{
						writeNode(entries,Level,tree);
						entries.clear();
					}
				}
			}
			if (!entries.empty())
				writeNode(entries,Level,tree);
		}

public:

	RtreeJoin(unsigned int buckets)
	{
		statsTOUCH.initialize.start();

		statsTOUCH.gridSize = buckets;

		leafsize = ceil((double)vdsA.size()/(double)buckets);
		nodesize = base;

		statsTOUCH.initialize.stop();
	}

	~RtreeJoin()
	{
		delete &tree;
	}

	void createPartitions()
	{
		statsTOUCH.partition.start();
		// Build PRtree levels from bottom up
		Levels = 0;
		totalnodes = 0;
		while(vdsA.size()>1)
		{
			cout << "Tree Level: " << Levels << " treeNodes: " << tree.size() << " Remaining Input: " << vdsA.size() <<endl;
			createTreeLevel(vdsA,Levels++,tree);      // writes final nodes in tree and next level in nextInput
			swap(vdsA,nextInput);					// swap input and nextInput list
			nextInput.clear();
		}

		root = vdsA.front();

		cout << "Levels " << Levels << endl;

		statsTOUCH.partition.stop();
		cout << "creating the partitions time: " << statsTOUCH.partition << endl;

		// Destroy HTree
	}

	void assignment(const SpatialObjectList& ds)
	{
		statsTOUCH.building.start();
		bool overlaps;
		bool assigned;
		for (unsigned int i=0;i<ds.size();++i)
		{
			//if(i%1000==0)cout<<i<<endl;
			SpatialObject* obj = ds.at(i);
			Box objMBR = obj->getMBR();
			//objMBR.low = objMBR.low - epsilon;
			//objMBR.high = objMBR.high + epsilon;

			TreeEntry* nextNode;
			TreeNode* ptr = tree.at(root->childIndex);

			nextNode = NULL;

			while(true)
			{
				overlaps = false;
				assigned = false;
				for (unsigned int cChild = 0; cChild < ptr->entries.size(); ++cChild)
					if ( Box::overlap(objMBR,ptr->entries.at(cChild)->mbr) )
					{
						if(!overlaps)
						{
							overlaps = true;
							nextNode = ptr->entries.at(cChild);
						}
						else
						{
							//should be assigned to this level
							ptr->attachedObjs.push_back(obj);
							assigned = true;
							break;
						}
					}

				if(assigned)
					break;
				if(!overlaps)
				{
					//filtered
					statsTOUCH.filtered ++;
					break;
				}
				ptr = tree.at(nextNode->childIndex);
				if(ptr->leafnode /*|| ptr->level < 2*/)
				{
					ptr->attachedObjs.push_back(obj);
					break;
				}
			}
		}

		statsTOUCH.building.stop();
	}

	//join two given cells at the given level
	void joinBucketsNode(uint64 index, TreeNode* node, vector<TreeNode*> & Buckets, ResultPairs& results)
	{
		/*
		SpatialObjectList A,B;
		A.reserve(node->entries.size());
		B.reserve(statsTOUCH.max);
		SpatialObject * objA, * objB;
		HashBucket* Bs;

		Box mbr;
		if(Levels > 1)
		{
			TreeNode* parent = Buckets.at(node->level+1);
			for (unsigned int cChild = 0; cChild < parent->entries.size(); ++cChild)
				if(parent->entries.at(cChild)->childIndex == index)
					mbr = parent->entries.at(cChild)->mbr;

			Box::expand(mbr,epsilon);
		}

		for (unsigned int cChild = 0; cChild < node->entries.size(); ++cChild)
		{
			//compare n->entries.at(cChild)->mbr against all the items in ans
			objA = node->entries.at(cChild)->obj;

			A.push_back(objA);

		}

		int iB;

		for(int l=node->level ; l<Levels ; l++)
		{
			Bs = Buckets.at(l)->objs;
			iB=0;
			while(true)
			{
				if (iB==Bs->added)
				{
					if (Bs->next==NULL)
						break;
					else
					{
						Bs = Bs->next;
						iB=0;
					}
				}
				objB = Bs->objPtrs[iB];
				if(Levels == 1 || Box::overlap(objB->getMBR(),mbr))
					B.push_back(objB);
				iB++;
			}
			//Do the actual distance checking

			JOIN(A,B,results);
			B.clear();
		}
*/

	}
	//Joins spatial objects of the given vector against spatial objects in the leaf node
	void joinLeafVector(TreeNode* leaf, SpatialObjectList& vect)//What is wrong here ???????
	{

		//statsTOUCH.ItemsCompared += leaf->entries.size()*vect.size();
		//return;


		SpatialObject * obj;
		//cout<<"(" <<leaf->entries.size() << " to " << vect.size();
		for (unsigned int child = 0; child < leaf->entries.size(); ++child)
		{
			obj = leaf->entries.at(child)->obj;
			for(uint64 i=0; i<vect.size(); i++)
			if ( istouching(vect.at(i),obj) )
				//results.addPair( vect.at(i),obj );
				statsTOUCH.results ++;
		}
		//cout<<").";


	}

	//probing function
	void probe(ResultPairs& results)
	{


		//For every cell of A join it with its corresponding cell of B
		statsTOUCH.probing.start();
		queue<uint64> internals, leaves;
		//queue<Box> uni;
		TreeNode* internal,* leaf;
		//cout<<"max "<<statsTOUCH.max<<endl;
		//if(PartitioningType == 1)
	    Box localUniverse;

		internals.push(root->childIndex);
		//uni.push(root->mbr);


		SpatialGridHash* spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
		int internalObjsCount;
		int lvl = Levels;
		// A BFS on the tree then for each find all its leaf nodes by another BFS
		while(internals.size()>0)
		{

			uint64 inter = internals.front();
			internal = tree.at(inter);
			internals.pop();
			//localUniverse = uni.front();
			//uni.pop();
			if(!internal->leafnode)
			for (uint64 child = 0; child < internal->entries.size(); ++child)
			{
				uint64 interChild = internal->entries.at(child)->childIndex;
				//if(!tree.at(interChild)->leafnode)
					internals.push(interChild);
					//uni.push(internal->entries.at(child)->mbr);
			}
			//join the internal node with all *its* leaves
			internalObjsCount = internal->attachedObjs.size();

			if(internalObjsCount==0)
				continue;

			//write all the objects in the internal node's bucket to a vector
			if(lvl!=internal->level)
			{
				lvl = internal->level;
				cout << "\n### Level " << lvl <<endl;
			}

			if(true)//localJoin == algo_SGrid)// && localPartitions < internalObjsCount)// && internal->level >0)
			{
				//constructing the grid for the internal bucket
				//cout<<localUniverse<<endl;
				//spatialGridHash = new SpatialGridHash(localUniverse,localPartitions);
				statsTOUCH.deDuplicate.start();
				//spatialGridHash = new SpatialGridHash(root->mbr,localPartitions);
				spatialGridHash->build(internal->attachedObjs);
				statsTOUCH.deDuplicate.stop();
			}

			leaves.push(inter);
			while(leaves.size()>0)
			{
				leaf = tree.at(leaves.front());
				leaves.pop();
				if(leaf->leafnode)
				{
					statsTOUCH.comparing.start();
					// join leaf->entries and vect

					if(true)//localJoin == algo_SGrid)// && localPartitions < internalObjsCount)// && internal->level >0)
					{
						ResultList temp;
						//cout << "P";
						spatialGridHash->probe(leaf , temp);
						//cout << "C";
						statsTOUCH.results += temp.size();
						statsTOUCH.duplicates -= temp.size();
						//cout << "." << results.objA.size();
					}
					else
						JOIN(leaf,internal->attachedObjs,results);

					statsTOUCH.comparing.stop();
					//else if(PartitioningType == 2)
						//joinVectorLeaf(leaf,hb,results);
				}
				else
				{
					for (uint64 child = 0; child < leaf->entries.size(); ++child)
					{
						leaves.push(leaf->entries.at(child)->childIndex);
					}
				}
			}

			//if(localJoin == algo_SGrid && localPartitions < internalObjsCount)
			//	delete spatialGridHash;

		}

		statsTOUCH.probing.stop();

	}

	void analyze(const SpatialObjectList& dsA,const SpatialObjectList& dsB)
	{

		statsTOUCH.analyzing.start();
		uint64 emptyCells=0;
		uint64 sum=0,sqsum=0;
		double differenceSquared=0;
		statsTOUCH.footprint += vdsA.size()*(sizeof(TreeEntry*));
		statsTOUCH.footprint += dsB.size()*(sizeof(SpatialObject*));
		LVL = Levels;
		//vector<uint64> ItemPerLevel;
		ItemPerLevel.reserve(Levels);
		for(int i = 0 ; i<Levels ; i++)
			ItemPerLevel.push_back(0);
		for(unsigned int ni = 0; ni<tree.size() ; ni++)
		{
			SpatialObjectList objs = tree.at(ni)->attachedObjs;
			uint64 ptrs = objs.size();
			if(objs.size()==0)emptyCells++;
			ItemPerLevel[tree.at(ni)->level]+=ptrs;
			sum += ptrs;
			sqsum += ptrs*ptrs;
			if (statsTOUCH.max<ptrs) statsTOUCH.max = ptrs;

		}
		for(int i = 0 ; i<Levels ; i++)
			cout<< "level " << i << " items " << ItemPerLevel[i] <<endl;

		stats.footprint += sum*sizeof(SpatialObject*) + tree.size()*(sizeof(TreeNode*));
		statsTOUCH.avg = (sum+0.0) / (tree.size());
		statsTOUCH.percentageEmpty = (emptyCells+0.0) / (tree.size())*100.0;
		differenceSquared = ((double)sqsum/(double)tree.size())-statsTOUCH.avg*statsTOUCH.avg;
		statsTOUCH.std = sqrt(differenceSquared);
		statsTOUCH.analyzing.stop();
	}
};
