#ifndef ANALYZER_
#define ANALYZER_

/*
 * For Analyzing Tree Structural properties
 */
class Analyzer
	{
	public:
	int minLevel;
	int maxLevel;
	int totalNodes;
	int totalData;

	Analyzer()
		{
		maxLevel=0;
		totalNodes=0;
		totalData=0;
		minLevel = INT_MAX;
		}

	friend std::ostream & operator << (std::ostream & lhs, const Analyzer & rhs)
		{
		lhs << "Total Data: " << rhs.totalData << "\n";
		lhs << "Total Nodes: " << rhs.totalNodes << "\n";
		lhs << "Max Height: " << rhs.maxLevel << "\n";
		lhs << "Min Height: " << rhs.minLevel << "\n";
		}
	};

#endif
